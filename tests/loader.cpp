// Copyright © 2024-2025 Dylan Baker
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

        TEST(Loader, minimal_complete_package) {
            std::stringstream ss(R"({
    "name": "minimal_complete_package",
    "cps_version": "0.12.0",
    "prefix": "/sentinel/",
    "components": {
        "default": {
            "type": "archive",
            "location": "/"
        }
    }
}
)"s);
            auto const package = cps::loader::load(ss, "minimal_complete_package");
            ASSERT_TRUE(package.has_value()) << "should have parsed, found error: " << package.error();
        }

        TEST(Loader, components_must_not_be_empty) {
            std::stringstream ss(R"({
    "name": "components_must_not_be_empty",
    "cps_version": "0.12.0",
    "prefix": "/sentinel/",
    "components": {}
}
)"s);
            auto const package = cps::loader::load(ss, "components_must_not_be_empty");
            ASSERT_FALSE(package.has_value())
                << "should not have parsed, components is required to have atleast one entry";
        }

        TEST(Loader, archive_missing_location) {
            std::stringstream ss(R"({
    "name": "archive_missing_location",
    "cps_version": "0.12.0",
    "prefix": "/sentinel/",
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

        TEST(Loader, missing_name) {
            std::stringstream ss(R"({
    "cps_version": "0.12.0",
    "prefix": "/sentinel/",
    "components": {
        "default": {
            "type": "archive",
            "location": "/",
        }
    }
}
)"s);
            auto const package = cps::loader::load(ss, "missing_name");
            ASSERT_FALSE(package.has_value()) << "should not have parsed, root requires `name` attribute";
        }

        TEST(Loader, missing_cps_version) {
            std::stringstream ss(R"({
    "name": "missing_cps_version",
    "prefix": "/sentinel/",
    "components": {
        "default": {
            "type": "archive",
            "location": "/",
        }
    }
}
)"s);
            auto const package = cps::loader::load(ss, "missing_cps_version");
            ASSERT_FALSE(package.has_value()) << "should not have parsed, root requires `cps_version` attribute";
        }

        TEST(Loader, missing_components) {
            std::stringstream ss(R"({
    "name": "missing_components",
    "prefix": "/sentinel/",
    "cps_version": "0.12.0",
}
)"s);
            auto const package = cps::loader::load(ss, "missing_components");
            ASSERT_FALSE(package.has_value()) << "should not have parsed, root requires `components` attribute";
        }

        TEST(Loader, name_is_string) {
            std::stringstream ss(R"({
    "name": [],
    "prefix": "/sentinel/",
    "cps_version": "0.12.0",
    "components": {
        "default": {
            "type": "archive",
            "location": "/",
        }
    }
}
)"s);
            auto const package = cps::loader::load(ss, "name_is_string");
            ASSERT_FALSE(package.has_value()) << "should not have parsed, root requires `name` value to be string";
        }

        TEST(Loader, cps_version_is_string) {
            std::stringstream ss(R"({
    "name": "cps_version_is_string",
    "prefix": "/sentinel/",
    "cps_version": [],
    "components": {
        "default": {
            "type": "archive",
            "location": "/",
        }
    }
}
)"s);
            auto const package = cps::loader::load(ss, "cps_version_is_string");
            ASSERT_FALSE(package.has_value())
                << "should not have parsed, root requires `cps_version` value to be string";
        }

        TEST(Loader, components_is_object) {
            std::stringstream ss(R"({
    "name": "components_is_object",
    "prefix": "/sentinel/",
    "cps_version": "0.12.0",
    "components": [],
}
)"s);
            auto const package = cps::loader::load(ss, "components_is_object");
            ASSERT_FALSE(package.has_value())
                << "should not have parsed, root requires `components` value to be object";
        }

        TEST(Loader, component_type_is_string) {
            std::stringstream ss(R"({
    "name": "component_type_is_string",
    "prefix": "/sentinel/",
    "cps_version": [],
    "components": {
        "default": {
            "type": [],
            "location": "/",
        }
    }
}
)"s);
            auto const package = cps::loader::load(ss, "component_type_is_string");
            ASSERT_FALSE(package.has_value())
                << "should not have parsed, root requires `components.<name>.type` value to be string";
        }

        TEST(Loader, component_location_is_string) {
            std::stringstream ss(R"({
    "name": "component_location_is_string",
    "prefix": "/sentinel/",
    "cps_version": [],
    "components": {
        "default": {
            "type": "archive",
            "location": [],
        }
    }
}
)"s);
            auto const package = cps::loader::load(ss, "component_location_is_string");
            ASSERT_FALSE(package.has_value())
                << "should not have parsed, root requires `components.<name>.location` value to be string";
        }

        TEST(Loader, cps_version_is_0_10_0) {
            std::stringstream ss(R"({
    "name": "cps_version_is_0_10_0",
    "prefix": "/sentinel/",
    "cps_version": "0.9.0",
    "components": {
        "default": {
            "type": "archive",
            "location": "/",
        }
    }
}
)"s);
            auto const package = cps::loader::load(ss, "cps_version_is_0_10_0");
            ASSERT_FALSE(package.has_value())
                << "should not have parsed, root requires `cps_version` value to exactly `0.12.0`";
        }

        TEST(Loader, valid_component_types) {
            std::stringstream ss(R"({
    "name": "valid_component_types",
    "prefix": "/sentinel/",
    "cps_version": "0.12.0",
    "components": {
        "a": {
            "type": "archive",
            "location": "/"
        },
        "b": {
            "type": "dylib"
        },
        "c": {
            "type": "module"
        },
        "d": {
            "type": "jar"
        },
        "e": {
            "type": "interface"
        },
        "f": {
            "type": "symbolic"
        },
        "g": {
            "type": "executable"
        }
    }
}
)"s);
            auto const package = cps::loader::load(ss, "valid_component_types");
            ASSERT_TRUE(package.has_value()) << "should have parsed, found error: " << package.error();
            ASSERT_EQ(package->components.size(), 7) << "should have found 7 different components";
        }

        TEST(Loader, valid_component_type_extension) {
            std::stringstream ss(R"({
    "name": "valid_component_type_extension",
    "prefix": "/sentinel/",
    "cps_version": "0.12.0",
    "components": {
        "a": {
            "type": "archive",
            "location": "/"
        },
        "b": {
            "type": "not_recognized_type_is_valid_but_ignored"
        }
    }
}
)"s);
            auto const package = cps::loader::load(ss, "valid_component_type_extension");
            ASSERT_TRUE(package.has_value())
                << "should have parsed, found error: " << package.error() << '\n'
                << "If the type is not recognized by the parser, the component shall be ignored. (Parsers are "
                   "permitted to support additional types as a conforming extension.)";
        }

        TEST(Loader, not_recognized_type_is_valid_but_ignored_can_make_empty_component) {
            std::stringstream ss(R"({
    "name": "not_recognized_type_is_valid_but_ignored_can_make_empty_component",
    "prefix": "/sentinel/",
    "cps_version": "0.12.0",
    "components": {
        "a": {
            "type": "not_recognized_type_is_valid_but_ignored",
        },
    }
}
)"s);
            auto const package =
                cps::loader::load(ss, "not_recognized_type_is_valid_but_ignored_can_make_empty_component");
            ASSERT_FALSE(package.has_value())
                << "should not have parsed, components only has non-standard type entry making it empty";
        }

        TEST(Loader, cps_path_and_prefix_cannot_both_be_set) {
            std::stringstream ss(R"({
    "name": "cps_path_and_prefix_cannot_both_be_set",
    "cps_version": "0.12.0",
    "cps_path": "/some/path",
    "prefix": "/prefix",
    "components": {
        "a": {
            "type": "archive",
            "location": "/"
        }
    }
    })"s);
            auto const package = cps::loader::load(ss, "cps_path_and_prefix_cannot_both_be_set");
            ASSERT_FALSE(package.has_value()) << "should not have parsed, prefix and cps_path cannot both be set"
                                              << "actual error:" << package.error();
        }

        TEST(Loader, one_of_cps_path_and_prefix_must_be_set) {
            std::stringstream ss(R"({
    "name": "one_of_cps_path_and_prefix_must_be_set",
    "cps_version": "0.12.0",
    "components": {
        "a": {
            "type": "archive",
            "location": "/"
        }
    }
    })"s);
            auto const package = cps::loader::load(ss, "one_of_cps_path_and_prefix_must_be_Set");
            ASSERT_FALSE(package.has_value()) << "should not have parsed, either prefix or cps_path must be set. "
                                              << "actual error:" << package.error();
        }

    } // unnamed namespace
} // namespace cps::utils::test
