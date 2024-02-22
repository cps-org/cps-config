// SPDX-License-Identifier: MIT
// Copyright © 2023 Dylan Baker

#include "search.hpp"
#include "fmt/core.h"
#include "utils.hpp"
#include <cstdlib>
#include <string>
#include <vector>
#include <fmt/core.h>

namespace fs = std::filesystem;

namespace search {

    namespace {

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

    }

    tl::expected<fs::path, std::string> find_package(std::string_view name) {
        // If a path is passed, then just return that.
        if (fs::is_regular_file(name)) {
            return name;
        }

        // TODO: Need something like pkgconf's --personality option
        // TODO: we likely either need to return all possible files, or load a file
        // TODO: what to do about finding multiple versions of the same dependency?
        auto && paths = search_paths();
        for (auto && prefix : paths) {
            // TODO: <prefix>/<libdir>/cps/<name-like>/
            // TODO: <prefix>/share/cps/<name-like>/
            // TODO: <prefix>/share/cps/

            const fs::path dir = prefix / libdir() / "cps";
            if (fs::is_directory(dir)) {
                // TODO: <name-like>
                const fs::path file = dir / fmt::format("{}.cps", name);
                if (fs::is_regular_file(file)) {
                    return file;
                }
            }
        }

        return tl::unexpected(fmt::format("Could not find a CPS file for {}", name));
    }
} // namespace search