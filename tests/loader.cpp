// Copyright © 2024 Dylan Baker
// Copyright © 2024 Bret Brown
// Copyright © 2024 Tyler Weaver
// SPDX-License-Identifier: MIT

#include "cps/loader.hpp"
#include <gtest/gtest.h>
#include <sstream>

using namespace std::string_literals;

namespace cps::utils::test {
    namespace {

        TEST(Loader, empty) {
            std::stringstream ss("");
            auto const package = cps::loader::load(ss, "empty");
            ASSERT_FALSE(package.has_value()) << package.error();
        }

        TEST(Loader, empty_json) {
            std::stringstream ss("{}");
            auto const package = cps::loader::load(ss, "empty_json");
            ASSERT_FALSE(package.has_value()) << package.error();
        }

        TEST(Loader, archive_missing_location) {
            std::stringstream ss(R"({
    "name": "archive_missing_location",
    "cps_version": "0.10.0",
    "components": {
        "default": {
            "type": "archive",
        }
    }
}
)"s);
            auto const package = cps::loader::load(ss, "archive_missing_location");
            ASSERT_FALSE(package.has_value())
                << "should not have parsed, archive component requires `location` attribute";
        }

    } // unnamed namespace
} // namespace cps::utils::test
