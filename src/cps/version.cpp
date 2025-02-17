// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Bret Brown
// SPDX-License-Identifier: MIT

#include "cps/version.hpp"

#include "cps/error.hpp"
#include "cps/utils.hpp"

#include <fmt/core.h>

#include <cstdint>
#include <optional>

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

        tl::expected<std::tuple<std::string, std::optional<std::string>>, std::string>
        split_plus_minus(std::string_view raw) {
            std::vector<std::string> split;
            if (raw.find("+") != raw.npos) {
                if (raw.find("-") != raw.npos) {
                    return tl::make_unexpected("Found both a '-' and a '+' in a simple version");
                }
                split = utils::split(raw, "+");
                if (split.size() != 2) {
                    return tl::make_unexpected("More than 1 '+' in a simple version");
                }
                return std::make_tuple(split[0], split[1]);
            }
            if (raw.find("-") != raw.npos) {
                if (raw.find("+") != raw.npos) {
                    return tl::make_unexpected("Found both a '-' and a '+' in a simple version");
                }
                split = utils::split(raw, "-");
                if (split.size() != 2) {
                    return tl::make_unexpected("More than 1 '-' in a simple version");
                }
                return std::make_tuple(split[0], split[1]);
            }
            return std::make_tuple(std::string{raw}, std::nullopt);
        };

        enum comp_value {
            yes,
            no,
            unknown,
        };

        template <typename T> comp_value compare(const T & left, Operator op, const T & right) {
            switch (op) {
            case Operator::eq:
                if (left != right) {
                    return comp_value::no;
                }
                break;
            case Operator::le:
                if (left > right) {
                    return comp_value::no;
                }
                break;
            case Operator::ge:
                if (left < right) {
                    return comp_value::no;
                }
                break;
            case Operator::lt:
                if (left < right) {
                    return comp_value::yes;
                }
                break;
            case Operator::gt:
                if (left > right) {
                    return comp_value::yes;
                }
                break;
            case Operator::ne:
                if (left != right) {
                    return comp_value::yes;
                }
                break;
            }
            return comp_value::unknown;
        }

        tl::expected<comp_value, std::string> compare_numbers(std::string & l, Operator op, std::string_view r) {
            std::vector<uint64_t> left = CPS_TRY(as_numbers(l));
            std::vector<uint64_t> right = CPS_TRY(as_numbers(r));
            equalize_length(left, right);

            for (size_t i = 0; i < left.size(); ++i) {
                const uint64_t lv = left[i];
                const uint64_t rv = right[i];

                comp_value v = compare(lv, op, rv);
                if (v != comp_value::unknown) {
                    return v;
                }
            }
            return comp_value::unknown;
        }

        tl::expected<bool, std::string> simple_compare(std::string_view l, Operator op, std::string_view r) {
            auto [lfirst, lrest] = CPS_TRY(split_plus_minus(l));
            auto [rfirst, rrest] = CPS_TRY(split_plus_minus(r));

            switch (CPS_TRY(compare_numbers(lfirst, op, rfirst))) {
            case comp_value::yes:
                return true;
            case comp_value::no:
                return false;
            case comp_value::unknown:
                break;
            }

            // If we do have a rest (both have rest), then we need to compare those as well.
            if (rrest && lrest) {
                switch (CPS_TRY(compare_numbers(lrest.value(), op, rrest.value()))) {
                case comp_value::yes:
                    return true;
                case comp_value::no:
                    return false;
                case comp_value::unknown:
                    break;
                }
            } else {
                switch (compare(lrest.has_value(), op, rrest.has_value())) {
                case comp_value::yes:
                    return true;
                case comp_value::no:
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
