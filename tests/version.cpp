// Copyright © 2024 Dylan Baker
// Copyright © 2024 Bret Brown
// SPDX-License-Identifier: MIT

#include "cps/version.hpp"

#include "cps/utils.hpp"

#include <gtest/gtest.h>

#include <string>

namespace cps::version::test {
    namespace {
        using version::Operator;
        std::string to_string(Operator op) {
            switch (op) {
            case Operator::gt:
                return ">";
            case Operator::ge:
                return ">=";
            case Operator::eq:
                return "==";
            case Operator::ne:
                return "!=";
            case Operator::le:
                return "<=";
            case Operator::lt:
                return "<";
            default:
                std::abort();
            }
        }

        class SimpleVersionTest
            : public ::testing::TestWithParam<std::tuple<std::string, Operator, std::string, bool>> {};

        TEST_P(SimpleVersionTest, compare) {
            auto && [v1, op, v2, expected] = GetParam();
            auto && result = version::compare(v1, op, v2, version::Schema::simple);
            ASSERT_TRUE(result.has_value()) << "Unexpected error " << result.error();
            ASSERT_EQ(result.value(), expected) << "Case: " << v1 << " " << to_string(op) << " " << v2 << std::endl;
        }

        INSTANTIATE_TEST_SUITE_P(
            VersionTest, SimpleVersionTest,
            ::testing::Values(
                std::tuple("0", Operator::eq, "0000000", true), std::tuple("0.0", Operator::eq, "0", true),
                std::tuple("0.0.0", Operator::eq, "0", true), std::tuple("0.0.0", Operator::eq, "0.0", true),
                std::tuple("0.0.0", Operator::eq, "1.0", false), std::tuple("0.0.0", Operator::eq, "0.1.0", false),
                std::tuple("0.0.0", Operator::ge, "0.0", true), std::tuple("0.4.0", Operator::ge, "0.0", true),
                std::tuple("0.4.0", Operator::ge, "3.0", false), std::tuple("0.4.0", Operator::gt, "0.0", true),
                std::tuple("0.0.0", Operator::gt, "3.0", false), std::tuple("0.0.0", Operator::le, "0.0", true),
                std::tuple("0.0.0", Operator::le, "3.0", true), std::tuple("6.0.0", Operator::le, "3.0", false),
                std::tuple("0.0.0", Operator::lt, "3.0", true), std::tuple("0.4.0", Operator::lt, "0.0", false),
                std::tuple("001.0.0", Operator::eq, "1", true), std::tuple("001.0.0+5", Operator::eq, "1", false),
                std::tuple("001.0.0-1", Operator::eq, "1-1", true), std::tuple("1+1", Operator::gt, "1", true),
                std::tuple("1-1", Operator::gt, "1", true), std::tuple("1+1", Operator::gt, "1", true),
                std::tuple("1+1", Operator::eq, "1-1", true), std::tuple("1+1", Operator::ge, "1-1", true),
                std::tuple("1+1", Operator::le, "1-1", true), std::tuple("1+1", Operator::ne, "1-1", false),
                std::tuple("1+1", Operator::lt, "1-1", false), std::tuple("1+1", Operator::gt, "1-1", false),
                std::tuple("001.0.0-1", Operator::eq, "1+001", true), std::tuple("0.0.0", Operator::ne, "10.0", true),
                std::tuple("0.0.0", Operator::ne, "0", false)));
    } // unnamed namespace
} // namespace cps::version::test
