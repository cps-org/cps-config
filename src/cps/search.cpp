// SPDX-License-Identifier: MIT
// Copyright © 2023-2025 Dylan Baker
// Copyright © 2024 Bret Brown

#include "cps/search.hpp"

#include "cps/error.hpp"
#include "cps/loader.hpp"
#include "cps/pc_compat/pc_loader.hpp"
#include "cps/platform.hpp"
#include "cps/utils.hpp"
#include "cps/version.hpp"

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <tl/expected.hpp>

#include <algorithm>
#include <deque>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace fs = std::filesystem;

namespace cps::search {

    namespace {

        using version::to_string;

        /// @brief Details about how a required componenet should be used
        struct ComponentDetails {
            /// @brief If this component is only required for linking, not compiling
            bool link_only;
        };

        /// @brief A CPS file, along with the components in that CPS file to
        /// load
        class Dependency {
          public:
            Dependency(loader::Package && obj) : package{std::move(obj)} {};

            /// @brief The loaded CPS file
            loader::Package package;
            /// @brief the components from that CPS file to use
            std::unordered_map<std::string, ComponentDetails> components;
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

        const std::vector<fs::path> nix_prefix{"/usr", "/usr/local"};
        // TODO: const std::vector<std::string> mac_prefix{""};
        // TODO: const std::vector<std::string> win_prefix{""};

        enum class SearchPathType { cps, pc };

        struct SearchPath {
            fs::path path;
            SearchPathType type;
        };

        std::vector<SearchPath> cached_paths{};

        /// @brief expands a single search prefix into a set of full paths
        /// @param prefix the prefix to build from
        /// @return A vector of paths to search, in order
        std::vector<fs::path> expand_prefix(const fs::path & prefix) {
            std::vector<fs::path> paths{};

            // TODO: macOS specific paths
            // TODO: Windows specific paths

            // TODO: handle name-like search paths
            paths.emplace_back(prefix / platform::libdir() / "cps");
            paths.emplace_back(prefix / platform::datadir() / "cps");

            return paths;
        };

        template <typename T> void add_to_search_path(const std::vector<T> & paths, SearchPathType type) {
            std::transform(paths.begin(), paths.end(), std::back_inserter(cached_paths),
                           [&type](const auto & path) { return SearchPath{.path = path, .type = type}; });
        };

        /// @brief Expands CPS search prefixes into concrete paths
        /// @param env stored environment variables
        /// @return A vector of paths to search, in order
        std::vector<SearchPath> search_paths(const Env & env) {

            if (!cached_paths.empty()) {
                return cached_paths;
            }

            if (env.cps_path) {
                auto && paths = utils::split(env.cps_path.value());
                add_to_search_path(paths, SearchPathType::cps);
            }

            if (env.cps_prefix_path) {
                auto && prefixes = env.cps_prefix_path.value();
                for (auto && p : prefixes) {
                    auto && paths = expand_prefix(p);
                    add_to_search_path(paths, SearchPathType::cps);
                }
            }

            // If PKG_CONFIG_PATH is defined, search for PC files in the specified directly before falling back to
            // system default CPS and PC search paths.
            if (env.pc_path) {
                cached_paths.emplace_back(SearchPath{.path = *env.pc_path, .type = SearchPathType::pc});
            }

            for (auto && p : nix_prefix) {
                auto && paths = expand_prefix(p);
                add_to_search_path(paths, SearchPathType::cps);
                add_to_search_path(paths, SearchPathType::pc);
            }

            return cached_paths;
        }

        std::optional<fs::path> find_library_in_path(std::string_view name, const SearchPath & search_path) {
            if (fs::is_directory(search_path.path)) {
                const std::string extension = search_path.type == SearchPathType::cps ? "cps" : "pc";
                // TODO: <name-like>
                if (fs::path file = search_path.path / fmt::format("{}.{}", name, extension);
                    fs::is_regular_file(file)) {
                    return file;
                }
            }
            return std::nullopt;
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
            for (auto && search_path : paths) {
                if (auto file = find_library_in_path(name, search_path)) {
                    found.push_back(*file);
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

                // Assume file is CPS unless file extension is .pc
                auto n = std::make_shared<Node>(CPS_TRY(
                    path.extension() == ".pc" ? pc_compat::load(file, path.parent_path()) : loader::load(file, path)));

                cache.emplace(name, n);
                return n;
            }

          private:
            std::unordered_map<std::string, std::shared_ptr<Node>> cache;
        };

        tl::expected<std::shared_ptr<Node>, std::string>
        build_node(std::string_view name, const loader::Requirement & requirements, NodeFactory factory, Env env) {
            const std::vector<fs::path> paths = CPS_TRY(find_paths(name, env));
            std::vector<std::string> errors{};
            for (auto && path : paths) {
                auto maybe_node = factory.get(name, path);
                if (!maybe_node) {
                    errors.emplace_back(
                        fmt::format("CPS file for '{}', in path '{}', generated the following error: '{}'", name,
                                    path.string(), maybe_node.error()));
                    continue;
                }
                auto node = maybe_node.value();
                const loader::Package & p = node->data.package;

                // If this package doesn't meet the requirements then reject it and continue on.
                // The conditions it couldIf we  fail to meet are:
                //  1. the provided version (or Compat-Version) is < the required version
                //  2. This package lacks required components
                if (requirements.version) {
                    // From the CPS spec, version 0.12.0, for package::version,
                    // which as the same semantics as requirement::version:
                    //
                    // > If not provided, the CPS will not satisfy any request for
                    // > a specific version of the package.
                    if (!(p.version || p.compat_version)) {
                        errors.emplace_back(fmt::format("Tried {}, which does not specify a version or compat_version, "
                                                        "but the user requires version {}",
                                                        path.string(), requirements.version.value()));
                        continue;
                    }
                    // From the CPS spec, version 0.12.0, for package::compat_version
                    //
                    // > Specifies the oldest version of the package with which
                    // > this version is compatible. This information is used when
                    // > a consumer requests a specific version. If the version
                    // > requested is equal to or newer than the compat_version,
                    // > the package may be used.
                    //
                    // > If not specified, the package is not compatible with
                    // > previous versions (i.e. compat_version is implicitly
                    // > equal to version).
                    auto && v = version::compare(p.compat_version.value_or(p.version.value()), version::Operator::lt,
                                                 requirements.version.value(), p.version_schema);
                    if (!v) {
                        errors.emplace_back(fmt::format("{}: {}", path.string(), v.error()));
                        continue;
                    }

                    if (v.value()) {
                        errors.emplace_back(fmt::format(
                            "{} has a version of {}, which is less than the required {}, using the schema {}",
                            path.string(), p.compat_version.value_or(p.version.value()), requirements.version.value(),
                            to_string(p.version_schema)));
                        continue;
                    }
                }

                if (!std::all_of(requirements.components.begin(), requirements.components.end(),
                                 [p](const std::string & c) { return p.components.find(c) != p.components.end(); })) {
                    // TODO: more fine grained error message
                    errors.emplace_back(fmt::format("{} does not implement all of the required components '{}'",
                                                    path.string(), fmt::join(requirements.components, ", ")));
                    continue;
                }

                std::vector<std::shared_ptr<Node>> found;
                found.reserve(p.require.size());
                for (auto && [n, r] : p.require) {
                    auto && child = build_node(n, r, factory, env);

                    if (child) {
                        found.emplace_back(child.value());
                    } else {
                        errors.emplace_back(child.error());
                        break;
                    }
                }
                if (found.size() < p.require.size()) {
                    continue;
                }

                node->depends.insert(node->depends.end(), found.begin(), found.end());
                return node;
            }

            return tl::unexpected(fmt::format("{}:\n  {}", name, fmt::join(errors, "\n  ")));
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

        /// @brief Calculate the required components in the graph
        /// @param node The node to process
        /// @param components the components required from this node
        void set_components(std::shared_ptr<Node> node, std::vector<std::string> components, bool default_components,
                            bool link_only = false) {
            const auto & component_updater = [&node, &link_only](std::string name) {
                if (const auto & entry = node->data.components.find(name); entry != node->data.components.end()) {
                    entry->second.link_only |= link_only;
                } else {
                    node->data.components.emplace(std::move(name), ComponentDetails{link_only});
                }
            };

            // Set the components that this package's dependees want
            if (default_components && node->data.package.default_components) {
                const auto & defs = node->data.package.default_components.value();
                components.insert(components.end(), defs.begin(), defs.end());
            }
            // Then add all of the explicitly listed components
            std::for_each(components.begin(), components.end(), component_updater);

            // Handle self requirements first
            // This takes the form `"requires": [":a", ":b"]`
            // These must be handled before child dependencies, as they may alter the requirements placed on the
            // children..
            std::vector<std::pair<std::string, ComponentDetails>> self_requires;
            std::transform(node->data.components.begin(), node->data.components.end(),
                           std::back_insert_iterator(self_requires),
                           [](std::pair<std::string, ComponentDetails> entry) { return entry; });
            std::unordered_set<std::string> processed;
            bool self_defaults = default_components;
            while (!self_requires.empty()) {
                const auto [this_name, this_comp] = self_requires.back();
                self_requires.pop_back();
                if (processed.find(this_name) != processed.end()) {
                    continue;
                }
                processed.emplace(this_name);

                const loader::Component & component = node->data.package.components.at(this_name);
                auto && required = process_requires(component.require);
                if (auto && self = required.find(""); self != required.end()) {
                    // Don't insert these twice
                    std::vector<std::string> self_comps = self->second.components;
                    if (!self_defaults && self->second.defaults && node->data.package.default_components) {
                        self_defaults = true;
                        const std::vector<std::string> & defs = node->data.package.default_components.value();
                        self_comps.insert(self_comps.end(), defs.begin(), defs.end());
                    }
                    std::for_each(self_comps.begin(), self_comps.end(), component_updater);

                    for (auto && comp : self_comps) {
                        if (processed.find(comp) != processed.end()) {
                            continue;
                        }
                        self_requires.emplace_back(std::make_pair(comp, ComponentDetails{link_only}));
                    }
                }
            }

            // Walk the list of components for this component, adding component
            // requirements recursively for external requirements.
            for (const auto & [this_name, this_comp] : node->data.components) {
                // It's possible that the Package::Requires section listed
                // dependencies we don't actually need. If we don't need them we
                // can trim the graph
                std::vector<std::shared_ptr<Node>> trimmed;

                // This *should* be validated such that we won't have an exception
                const loader::Component & component = node->data.package.components.at(this_name);
                auto && required = process_requires(component.require);
                for (std::shared_ptr<Node> & child : node->depends) {
                    if (auto && child_comps = required.find(child->data.package.name); child_comps != required.end()) {
                        trimmed.emplace_back(child);
                        set_components(child, child_comps->second.components, child_comps->second.defaults, link_only);
                    }
                }
                auto && link_required = process_requires(component.link_requires);
                for (std::shared_ptr<Node> & child : node->depends) {
                    if (auto && child_comps = link_required.find(child->data.package.name);
                        child_comps != link_required.end()) {
                        trimmed.emplace_back(child);
                        set_components(child, child_comps->second.components, child_comps->second.defaults, true);
                    }
                }
                node->depends = trimmed;
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

        const auto prefix_path =
            prefix_variable.has_value() ? std::optional{fs::path{prefix_variable.value()}} : std::nullopt;

        for (auto && node : flat) {

            const auto prefix = prefix_path.value_or(node->data.package.prefix);

            const auto && prefix_replacer = [&](const std::string & s) -> std::string {
                // TODO: Windows…
                auto && split = utils::split(s, "/");
                if (split[0] == "@prefix@") {
                    fs::path p = prefix;
                    for (auto it = split.begin() + 1; it != split.end(); ++it) {
                        p /= *it;
                    }
                    return p.string();
                }
                return s;
            };

            for (const auto & [comp_name, cps_comp] : node->data.components) {
                // We should have already errored if this is not the case
                auto && f = node->data.package.components.find(comp_name);
                utils::assert_fn(
                    f != node->data.package.components.end(),
                    fmt::format("Could not find component {} of package {}", comp_name, node->data.package.name));
                auto && comp = f->second;

                // Convert prefix at this point because:
                // 1. we are about to lose which CPS file the information came
                // from
                // 2. if we do it at the search point we have to plumb overrides
                // deep into that
                if (!cps_comp.link_only) {
                    merge_result<loader::KnownLanguages, std::string>(comp.includes, result.includes, prefix_replacer);
                    merge_result(comp.definitions, result.definitions);
                    merge_result(comp.compile_flags, result.compile_flags);
                }
                merge_result(comp.link_libraries, result.link_libraries);
                merge_result(comp.link_flags, result.link_flags);
                if (comp.type != loader::Type::interface) {
                    if (!comp.location) {
                        return tl::make_unexpected(
                            fmt::format("Component `{}` requires 'location' attribute", comp_name));
                    }
                    result.link_location.emplace_back(
                        prefix_replacer(comp.link_location.value_or(comp.location.value())));
                }
            }
        }

        return result;
    }

} // namespace cps::search
