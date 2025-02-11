// Copyright © 2023-2025 Dylan Baker
// Copyright © 2024 Bret Brown
// SPDX-License-Identifier: MIT

#include "cps/loader.hpp"

#include "cps/error.hpp"

#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <tl/expected.hpp>

#include <filesystem>
#include <iostream>
#include <optional>

namespace cps::loader {

    namespace {

        namespace fs = std::filesystem;

        template <typename T>
        tl::expected<std::optional<T>, std::string>
        get_optional(const nlohmann::json & parent, std::string_view parent_name, const std::string & name) {
            // It's okay for a member to be missing from an optional value
            if (!parent.contains(name)) {
                return std::nullopt;
            }
            const nlohmann::json & value = parent[name];

            if constexpr (std::is_same_v<T, std::string>) {
                if (value.is_string()) {
                    return value.get<std::string>();
                }
            } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                if (value.is_array()) {
                    std::vector<std::string> ret;
                    for (auto && v : value) {
                        // TODO: Error handling?
                        ret.emplace_back(v.get<std::string>());
                    }
                    return ret;
                }
            }

            return tl::unexpected(
                fmt::format("Optional field `{}` in `{}` is not of type `{}`!", name, parent_name, typeid(T).name()));
        };

        template <typename T>
        tl::expected<T, std::string> get_required(const nlohmann::json & parent, std::string_view parent_name,
                                                  const std::string & name) {
            if (!parent.contains(name)) {
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
            fmt::print(stderr, "Unknown version schema: `{}`", str);
            std::abort();
        }

        template <>
        tl::expected<LangValues, std::string> get_required<LangValues>(const nlohmann::json & parent,
                                                                       std::string_view parent_name,
                                                                       const std::string & name) {
            LangValues ret{};
            if (!parent.contains(name)) {
                return ret;
            }

            const nlohmann::json & value = parent[name];
            if (value.is_object()) {
                auto && fallback = CPS_TRY(get_optional<std::vector<std::string>>(value, name, "*"))
                                       .value_or(std::vector<std::string>{});
                ret[KnownLanguages::c] =
                    CPS_TRY(get_optional<std::vector<std::string>>(value, name, "c")).value_or(fallback);
                ret[KnownLanguages::cxx] =
                    CPS_TRY(get_optional<std::vector<std::string>>(value, name, "c++")).value_or(fallback);
                ret[KnownLanguages::fortran] =
                    CPS_TRY(get_optional<std::vector<std::string>>(value, name, "fortran")).value_or(fallback);
            } else if (value.is_array()) {
                std::vector<std::string> fin;
                for (auto && v : value) {
                    fin.emplace_back(v.get<std::string>());
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
        get_required<Defines>(const nlohmann::json & parent, std::string_view parent_name, const std::string & name) {
            Defines ret;
            if (!parent.contains(name)) {
                return ret;
            }

            const nlohmann::json & defines = parent[name];
            if (!defines.is_object()) {
                return tl::unexpected(fmt::format("Section `{}` of `{}` is not an object", parent_name, name));
            }

            const auto getter =
                [&](const std::string & lang) -> tl::expected<std::optional<std::vector<Define>>, std::string> {
                if (!defines.contains(lang)) {
                    return std::nullopt;
                }

                std::vector<Define> ret2;
                for (auto && [k, v] : defines.at(lang).items()) {
                    if (v.is_null()) {
                        ret2.emplace_back(Define{k});
                        continue;
                    }
                    if (!v.is_string()) {
                        return tl::unexpected(fmt::format(
                            "key `{}` of language `{}` of section `{}` of `{}` has a value that is not a string", k,
                            lang, parent_name, name));
                    }
                    ret2.emplace_back(Define{k, v.get<std::string>()});
                }

                return ret2;
            };

            auto && fallback = CPS_TRY(getter("*")).value_or(std::vector<Define>{});
            ret[KnownLanguages::c] = CPS_TRY(getter("c")).value_or(fallback);
            ret[KnownLanguages::cxx] = CPS_TRY(getter("cxx")).value_or(fallback);
            ret[KnownLanguages::fortran] = CPS_TRY(getter("fortran")).value_or(fallback);

            return ret;
        };

        template <>
        tl::expected<Requires, std::string>
        get_required<Requires>(const nlohmann::json & parent, std::string_view parent_name, const std::string & name) {
            Requires ret{};
            if (!parent.contains(name)) {
                return ret;
            }

            nlohmann::json require = parent[name];
            if (!require.is_object()) {
                return tl::unexpected(fmt::format("`{}` field of `{}` is not an object", name, parent_name));
            }

            for (const auto & item : require.items()) {
                // TODO: error handling for not a string?
                const std::string key = item.key();
                const nlohmann::json & obj = item.value();

                ret.emplace(key, Requirement{
                                     CPS_TRY(get_optional<std::vector<std::string>>(obj, name, "components"))
                                         .value_or(std::vector<std::string>{}),
                                     CPS_TRY(get_optional<std::string>(obj, name, "version")),
                                 });
            }

            return ret;
        };

        using Components = std::unordered_map<std::string, Component>;

        template <>
        tl::expected<Components, std::string> get_required<Components>(const nlohmann::json & parent,
                                                                       std::string_view parent_name,
                                                                       const std::string & name) {
            if (!parent.contains(name)) {
                return tl::unexpected(fmt::format("Required field `components` of `{}` is missing!", parent_name));
            }

            std::unordered_map<std::string, Component> components{};

            // TODO: error handling for not an object
            nlohmann::json compmap = parent[name];
            if (!compmap.is_object()) {
                return tl::unexpected(fmt::format("`{}` field of `{}` is not an object", name, parent_name));
            }

            for (const auto & item : compmap.items()) {
                // TODO: Error handling for not a string?
                const std::string key = item.key();
                const nlohmann::json & comp = item.value();

                if (!comp.is_object()) {
                    return tl::unexpected(fmt::format("`{}` `{}` is not an object", name, key));
                }

                auto const type = CPS_TRY(get_required<std::string>(comp, name, "type").map(string_to_type));
                auto const compile_flags = CPS_TRY(get_required<LangValues>(comp, name, "compile_flags"));
                auto const includes = CPS_TRY(get_required<LangValues>(comp, name, "includes"));
                auto const definitions = CPS_TRY(get_required<Defines>(comp, name, "definitions"));
                auto const link_flags = CPS_TRY(get_optional<std::vector<std::string>>(comp, name, "link_flags"))
                                            .value_or(std::vector<std::string>{});
                auto const link_libraries =
                    CPS_TRY(get_optional<std::vector<std::string>>(comp, name, "link_libraries"))
                        .value_or(std::vector<std::string>{});
                auto const location = CPS_TRY(get_optional<std::string>(comp, name, "location"));
                auto const link_location = CPS_TRY(get_optional<std::string>(comp, name, "link_location"));
                auto const link_requires = CPS_TRY(get_optional<std::vector<std::string>>(comp, name, "link_requires"))
                                               .value_or(std::vector<std::string>{});
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
                    .definitions = std::move(definitions),
                    .link_flags = std::move(link_flags),
                    .link_libraries = std::move(link_libraries),
                    .link_requires = std::move(link_requires),
                    .location = std::move(location),
                    .link_location = std::move(link_location),
                    .require = std::move(require),
                };
            }

            if (components.empty()) {
                return tl::unexpected(fmt::format("`{}` must have at least one component", parent_name));
            }

            return components;
        };

        tl::expected<fs::path, std::string> calculate_prefix(fs::path p, const fs::path & filename) {
            if (p.stem() == "") {
                p = p.parent_path();
            }
            fs::path f = filename.parent_path();
            while (p != "@prefix@") {
                if (p.stem() != f.stem()) {
                    return tl::unexpected(
                        fmt::format("filepath and cps_path have non overlapping stems, prefix: {}, filename {}",
                                    p.string(), f.string()));
                }
                p = p.parent_path();
                f = f.parent_path();
            }
            return f;
        }

    } // namespace

    Define::Define(std::string name_) : name{std::move(name_)}, value{std::nullopt} {};
    Define::Define(std::string name_, std::string value_) : name{std::move(name_)}, value{std::move(value_)} {};

    std::string Define::get_name() const { return name; }
    std::optional<std::string> Define::get_value() const { return value; }

    Configuration::Configuration() = default;
    Configuration::Configuration(LangValues cflags) : compile_flags{std::move(cflags)} {};

    Requirement::Requirement() = default;
    Requirement::Requirement(std::vector<std::string> comps) : components{std::move(comps)} {};
    Requirement::Requirement(std::vector<std::string> && comps, std::optional<std::string> && ver)
        : components{std::move(comps)}, version{std::move(ver)} {};

    Platform::Platform() = default;

    tl::expected<Package, std::string> load(std::istream & input_buffer, const std::filesystem::path & filename) {
        nlohmann::json root;
        try {
            root = nlohmann::json::parse(input_buffer);
        } catch (const nlohmann::json::exception & ex) {
            return tl::make_unexpected(
                fmt::format("Exception caught while parsing json for `{}.cps`\n{}", filename.string(), ex.what()));
        }

        auto const name = CPS_TRY(get_required<std::string>(root, "package", "name"));
        auto const cps_version = CPS_TRY(get_required<std::string>(root, "package", "cps_version"));
        auto const compat_version = CPS_TRY(get_optional<std::string>(root, "package", "compat_version"));
        auto const components = CPS_TRY(get_required<Components>(root, "package", "components"));
        auto const cps_path = CPS_TRY(get_optional<std::string>(root, "package", "cps_path"));
        auto prefix = CPS_TRY(get_optional<std::string>(root, "package", "prefix"));
        auto const default_components =
            CPS_TRY(get_optional<std::vector<std::string>>(root, "package", "default_components"));
        auto const platform = std::nullopt; // TODO: parse platform
        auto const require = CPS_TRY(get_required<Requires>(root, "package", "requires"));
        auto const version = CPS_TRY(get_optional<std::string>(root, "package", "version"));
        auto const version_schema =
            CPS_TRY(get_optional<std::string>(root, "package", "version_schema").map([](auto && v) {
                return string_to_schema(v.value_or("simple"));
            }));

        if (cps_path.has_value() == prefix.has_value()) {
            return tl::make_unexpected("must define exactly one of 'prefix' or 'cps_path'");
        }

        if (cps_version != CPS_VERSION) {
            return tl::make_unexpected(fmt::format("cps-config only supports CPS_VERSION `{}` found `{}` in `{}`",
                                                   CPS_VERSION, cps_version, name));
        }

        // If we don't have a prefix, calculate it now from the cps_path.
        if (!prefix) {
            prefix = CPS_TRY(calculate_prefix(cps_path.value(), filename));
        }

        return Package{
            .name = std::move(name),
            .cps_version = std::move(cps_version),
            .components = std::move(components),
            .compat_version = std::move(compat_version),
            .cps_path = std::move(cps_path),
            .prefix = std::move(prefix.value()),
            .filename = filename.string(),
            .default_components = std::move(default_components),
            .platform = std::move(platform),
            .require = std::move(require), // requires is a keyword
            .version = std::move(version),
            .version_schema = std::move(version_schema),
        };
    }
} // namespace cps::loader
