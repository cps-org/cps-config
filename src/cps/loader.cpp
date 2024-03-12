// Copyright © 2023-2024 Dylan Baker
// Copyright © 2024 Bret Brown
// SPDX-License-Identifier: MIT

#include "cps/loader.hpp"

#include "cps/error.hpp"
#include "cps/utils.hpp"

#include <fmt/core.h>
#include <json/json.h>
#include <tl/expected.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace cps::loader {

    namespace {

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
                fmt::format("Optional field {} in {} is not of type {}!", name, parent_name, typeid(T).name()));
        };

        template <typename T>
        tl::expected<T, std::string> get_required(const Json::Value & parent, std::string_view parent_name,
                                                  const std::string & name) {
            if (!parent.isMember(name)) {
                // TODO: it would be nice to have the parent name…
                return tl::unexpected(fmt::format("Required field {} in {} is missing!", name, parent_name));
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
            if (str == "interface") {
                return Type::interface;
            }
            if (str == "symbolic") {
                return Type::symbolic;
            }
            CPS_UNREACHABLE(fmt::format("Unknown type: {}", str).c_str());
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
            CPS_UNREACHABLE(fmt::format("Unknown version schema: {}", str).c_str());
        }

        tl::expected<LangValues, std::string> get_lang_values(const Json::Value & parent, std::string_view parent_name,
                                                              const std::string & name) {
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
                    fmt::format("Section {} of {} is neither an object nor an array!", parent_name, name));
            }
            return ret;
        }

        tl::expected<Defines, std::string> get_defines(const Json::Value & parent, std::string_view parent_name,
                                                       const std::string & name) {
            LangValues && lang = CPS_TRY(get_lang_values(parent, parent_name, name));
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

        tl::expected<Requires, std::string> get_requires(const Json::Value & parent, std::string_view parent_name,
                                                         const std::string & name) {
            Requires ret{};
            if (!parent.isMember(name)) {
                return ret;
            }

            Json::Value require = parent[name];
            if (!require.isObject()) {
                return tl::unexpected(fmt::format("{} field of {} is not an object", name, parent_name));
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

        tl::expected<std::unordered_map<std::string, Component>, std::string>
        get_components(const Json::Value & parent, std::string_view parent_name, const std::string & name) {
            Json::Value compmap;
            if (!parent.isMember(name)) {
                return tl::unexpected(fmt::format("Required field Components of {} is missing!", parent_name));
            }

            std::unordered_map<std::string, Component> components{};

            // TODO: error handling for not an object
            compmap = parent[name];
            if (!compmap.isObject()) {
                return tl::unexpected(fmt::format("{} field of {} is not an object", name, parent_name));
            }
            if (compmap.empty()) {
                return tl::unexpected(fmt::format("Components field of {} is empty, but must "
                                                  "have at least one component",
                                                  parent_name));
            }

            for (auto && itr = compmap.begin(); itr != compmap.end(); ++itr) {
                // TODO: Error handling for not a string?
                const std::string key = itr.key().asString();
                const Json::Value comp = *itr;

                if (!comp.isObject()) {
                    return tl::unexpected(fmt::format("{} {} is not an object", name, key));
                }

                components[key] = Component{
                    CPS_TRY(get_required<std::string>(comp, name, "type").map(string_to_type)),
                    CPS_TRY(get_lang_values(comp, name, "compile_flags")),
                    CPS_TRY(get_lang_values(comp, name, "includes")), CPS_TRY(get_defines(comp, name, "defines")),
                    CPS_TRY(get_optional<std::vector<std::string>>(comp, name, "link_libraries"))
                        .value_or(std::vector<std::string>{}),
                    // TODO: this is required if the type != interface
                    CPS_TRY(get_optional<std::string>(comp, name, "location")),
                    // XXX: https://github.com/cps-org/cps/issues/34
                    CPS_TRY(get_optional<std::string>(comp, name, "link_location")),
                    CPS_TRY(get_optional<std::vector<std::string>>(comp, name, "requires"))
                        .value_or(std::vector<std::string>{})};
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

    Component::Component() = default;
    Component::Component(Type _type, LangValues _cflags, LangValues _includes, Defines _defines,
                         std::vector<std::string> _link_libs, std::optional<std::string> _loc,
                         std::optional<std::string> _link_loc, std::vector<std::string> req)
        : type{_type}, compile_flags{std::move(_cflags)}, includes{std::move(_includes)}, defines{std::move(_defines)},
          link_libraries{std::move(_link_libs)}, location{std::move(_loc)}, link_location{std::move(_link_loc)},
          require{std::move(req)} {};

    Configuration::Configuration() = default;
    Configuration::Configuration(LangValues cflags) : compile_flags{std::move(cflags)} {};

    Requirement::Requirement() = default;
    Requirement::Requirement(std::vector<std::string> comps) : components{std::move(comps)} {};
    Requirement::Requirement(std::vector<std::string> && comps, std::optional<std::string> && ver)
        : components{std::move(comps)}, version{std::move(ver)} {};

    Platform::Platform() = default;

    Package::Package() = default;
    Package::Package(std::string _name, std::string _cps_version,
                     std::unordered_map<std::string, Component> && _components, std::string cps_path_,
                     std::optional<std::vector<std::string>> && _default_comps, Requires req,
                     std::optional<std::string> ver, version::Schema schema)
        : name{std::move(_name)}, cps_version{std::move(_cps_version)}, components{std::move(_components)},
          cps_path{std::move(cps_path_)}, default_components{std::move(_default_comps)}, require{std::move(req)},
          version{std::move(ver)}, version_schema{schema} {};

    tl::expected<Package, std::string> load(const fs::path & path) {
        std::ifstream file;
        file.open(path);

        Json::Value root;
        file >> root;

        return Package{
            CPS_TRY(get_required<std::string>(root, "package", "name")),
            CPS_TRY(get_required<std::string>(root, "package", "cps_version")),
            CPS_TRY(get_components(root, "package", "components")),
            CPS_TRY(get_optional<std::string>(root, "package", "cps_path")).value_or(path.parent_path()),
            CPS_TRY(get_optional<std::vector<std::string>>(root, "package", "default_components")),
            CPS_TRY(get_requires(root, "package", "requires")),
            CPS_TRY(get_optional<std::string>(root, "package", "version")),
            CPS_TRY(get_optional<std::string>(root, "package", "version_schema").map([](auto && v) {
                return string_to_schema(v.value_or("simple"));
            })),
        };
    }
} // namespace cps::loader
