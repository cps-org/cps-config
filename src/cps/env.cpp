// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Tyler Weaver
// SPDX-License-Identifier: MIT

#include "cps/env.hpp"

#include <cstdlib>
#include <string>

namespace cps {

    Env get_env() {
        auto env = Env{};
        if (const char * env_c = std::getenv("CPS_PATH")) {
            env.cps_path = std::string(env_c);
        }
        if (const char * env_c = std::getenv("CPS_PREFIX_PATH")) {
            env.cps_prefix_path = std::string(env_c);
        }
        if (std::getenv("PKG_CONFIG_DEBUG_SPEW") || std::getenv("CPS_CONFIG_DEBUG_SPEW")) {
            env.debug_spew = true;
        }
        return env;
    }

} // namespace cps
