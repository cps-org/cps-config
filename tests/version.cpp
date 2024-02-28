// Copyright © 2024 Dylan Baker
// SPDX-License-Identifier: MIT

#include "version.hpp"
#include "utils.hpp"
#include <gtest/gtest.h>
#include <string>
#include <tuple>

using version::Operator;

namespace {
    std::string to_string(Operator op) {
        switch (op) {
        case Operator::GT:
            return ">";
        case Operator::GE:
            return ">=";
        case Operator::EQ:
            return "==";
        case Operator::NE:
            return "!=";
        case Operator::LE:
            return "<=";
        case Operator::LT:
            return "<";
        default:
            unreachable("How did you get here?");
        }
    }
} // namespace

class SimpleVersionTest : public ::testing::TestWithParam<std::tuple<std::string, Operator, std::string, bool>> {};

TEST_P(SimpleVersionTest, compare) {
    auto && [v1, op, v2, expected] = GetParam();
    auto && result = version::compare(v1, op, v2, version::Schema::SIMPLE);
    ASSERT_TRUE(result.has_value()) << "Unexpected error " << result.error();
    ASSERT_EQ(result.value(), expected) << "Case: " << v1 << " " << to_string(op) << " " << v2 << std::endl;
}

INSTANTIATE_TEST_SUITE_P(
    VersionTest, SimpleVersionTest,
    ::testing::Values(std::tuple("0", Operator::EQ, "0000000", true), std::tuple("0.0", Operator::EQ, "0", true),
                      std::tuple("0.0.0", Operator::EQ, "0", true), std::tuple("0.0.0", Operator::EQ, "0.0", true),
                      std::tuple("0.0.0", Operator::EQ, "1.0", false),
                      std::tuple("0.0.0", Operator::EQ, "0.1.0", false), std::tuple("0.0.0", Operator::GE, "0.0", true),
                      std::tuple("0.4.0", Operator::GE, "0.0", true), std::tuple("0.4.0", Operator::GE, "3.0", false),
                      std::tuple("0.4.0", Operator::GT, "0.0", true), std::tuple("0.0.0", Operator::GT, "3.0", false),
                      std::tuple("0.0.0", Operator::LE, "0.0", true), std::tuple("0.0.0", Operator::LE, "3.0", true),
                      std::tuple("6.0.0", Operator::LE, "3.0", false), std::tuple("0.0.0", Operator::LT, "3.0", true),
                      std::tuple("0.4.0", Operator::LT, "0.0", false), std::tuple("0.0.0", Operator::NE, "10.0", true),
                      std::tuple("0.0.0", Operator::NE, "0", false)));