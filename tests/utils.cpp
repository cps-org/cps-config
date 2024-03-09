// Copyright © 2024 Dylan Baker
// Copyright © 2024 Bret Brown
// SPDX-License-Identifier: MIT

#include "cps/utils.hpp"
#include <gtest/gtest.h>

namespace cps::utils::test {
    namespace {

        TEST(SplitTest, nothing) {
            const std::vector<std::string> expected{"val"};
            const std::vector<std::string> actual = utils::split("val", ".");
            ASSERT_EQ(actual, expected);
        }

        TEST(SplitTest, common) {
            const std::vector<std::string> expected{"a", "b"};
            const std::vector<std::string> actual = utils::split("a:b");
            ASSERT_EQ(actual, expected);
        }

        TEST(SplitTest, multi) {
            const std::vector<std::string> expected{"a", "b", "c"};
            const std::vector<std::string> actual = utils::split("a:b:c");
            ASSERT_EQ(actual, expected);
        }

        TEST(SplitTest, start_delim) {
            const std::vector<std::string> expected{"", "a"};
            const std::vector<std::string> actual = utils::split(":a");
            ASSERT_EQ(actual, expected);
        }

        TEST(SplitTest, delim) {
            const std::vector<std::string> expected{"a", "b", "c"};
            const std::vector<std::string> actual = utils::split("a.b.c", ".");
            ASSERT_EQ(actual, expected);
        }

    } // unnamed namespace
} // namespace cps::utils::test