// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Bret Brown
// SPDX-License-Identifier: MIT

#include "cps/loader.hpp"

#include "cps/error.hpp"
#include "cps/utils.hpp"

#include <algorithm>
#include <fmt/core.h>
#include <json/json.h>
#include <optional>
#include <tl/expected.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace cps::loader {

    namespace {

        constexpr static std::string_view CPS_VERSION = "0.10.0";

        template <typename T>
        tl::expected<std::optional<T>, std::string>
        get_optional(const Json::Value & parent, std::string_view parent_name, const std::string & name) {
            // It's okay for a member to be missing from an optional value
            if (!parent.isMember(name)) {
                return std::nullopt;
            }
            const Json::Value value = parent[name];

            if constexpr (std::is_same_v<T, std::string>) {
                if (value.isString()) {
                    return value.asString();
                }
            } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                if (value.isArray()) {
                    std::vector<std::string> ret;
                    for (auto && v : value) {
                        // TODO: Error handling?
                        ret.emplace_back(v.asString());
                    }
                    return ret;
                }
            }

            return tl::unexpected(
                fmt::format("Optional field `{}` in `{}` is not of type `{}`!", name, parent_name, typeid(T).name()));
        };

        template <typename T>
        tl::expected<T, std::string> get_required(const Json::Value & parent, std::string_view parent_name,
                                                  const std::string & name) {
            if (!parent.isMember(name)) {
                // TODO: it would be nice to have the parent name…
                return tl::unexpected(fmt::format("Required field `{}` in `{}` is missing!", name, parent_name));
            }

            return get_optional<T>(parent, parent_name, name).and_then([](auto && v) -> tl::expected<T, std::string> {
                if (v)
                    return v.value();
                return "bad";
            });
            // TODO: also need to fixup error message for "Optional type ..."
        }

        Type string_to_type(std::string_view str) {
            if (str == "executable") {
                return Type::executable;
            }
            if (str == "archive") {
                return Type::archive;
            }
            if (str == "dylib") {
                return Type::dylib;
            }
            if (str == "module") {
                return Type::module;
            }
            if (str == "jar") {
                return Type::jar;
            }
            if (str == "interface") {
                return Type::interface;
            }
            if (str == "symbolic") {
                return Type::symbolic;
            }
            return Type::unknown;
        }

        version::Schema string_to_schema(std::string_view str) {
            if (str == "simple") {
                return version::Schema::simple;
            }
            if (str == "rpm") {
                return version::Schema::rpm;
            }
            if (str == "dpkg") {
                return version::Schema::dpkg;
            }
            if (str == "custom") {
                return version::Schema::custom;
            }
            CPS_UNREACHABLE(fmt::format("Unknown version schema: `{}`", str).c_str());
        }

        template <>
        tl::expected<LangValues, std::string>
        get_required<LangValues>(const Json::Value & parent, std::string_view parent_name, const std::string & name) {
            LangValues ret{};
            if (!parent.isMember(name)) {
                return ret;
            }

            Json::Value value = parent[name];
            if (value.isObject()) {
                // TODO: simplify this further, maybe with a loop?
                auto && cb = [](auto && r) { return r.value_or(std::vector<std::string>{}); };
                ret[KnownLanguages::c] = CPS_TRY(get_optional<std::vector<std::string>>(value, name, "c").map(cb));
                ret[KnownLanguages::cxx] = CPS_TRY(get_optional<std::vector<std::string>>(value, name, "c++").map(cb));
                ret[KnownLanguages::fortran] =
                    CPS_TRY(get_optional<std::vector<std::string>>(value, name, "fortran").map(cb));
            } else if (value.isArray()) {
                std::vector<std::string> fin;
                for (auto && v : value) {
                    fin.emplace_back(v.asString());
                }
                for (auto && v : {KnownLanguages::c, KnownLanguages::cxx, KnownLanguages::fortran}) {
                    ret.emplace(v, fin);
                }
            } else {
                return tl::unexpected(
                    fmt::format("Section `{}` of `{}` is neither an object nor an array!", parent_name, name));
            }
            return ret;
        }

        template <>
        tl::expected<Defines, std::string>
        get_required<Defines>(const Json::Value & parent, std::string_view parent_name, const std::string & name) {
            LangValues && lang = CPS_TRY(get_required<LangValues>(parent, parent_name, name));
            Defines ret;
            for (auto && [k, values] : lang) {
                ret[k] = {};
                for (auto && value : values) {
                    if (value.front() == '!') {
                        ret[k].emplace_back(Define{value.substr(1), false});
                    } else if (const size_t sep = value.find("="); sep != value.npos) {
                        std::string dkey = value.substr(0, sep);
                        std::string dvalue = value.substr(sep + 1);
                        ret[k].emplace_back(Define{dkey, dvalue});
                    } else {
                        ret[k].emplace_back(Define{value});
                    }
                }
            }
            return ret;
        };

        template <>
        tl::expected<Requires, std::string>
        get_required<Requires>(const Json::Value & parent, std::string_view parent_name, const std::string & name) {
            Requires ret{};
            if (!parent.isMember(name)) {
                return ret;
            }

            Json::Value require = parent[name];
            if (!require.isObject()) {
                return tl::unexpected(fmt::format("`{}` field of `{}` is not an object", name, parent_name));
            }

            for (auto && itr = require.begin(); itr != require.end(); ++itr) {
                // TODO: error handling for not a string?
                const std::string key = itr.key().asString();
                const Json::Value obj = *itr;

                ret.emplace(key, Requirement{
                                     CPS_TRY(get_optional<std::vector<std::string>>(require, name, "components"))
                                         .value_or(std::vector<std::string>{}),
                                     CPS_TRY(get_optional<std::string>(require, name, "version")),
                                 });
            }

            return ret;
        };

        using Components = std::unordered_map<std::string, Component>;

        template <>
        tl::expected<Components, std::string>
        get_required<Components>(const Json::Value & parent, std::string_view parent_name, const std::string & name) {
            Json::Value compmap;
            if (!parent.isMember(name)) {
                return tl::unexpected(fmt::format("Required field `components` of `{}` is missing!", parent_name));
            }

            std::unordered_map<std::string, Component> components{};

            // TODO: error handling for not an object
            compmap = parent[name];
            if (!compmap.isObject()) {
                return tl::unexpected(fmt::format("`{}` field of `{}` is not an object", name, parent_name));
            }
            if (compmap.empty()) {
                return tl::unexpected(fmt::format("Components field of `{}` is empty, but must "
                                                  "have at least one component",
                                                  parent_name));
            }

            for (auto && itr = compmap.begin(); itr != compmap.end(); ++itr) {
                // TODO: Error handling for not a string?
                const std::string key = itr.key().asString();
                const Json::Value comp = *itr;

                if (!comp.isObject()) {
                    return tl::unexpected(fmt::format("`{}` `{}` is not an object", name, key));
                }

                auto const type = CPS_TRY(get_required<std::string>(comp, name, "type").map(string_to_type));
                auto const compile_flags = CPS_TRY(get_required<LangValues>(comp, name, "compile_flags"));
                auto const includes = CPS_TRY(get_required<LangValues>(comp, name, "includes"));
                auto const defines = CPS_TRY(get_required<Defines>(comp, name, "defines"));
                auto const link_flags = CPS_TRY(get_optional<std::vector<std::string>>(comp, name, "link_flags"))
                                            .value_or(std::vector<std::string>{});
                auto const link_libraries =
                    CPS_TRY(get_optional<std::vector<std::string>>(comp, name, "link_libraries"))
                        .value_or(std::vector<std::string>{});
                auto const location = CPS_TRY(get_optional<std::string>(comp, name, "location"));
                auto const link_location = CPS_TRY(get_optional<std::string>(comp, name, "link_location"));
                auto const require = CPS_TRY(get_optional<std::vector<std::string>>(comp, name, "requires"))
                                         .value_or(std::vector<std::string>{});

                if (type == Type::unknown) {
                    continue;
                }
                if (type == Type::archive && !location.has_value()) {
                    return tl::make_unexpected(
                        fmt::format("component `{}` of type `archive` missing required key `location`", key));
                }
                // TODO: Validate link_location, see https://github.com/cps-org/cps/issues/34

                components[key] = Component{
                    .type = std::move(type),
                    .compile_flags = std::move(compile_flags),
                    .includes = std::move(includes),
                    .defines = std::move(defines),
                    .link_flags = std::move(link_flags),
                    .link_libraries = std::move(link_libraries),
                    .location = std::move(location),
                    .link_location = std::move(link_location),
                    .require = std::move(require),
                };
            }

            return components;
        };

    } // namespace

    Define::Define(std::string name_) : name{std::move(name_)}, value{}, define{true} {};
    Define::Define(std::string name_, std::string value_)
        : name{std::move(name_)}, value{std::move(value_)}, define{true} {};
    Define::Define(std::string name_, bool define_) : name{std::move(name_)}, value{}, define{define_} {};

    bool Define::is_undefine() const { return !define; }

    bool Define::is_define() const { return define && value.empty(); }

    std::string Define::get_name() const { return name; }
    std::string Define::get_value() const { return value; }

    Configuration::Configuration() = default;
    Configuration::Configuration(LangValues cflags) : compile_flags{std::move(cflags)} {};

    Requirement::Requirement() = default;
    Requirement::Requirement(std::vector<std::string> comps) : components{std::move(comps)} {};
    Requirement::Requirement(std::vector<std::string> && comps, std::optional<std::string> && ver)
        : components{std::move(comps)}, version{std::move(ver)} {};

    Platform::Platform() = default;

    tl::expected<Package, std::string> load(std::istream & input_buffer, std::string const & filename) {
        Json::Value root;
        try {
            input_buffer >> root;
        } catch (std::exception const & ex) {
            return tl::make_unexpected(
                fmt::format("Exception caught while parsing json for `{}.cps`\n{}", filename, ex.what()));
        }

        auto const name = CPS_TRY(get_required<std::string>(root, "package", "name"));
        auto const cps_version = CPS_TRY(get_required<std::string>(root, "package", "cps_version"));
        auto const components = CPS_TRY(get_required<Components>(root, "package", "components"));
        auto const cps_path = CPS_TRY(get_optional<std::string>(root, "package", "cps_path")).value_or(filename);
        auto const default_components =
            CPS_TRY(get_optional<std::vector<std::string>>(root, "package", "default_components"));
        auto const platform = std::nullopt; // TODO: parse platform
        auto const require = CPS_TRY(get_requires(root, "package", "requires"));
        auto const version = CPS_TRY(get_optional<std::string>(root, "package", "version"));
        auto const version_schema =
            CPS_TRY(get_optional<std::string>(root, "package", "version_schema").map([](auto && v) {
                return string_to_schema(v.value_or("simple"));
            }));

        if (cps_version != CPS_VERSION) {
            return tl::make_unexpected(fmt::format("cps-config only supports CPS_VERSION `{}` found `{}` in `{}`",
                                                   CPS_VERSION, cps_version, name));
        }

        return Package{
            .name = std::move(name),
            .cps_version = std::move(cps_version),
            .components = std::move(components),
            .cps_path = std::move(cps_path),
            .default_components = std::move(default_components),
            .platform = std::move(platform),
            .require = std::move(require), // requires is a keyword
            .version = std::move(version),
            .version_schema = std::move(version_schema),
        };
    }
} // namespace cps::loader
