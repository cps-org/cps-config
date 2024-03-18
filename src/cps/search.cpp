// SPDX-License-Identifier: MIT
// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Bret Brown

#include "cps/search.hpp"

#include "cps/error.hpp"
#include "cps/loader.hpp"
#include "cps/utils.hpp"
#include "cps/version.hpp"

#include <fmt/core.h>
#include <tl/expected.hpp>

#include <deque>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_set>

namespace fs = std::filesystem;

namespace cps::search {

    namespace {

        /// @brief A CPS file, along with the components in that CPS file to
        /// load
        class Dependency {
          public:
            Dependency(loader::Package && obj) : package{std::move(obj)} {};

            /// @brief The loaded CPS file
            loader::Package package;
            /// @brief the components from that CPS file to use
            std::vector<std::string> components;
        };

        /// @brief A DAG node
        class Node {
          public:
            Node(Dependency obj) : data{std::move(obj)} {};
            Node(loader::Package obj) : data{std::move(obj)} {};

            Dependency data;
            std::vector<std::shared_ptr<Node>> depends;
        };

        void dfs(const std::shared_ptr<Node> & node, std::unordered_set<std::shared_ptr<Node>> & visited,
                 std::deque<std::shared_ptr<Node>> & sorted) {
            visited.emplace(node);
            for (auto && d : node->depends) {
                if (visited.find(d) == visited.end()) {
                    dfs(d, visited, sorted);
                }
            }
            sorted.emplace_front(node);
        }

        /// @brief Perform a topological sort of the DAG
        /// @param root The root Node
        /// @return A linear topological sorting of the DAG
        std::vector<std::shared_ptr<Node>> tsort(const std::shared_ptr<Node> & root) {
            std::deque<std::shared_ptr<Node>> sorted;
            std::unordered_set<std::shared_ptr<Node>> visited;
            dfs(root, visited, sorted);

            std::vector<std::shared_ptr<Node>> out{};
            out.insert(out.end(), sorted.begin(), sorted.end());
            return out;
        }

        const std::vector<fs::path> nix{"/usr", "/usr/local"};
        // TODO: const std::vector<std::string> mac{""};
        // TODO: const std::vector<std::string> win{""};

        std::vector<fs::path> cached_paths{};

        const std::vector<fs::path> search_paths(Env env) {
            if (!cached_paths.empty()) {
                return cached_paths;
            }

            if (env.cps_path) {
                cached_paths.reserve(nix.size());
                cached_paths.insert(cached_paths.end(), nix.begin(), nix.end());
                auto && paths = utils::split(env.cps_path.value());
                cached_paths.reserve(paths.size());
                cached_paths.insert(cached_paths.end(), paths.begin(), paths.end());
            } else {
                cached_paths = nix;
            }

            return cached_paths;
        }

        const fs::path libdir() {
            // TODO: libdir needs to be configurable based on the personality,
            //       and different name schemes.
            //       This is complicated by the fact that different distros have
            //       different schemes.
            return "lib";
        }

        /// @brief Find all possible paths for a given CPS name
        /// @param name The name of the CPS file to find
        /// @return A vector of paths which patch the given name, or an error
        tl::expected<std::vector<fs::path>, std::string> find_paths(std::string_view name, Env env) {
            // If a path is passed, then just return that.
            if (fs::is_regular_file(name)) {
                return std::vector<fs::path>{name};
            }

            // TODO: Need something like pkgconf's --personality option
            // TODO: we likely either need to return all possible files, or load
            // a file
            // TODO: what to do about finding multiple versions of the same
            // dependency?
            auto && paths = search_paths(env);
            std::vector<fs::path> found{};
            for (auto && prefix : paths) {
                // TODO: <prefix>/<libdir>/cps/<name-like>/
                // TODO: <prefix>/share/cps/<name-like>/
                // TODO: <prefix>/share/cps/

                const fs::path dir = prefix / libdir() / "cps";
                if (fs::is_directory(dir)) {
                    // TODO: <name-like>
                    const fs::path file = dir / fmt::format("{}.cps", name);
                    if (fs::is_regular_file(file)) {
                        found.push_back(file);
                    }
                }
            }

            if (found.empty()) {
                return tl::unexpected(fmt::format("Could not find a CPS file for {}", name));
            }
            return found;
        }

        struct ProcessedRequires {
            std::vector<std::string> components;
            bool defaults;

            ProcessedRequires(bool d) : defaults{d} {};
            ProcessedRequires(std::string s) : components{{std::move(s)}}, defaults{false} {};
        };

        /// @brief Extract all required dependencies with their components
        /// @param components The requested componenets
        /// @return a map of dependency to (components[], use_defaults)
        std::unordered_map<std::string, ProcessedRequires>
        process_requires(const std::vector<std::string> & components) {
            std::unordered_map<std::string, ProcessedRequires> map;
            for (auto && c : components) {
                std::vector<std::string> vals = utils::split(c);
                if (vals.size() == 1) {
                    // In this case we want to use the default components
                    // TODO: it's probably an error for one CPS file to specify
                    // the same component with default and non-default?
                    if (auto x = map.find(vals[0]); x != map.end()) {
                        /// XXX: blarg this is ugly
                        x->second.defaults = true;
                    } else {
                        map.emplace(vals[0], ProcessedRequires{true});
                    }
                } else {
                    // "" is a special value that means "this dependency"
                    if (auto x = map.find(vals[0]); x != map.end()) {
                        /// XXX: blarg this is ugly
                        x->second.components.emplace_back(vals[1]);
                    } else {
                        map.emplace(vals[0], vals[1]);
                    }
                }
            }
            return map;
        }

        class NodeFactory {
          public:
            NodeFactory() = default;

            tl::expected<std::shared_ptr<Node>, std::string> get(std::string_view name, const fs::path & path) {
                if (auto && hit = cache.find(std::string{name}); hit != cache.end()) {
                    return hit->second;
                }

                std::ifstream file;
                file.open(path);
                auto n = std::make_shared<Node>(CPS_TRY(loader::load(file, path.parent_path())));

                cache.emplace(name, n);
                return n;
            }

          private:
            std::unordered_map<std::string, std::shared_ptr<Node>> cache;
        };

        tl::expected<std::shared_ptr<Node>, std::string>
        build_node(std::string_view name, const loader::Requirement & requirements, NodeFactory factory, Env env) {
            const std::vector<fs::path> paths = CPS_TRY(find_paths(name, env));
            for (auto && path : paths) {

                auto maybe_node = factory.get(name, path);
                if (!maybe_node) {
                    continue;
                }
                auto node = maybe_node.value();
                const loader::Package & p = node->data.package;

                // If this package doesn't meet the requirements then reject it and continue on.
                // The conditions it could fail to meet are:
                //  1. the provided version (or Compat-Version) is < the required version
                //  2. This package lacks required components
                if (requirements.version) {
                    // From the CPS spec, version 0.10.0, for package::version,
                    // which as the same semantics as requirement::version:
                    //
                    // > If not provided, the CPS will not satisfy any request for
                    // > a specific version of the package.
                    if (!p.version) {
                        continue;
                    }
                    if (version::compare(p.version.value(), version::Operator::lt, requirements.version.value(),
                                         p.version_schema)) {
                        continue;
                    }
                }

                if (!std::all_of(requirements.components.begin(), requirements.components.end(),
                                 [p](const std::string & c) { return p.components.find(c) != p.components.end(); })) {
                    continue;
                }

                std::vector<std::shared_ptr<Node>> found;
                found.reserve(p.require.size());
                for (auto && [n, r] : p.require) {
                    auto && child = build_node(n, r, factory, env);
                    if (child) {
                        found.emplace_back(child.value());
                    } else {
                        break;
                    }
                }
                if (found.size() < p.require.size()) {
                    continue;
                }

                node->depends.insert(node->depends.end(), found.begin(), found.end());
                return node;
            }

            return tl::unexpected(fmt::format("Could not find a dependency to satisfy {}", name));
        }

        tl::expected<std::shared_ptr<Node>, std::string> build_node(std::string_view name,
                                                                    const loader::Requirement & requirements, Env env) {
            NodeFactory factory{};
            return build_node(name, requirements, factory, env);
        }

        template <typename T, typename U>
        void merge_result(const std::unordered_map<T, std::vector<U>> & input,
                          std::unordered_map<T, std::vector<U>> & output) {
            for (auto && [l, vals] : input) {
                output[l].insert(output[l].end(), vals.begin(), vals.end());
            }
        }

        template <typename T, typename U>
        void merge_result(const std::unordered_map<T, std::vector<U>> & input,
                          std::unordered_map<T, std::vector<U>> & output,
                          const std::function<U(const U &)> transformer) {
            for (auto && [l, vals] : input) {
                std::transform(vals.begin(), vals.end(), std::back_inserter(output[l]), transformer);
            }
        }

        template <typename T> void merge_result(const std::vector<T> & input, std::vector<T> & output) {
            output.insert(output.end(), input.begin(), input.end());
        }

        template <typename T> void merge_result(const std::optional<T> & input, std::vector<T> & output) {
            if (input) {
                output.emplace_back(input.value());
            }
        }

        fs::path calculate_prefix(const fs::path & path) {
            // TODO: Windows
            // TODO: /cps/<name-like>
            std::vector<std::string> split = utils::split(std::string{path}, "/");
            if (split.back() == "") {
                split.pop_back();
            }
            if (split.back() == "cps") {
                split.pop_back();
            }
            if (split.back() == "share") {
                split.pop_back();
            }
            // TODO: this needs to be generic
            if (split.back() == "lib") {
                split.pop_back();
            }
            fs::path p{"/"};
            for (auto && s : split) {
                p /= s;
            }
            return p;
        }

        /// @brief Calculate the required components in the graph
        /// @param node The node to process
        /// @param components the components required from this node
        void set_components(std::shared_ptr<Node> node, const std::vector<std::string> & components,
                            bool default_components) {
            // Set the components that this package's depndees want
            if (default_components && node->data.package.default_components) {
                const std::vector<std::string> & defs = node->data.package.default_components.value();
                node->data.components.insert(node->data.components.end(), defs.begin(), defs.end());
            }
            node->data.components.insert(node->data.components.end(), components.begin(), components.end());

            for (const std::string & c_name : node->data.components) {
                // It's possible that the Package::Requires section listed
                // dependencies we don't actually need. If we don't need them we
                // can trim the graph
                std::vector<std::shared_ptr<Node>> trimmed;

                // This *should* be validated such that we won't have an exception
                const loader::Component & component = node->data.package.components.at(c_name);
                auto && required = process_requires(component.require);
                for (std::shared_ptr<Node> & child : node->depends) {
                    if (auto && child_comps = required.find(child->data.package.name); child_comps != required.end()) {
                        trimmed.emplace_back(child);
                        set_components(child, child_comps->second.components, child_comps->second.defaults);
                    }
                }
                node->depends = trimmed;

                if (auto && self = required.find(""); self != required.end()) {
                    // Don't insert these twice
                    if (!default_components && self->second.defaults && node->data.package.default_components) {
                        const std::vector<std::string> & defs = node->data.package.default_components.value();
                        node->data.components.insert(node->data.components.end(), defs.begin(), defs.end());
                    }
                    node->data.components.insert(node->data.components.end(), self->second.components.begin(),
                                                 self->second.components.end());
                }
            }
        }

    } // namespace

    Result::Result(){};

    tl::expected<Result, std::string> find_package(std::string_view name, Env env) {
        return find_package(name, {}, true, env, std::nullopt);
    }

    tl::expected<Result, std::string> find_package(std::string_view name, const std::vector<std::string> & components,
                                                   bool default_components, Env env,
                                                   std::optional<std::string> prefix_variable) {
        // XXX: do we need process_requires here?
        auto && root = CPS_TRY(build_node(name, loader::Requirement{components}, env));
        // This has to be done as a two step pass, since we want to trim any
        // unecessary nodes from the graph, but we cannot do that while finding,
        // since we could hae a diamond dependency, where the two dependees have
        // different components they want.
        set_components(root, components, default_components);
        auto && flat = tsort(root);

        Result result{};

        result.version = root->data.package.version.value_or("unknown");

        const auto prefix_path = [&]() -> std::optional<fs::path> {
            if (prefix_variable) {
                return fs::path{prefix_variable.value()};
            }
            return std::nullopt;
        }();

        for (auto && node : flat) {

            const auto && prefix_replacer = [&](const std::string & s) -> std::string {
                // TODO: Windows…
                auto && split = utils::split(s, "/");
                if (split[0] == "@prefix@") {
                    auto p = prefix_path.value_or(calculate_prefix(node->data.package.cps_path));
                    for (auto it = split.begin() + 1; it != split.end(); ++it) {
                        p /= *it;
                    }
                    return p;
                }
                return s;
            };

            for (const auto & c_name : node->data.components) {
                // We should have already errored if this is not the case
                auto && f = node->data.package.components.find(c_name);
                utils::assert_fn(
                    f != node->data.package.components.end(),
                    fmt::format("Could not find component {} of pacakge {}", c_name, node->data.package.name));
                auto && comp = f->second;

                // Convert prefix at this point because:
                // 1. we are about to lose which CPS file the information came
                // from
                // 2. if we do it at the search point we have to plumb overrides
                // deep into that
                merge_result<loader::KnownLanguages, std::string>(comp.includes, result.includes, prefix_replacer);
                merge_result(comp.defines, result.defines);
                merge_result(comp.compile_flags, result.compile_flags);
                merge_result(comp.link_libraries, result.link_libraries);
                merge_result(comp.link_flags, result.link_flags);
                if (comp.type != loader::Type::interface) {
                    if (!comp.location) {
                        return tl::make_unexpected(fmt::format("Component `{}` requires 'location' attribute", c_name));
                    }
                    result.link_location.emplace_back(
                        prefix_replacer(comp.link_location.value_or(comp.location.value())));
                }
            }
        }

        return result;
    }

} // namespace cps::search
