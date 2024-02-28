// SPDX-License-Identifier: MIT
// Copyright © 2023 Dylan Baker

#include "search.hpp"
#include "error.hpp"
#include "fmt/core.h"
#include "loader.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <fmt/core.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace fs = std::filesystem;

namespace search {

    namespace {

        /// @brief A CPS file, along with the components in that CPS file to
        /// load
        class Dependency {
          public:
            Dependency(loader::Package && obj,
                       std::vector<std::string> && comps)
                : package{std::move(obj)}, components{std::move(comps)} {};

            /// @brief The loaded CPS file
            loader::Package package;
            /// @brief the components from that CPS file to use
            std::vector<std::string> components;
        };

        /// @brief A DAG node
        class Node {
          public:
            Node(Dependency obj) : data{std::move(obj)} {};
            Node(loader::Package obj, std::vector<std::string> comps)
                : data{std::move(obj), std::move(comps)} {};

            Dependency data;
            std::vector<std::shared_ptr<Node>> depends;
        };

        void dfs(const std::shared_ptr<Node> & node,
                 std::unordered_set<std::shared_ptr<Node>> & visited,
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
        std::vector<std::shared_ptr<Node>>
        tsort(const std::shared_ptr<Node> & root) {
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

        const std::vector<fs::path> search_paths() {
            if (!cached_paths.empty()) {
                return cached_paths;
            }

            if (const char * env_c = std::getenv("CPS_PATH")) {
                cached_paths.reserve(nix.size());
                cached_paths.insert(cached_paths.end(), nix.begin(), nix.end());
                auto && env = utils::split(env_c);
                cached_paths.reserve(env.size());
                cached_paths.insert(cached_paths.end(), env.begin(), env.end());
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
        tl::expected<std::vector<fs::path>, std::string>
        find_paths(std::string_view name) {
            // If a path is passed, then just return that.
            if (fs::is_regular_file(name)) {
                return std::vector<fs::path>{name};
            }

            // TODO: Need something like pkgconf's --personality option
            // TODO: we likely either need to return all possible files, or load
            // a file
            // TODO: what to do about finding multiple versions of the same
            // dependency?
            auto && paths = search_paths();
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
                return tl::unexpected(
                    fmt::format("Could not find a CPS file for {}", name));
            }
            return found;
        }

        struct ProcessedRequires {
            std::vector<std::string> components;
            bool defaults;

            ProcessedRequires(bool d) : defaults{d} {};
            ProcessedRequires(std::string s)
                : components{{std::move(s)}}, defaults{false} {};
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

        tl::expected<std::shared_ptr<Node>, std::string>
        build_node(std::string_view name,
                   const std::vector<std::string> & components,
                   bool default_components) {
            const std::vector<fs::path> paths = TRY(find_paths(name));
            for (auto && path : paths) {

                loader::Package p = TRY(loader::load(path));

                std::vector<std::string> comps = components;
                if (default_components && p.default_components) {
                    comps.insert(comps.end(),
                                 p.default_components.value().begin(),
                                 p.default_components.value().end());
                }

                // If we don't have all of the components requested this isn't a
                // valid candidate
                if (!std::all_of(
                        comps.begin(), comps.end(), [&](const std::string & s) {
                            return p.components.find(s) != p.components.end();
                        })) {
                    continue;
                }

                auto node = std::make_shared<Node>(p, comps);

                // TODO: need to ensure that any version requirements are met
                // TODO: need to check the graph for cycles and error if there
                // is a
                //       cycle
                // TODO: this needs a  lot of testing
                for (auto && c : comps) {
                    // TODO: Error handling
                    auto && comp = p.components.at(c);
                    auto && split = process_requires(comp.require);
                    for (auto && req : split) {
                        if (req.first == "") {
                            node->data.components.insert(
                                node->data.components.end(),
                                req.second.components.begin(),
                                req.second.components.end());
                            continue;
                        }
                        // XXX: This loop needs to be transactional, if any of
                        // the nodes is an error, then we need to throw away all
                        // of the work and go back and try again.
                        auto && n = build_node(req.first, req.second.components,
                                               req.second.defaults);
                        if (n.has_value()) {
                            node->depends.emplace_back(std::move(n.value()));
                        }
                    }
                }

                return node;
            }

            return tl::unexpected(
                fmt::format("Could not find a dependency to satisfy {}", name));
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
                std::transform(vals.begin(), vals.end(),
                               std::back_inserter(output[l]), transformer);
            }
        }

        template <typename T>
        void merge_result(const std::vector<T> & input,
                          std::vector<T> & output) {
            output.insert(output.end(), input.begin(), input.end());
        }

        template <typename T>
        void merge_result(const std::optional<T> & input,
                          std::vector<T> & output) {
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

    } // namespace

    Result::Result(){};

    tl::expected<Result, std::string> find_package(std::string_view name) {
        return find_package(name, {}, true);
    }

    tl::expected<Result, std::string>
    find_package(std::string_view name,
                 const std::vector<std::string> & components,
                 bool default_components) {
        // XXX: do we need process_requires here?
        auto && root = TRY(build_node(name, components, default_components));
        auto && flat = tsort(root);

        Result result{};

        result.version = root->data.package.version.value_or("unknown");

        for (auto && node : flat) {

            const auto && prefix_replacer =
                [&](const std::string & s) -> std::string {
                // TODO: Windows…
                auto && split = utils::split(s, "/");
                if (split[0] == "@prefix@") {
                    fs::path p = calculate_prefix(node->data.package.cps_path);
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
                assert_fn(
                    f != node->data.package.components.end(),
                    fmt::format("Could not find component {} of pacakge {}",
                                c_name, node->data.package.name));
                auto && comp = f->second;

                // Convert prefix at this point because:
                // 1. we are about to lose which CPS file the information came
                // from
                // 2. if we do it at the search point we have to plumb overrides
                // deep into that
                merge_result<loader::KnownLanguages, std::string>(
                    comp.includes, result.includes, prefix_replacer);
                merge_result(comp.defines, result.defines);
                merge_result(comp.compile_flags, result.compile_flags);
                merge_result(comp.link_libraries, result.link_libraries);
                if (comp.type != loader::Type::INTERFACE) {
                    result.link_location.emplace_back(prefix_replacer(
                        comp.link_location.value_or(comp.location.value())));
                }
            }
        }

        return result;
    }

} // namespace search
