// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#include "loader.hpp"
#include "error.hpp"
#include "utils.hpp"
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <tl/expected.hpp>

namespace loader {

    namespace {

        template <typename T>
        tl::expected<std::optional<T>, std::string>
        get_optional(const Json::Value & parent, std::string_view parent_name,
                     const std::string & name) {
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
            } else if constexpr (std::is_same_v<
                                     T, std::unordered_map<
                                            std::string,
                                            std::vector<std::string>>>) {
                if (value.isArray()) {
                    std::unordered_map<std::string, std::vector<std::string>>
                        ret;
                    for (auto && [k, v] : value) {
                        // TODO: Error handling?
                        ret.emplace(k.asString(), v.asString());
                    }
                    return ret;
                }
            }

            return tl::unexpected(
                fmt::format("Optional field {} in {} is not of type {}!", name,
                            parent_name, typeid(T).name()));
        };

        template <typename T>
        tl::expected<T, std::string> get_required(const Json::Value & parent,
                                                  std::string_view parent_name,
                                                  const std::string & name) {
            if (!parent.isMember(name)) {
                // TODO: it would be nice to have the parent name…
                return tl::unexpected(fmt::format(
                    "Required field {} in {} is missing!", name, parent_name));
            }

            return get_optional<T>(parent, parent_name, name)
                .and_then([](auto && v) -> tl::expected<T, std::string> {
                    if (v)
                        return v.value();
                    return "bad";
                });
            // TODO: also need to fixup error message for "Optional type ..."
        }

        Type from_string(std::string_view str) {
            if (str == "executable") {
                return Type::EXECUTABLE;
            }
            if (str == "archive") {
                return Type::ARCHIVE;
            }
            if (str == "dylib") {
                return Type::DYLIB;
            }
            if (str == "module") {
                return Type::MODULE;
            }
            if (str == "interfafce") {
                return Type::INTERFACE;
            }
            if (str == "symbolic") {
                return Type::SYMBOLIC;
            }
            unreachable(fmt::format("Unkown type: {}", str).c_str());
        }

        tl::expected<LangValues, std::string>
        get_lang_values(const Json::Value & parent,
                        std::string_view parent_name,
                        const std::string & name) {
            LangValues ret{};
            if (!parent.isMember(name)) {
                return ret;
            }

            Json::Value value = parent[name];
            if (value.isObject()) {
                // TODO: simplify this further, maybe with a loop?
                auto cb = [](auto && r) {
                    return r.value_or(std::vector<std::string>{});
                };
                ret[KnownLanguages::C] =
                    TRY(get_optional<std::vector<std::string>>(value, name, "C")
                            .map(cb));
                ret[KnownLanguages::CPP] = TRY(
                    get_optional<std::vector<std::string>>(value, name, "C++")
                        .map(cb));
                ret[KnownLanguages::FORTRAN] =
                    TRY(get_optional<std::vector<std::string>>(value, name,
                                                               "Fortran")
                            .map(cb));
            } else if (value.isArray()) {
                std::vector<std::string> fin;
                for (auto && v : value) {
                    fin.emplace_back(v.asString());
                }
                for (auto && v : {KnownLanguages::C, KnownLanguages::CPP,
                                  KnownLanguages::FORTRAN}) {
                    ret.emplace(v, fin);
                }
            } else {
                return tl::unexpected(fmt::format(
                    "Section {} of {} is neither an object nor an array!",
                    parent_name, name));
            }
            return ret;
        }

        tl::expected<Defines, std::string>
        get_defines(const Json::Value & parent, std::string_view parent_name,
                    const std::string & name) {
            LangValues && lang =
                TRY(get_lang_values(parent, parent_name, name));
            Defines ret;
            for (auto && [k, values] : lang) {
                ret[k] = {};
                for (auto && value : values) {
                    if (value.front() == '!') {
                        ret[k].emplace_back(Define{value.substr(1), false});
                    } else if (const size_t sep = value.find("=");
                               sep != value.npos) {
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

        tl::expected<std::unordered_map<std::string, Component>, std::string>
        get_components(const Json::Value & parent, std::string_view parent_name,
                       const std::string & name) {
            Json::Value compmap;
            if (!parent.isMember("Components")) {
                return tl::unexpected(
                    fmt::format("Required field Components of {} is missing!",
                                parent_name));
            }

            std::unordered_map<std::string, Component> components{};

            // TODO: error handling for not an object
            compmap = parent["Components"];
            if (!compmap.isObject()) {
                return tl::unexpected(fmt::format(
                    "Components field of {} is not an object", parent_name));
            }
            if (compmap.empty()) {
                return tl::unexpected(
                    fmt::format("Components field of {} is empty, but must "
                                "have at least one component",
                                parent_name));
            }

            for (auto && itr = compmap.begin(); itr != compmap.end(); ++itr) {
                // TODO: Error handling for not a string?
                const std::string key = itr.key().asString();
                const Json::Value comp = *itr;

                if (!comp.isObject()) {
                    return tl::unexpected(
                        fmt::format("Component {} is not an object", key));
                }

                components[key] = Component{
                    TRY(get_required<std::string>(comp, "Component", "Type")
                            .map(from_string)),
                    TRY(get_lang_values(comp, "Component", "Compile-Flags")),
                    TRY(get_lang_values(comp, "Component", "Includes")),
                    TRY(get_defines(comp, "Component", "Defines"))};
            }

            return components;
        };

    } // namespace

    Define::Define(std::string name)
        : name{std::move(name)}, value{}, define{true} {};
    Define::Define(std::string name, std::string value)
        : name{std::move(name)}, value{std::move(value)}, define{true} {};
    Define::Define(std::string name, bool define)
        : name{std::move(name)}, value{}, define{define} {};

    bool Define::is_undefine() const { return !define; }

    bool Define::is_define() const { return define && value.empty(); }

    std::string Define::get_name() const { return name; }
    std::string Define::get_value() const { return value; }

    Component::Component() = default;
    Component::Component(Type type, LangValues cflags, LangValues includes,
                         Defines defines)
        : type{type}, compile_flags{std::move(cflags)},
          includes{std::move(includes)}, defines{std::move(defines)} {};

    Configuration::Configuration() = default;
    Configuration::Configuration(LangValues cflags)
        : compile_flags{std::move(cflags)} {};

    Requirement::Requirement() = default;

    Platform::Platform() = default;

    Package::Package() = default;
    Package::Package(std::string name, std::string cps_version,
                     std::unordered_map<std::string, Component> && components,
                     std::optional<std::vector<std::string>> && default_comps)
        : name{std::move(name)}, cps_version{std::move(cps_version)},
          components{std::move(components)},
          default_components{std::move(default_comps)} {};

    tl::expected<Package, std::string>
    load(const std::filesystem::path & path) {
        std::ifstream file;
        file.open(path);

        Json::Value root;
        file >> root;

        return Package{
            TRY(get_required<std::string>(root, "package", "Name")),
            TRY(get_required<std::string>(root, "package", "Cps-Version")),
            TRY(get_components(root, "package", "Components")),
            TRY(get_optional<std::vector<std::string>>(root, "package",
                                                       "Default-Components")),
        };
    }
} // namespace loader