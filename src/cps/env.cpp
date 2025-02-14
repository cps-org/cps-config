// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Tyler Weaver
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cstdlib>
#include <string>

#include "cps/env.hpp"
#include "cps/utils.hpp"

namespace cps {

    namespace {
        std::vector<fs::path> get_paths(const char * input) {
            const char * path_sep =
#ifdef _WIN32
                ";"
#else
                ":"
#endif
                ;
            std::vector<std::string> path_strs = utils::split(input, path_sep);
            auto result = std::vector<fs::path>{};
            std::transform(path_strs.begin(), path_strs.end(), std::back_inserter(result),
                           [](const std::string & s) { return fs::path{s}; });
            return result;
        }
    } // namespace

    Env get_env() {
        auto env = Env{};
        if (const char * env_c = std::getenv("CPS_PATH")) {
            env.cps_path = get_paths(env_c);
        }
        if (const char * env_c = std::getenv("CPS_PREFIX_PATH")) {
            // TODO: Windows
            env.cps_prefix_path = get_paths(env_c);
        }
        if (const char * env_c = std::getenv("PKG_CONFIG_PATH")) {
            env.pc_path = get_paths(env_c);
        }
        if (std::getenv("PKG_CONFIG_DEBUG_SPEW") || std::getenv("CPS_CONFIG_DEBUG_SPEW")) {
            env.debug_spew = true;
        }
        return env;
    }

} // namespace cps
