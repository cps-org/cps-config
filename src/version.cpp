// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#include "version.hpp"
#include "error.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cstdint>
#include <fmt/core.h>
#include <functional>

namespace version {

    namespace {

        tl::expected<std::vector<uint64_t>, std::string> as_numbers(std::string_view v) {
            std::vector<std::string> split = utils::split(v, ".");
            std::vector<uint64_t> left;
            left.reserve(split.size());
            for (auto && n : split) {
                try {
                    left.emplace_back(std::stoull(n));
                } catch (std::invalid_argument &) {
                    return tl::unexpected{fmt::format("'{}' is not a valid number", n)};
                } catch (std::out_of_range &) {
                    return tl::unexpected{fmt::format("'{}' is too large to be represented by a uint64. What "
                                                      "kind of versions are you creating?",
                                                      n)};
                }
            }
            return left;
        }

        inline void lengthen(std::vector<uint64_t> & target, uint64_t diff) {
            target.reserve(diff);
            for (uint64_t i = 0; i < diff; ++i) {
                target.emplace_back(0);
            }
        }

        inline void equalize_length(std::vector<uint64_t> & left, std::vector<uint64_t> & right) {
            if (left.size() < right.size()) {
                lengthen(left, right.size() - left.size());
            } else if (left.size() > right.size()) {
                lengthen(right, left.size() - right.size());
            }
        }

        tl::expected<bool, std::string> simple_compare(std::string_view l, Operator op, std::string_view r) {
            // TODO: handle the -.* or +.* ending
            // TODO: 32 bit probably needs stoull…
            std::vector<uint64_t> left = TRY(as_numbers(l));
            std::vector<uint64_t> right = TRY(as_numbers(r));
            equalize_length(left, right);

            // TODO: this is so ugly
            for (size_t i = 0; i < left.size(); ++i) {
                const uint64_t lv = left[i];
                const uint64_t rv = right[i];

                switch (op) {
                case Operator::EQ:
                    if (lv != rv) {
                        return false;
                    }
                    break;
                case Operator::LE:
                    if (lv > rv) {
                        return false;
                    }
                    break;
                case Operator::GE:
                    if (lv < rv) {
                        return false;
                    }
                    break;
                case Operator::LT:
                    if (lv < rv) {
                        return true;
                    }
                    break;
                case Operator::GT:
                    if (lv > rv) {
                        return true;
                    }
                    break;
                case Operator::NE:
                    if (lv != rv) {
                        return true;
                    }
                    break;
                }
            }

            return (op == Operator::EQ || op == Operator::LE || op == Operator::GE);
        }

    } // namespace

    tl::expected<bool, std::string> compare(std::string_view left, Operator op, std::string_view right, Schema schema) {
        switch (schema) {
        case Schema::SIMPLE:
            return simple_compare(left, op, right);
        default:
            unreachable("Only the simple schema is implemented");
        }
    }

} // namespace version