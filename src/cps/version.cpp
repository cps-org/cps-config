// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Bret Brown
// SPDX-License-Identifier: MIT

#include "cps/version.hpp"

#include "cps/error.hpp"
#include "cps/utils.hpp"

#include <fmt/core.h>

#include <cstdint>

namespace cps::version {

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

        enum comp_value {
            eq,
            ne,
            unknown,
        };

        comp_value compare(const uint64_t & left, Operator op, const uint64_t & right) {
            switch (op) {
            case Operator::eq:
                if (left != right) {
                    return comp_value::ne;
                }
                break;
            case Operator::le:
                if (left > right) {
                    return comp_value::ne;
                }
                break;
            case Operator::ge:
                if (left < right) {
                    return comp_value::ne;
                }
                break;
            case Operator::lt:
                if (left < right) {
                    return comp_value::eq;
                }
                break;
            case Operator::gt:
                if (left > right) {
                    return comp_value::eq;
                }
                break;
            case Operator::ne:
                if (left != right) {
                    return comp_value::eq;
                }
                break;
            }
            return comp_value::unknown;
        }

        tl::expected<bool, std::string> simple_compare(std::string_view l, Operator op, std::string_view r) {
            // TODO: handle the -.* or +.* ending
            std::vector<uint64_t> left = CPS_TRY(as_numbers(l));
            std::vector<uint64_t> right = CPS_TRY(as_numbers(r));
            equalize_length(left, right);

            // TODO: this is so ugly
            for (size_t i = 0; i < left.size(); ++i) {
                const uint64_t lv = left[i];
                const uint64_t rv = right[i];

                switch (compare(lv, op, rv)) {
                case comp_value::eq:
                    return true;
                case comp_value::ne:
                    return false;
                case comp_value::unknown:
                    break;
                }
            }

            return (op == Operator::eq || op == Operator::le || op == Operator::ge);
        }

    } // namespace

    std::string to_string(const Schema schema) {
        switch (schema) {
        case Schema::simple:
            return "simple";
        case Schema::dpkg:
            return "dpkg";
        case Schema::rpm:
            return "rpm";
        case Schema::custom:
            return "custom";
        default:
            abort();
        };
    }

    tl::expected<bool, std::string> compare(std::string_view left, Operator op, std::string_view right, Schema schema) {
        switch (schema) {
        case Schema::simple:
            return simple_compare(left, op, right);
        default:
            return tl::unexpected{fmt::format("The {} schema is not implemented", to_string(schema))};
        }
    }

} // namespace cps::version
