// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#include "loader.hpp"
#include <fstream>
#include <iostream>
#include <json/json.h>

namespace cps_config::loader {

    namespace {

        template <typename T>
        T get_required(const Json::Value & parent, std::string_view parent_name,
                       const std::string & name) {
            if (!parent.isMember(name)) {
                // TODO: it would be nice to have the parent name…
                std::cerr << "Required field " << name << " in " << parent_name
                          << " is missing!\n";
                abort();
            }
            const Json::Value value = parent[name];

            if constexpr (std::is_same_v<T, std::string>) {
                if (value.isString()) {
                    return value.asString();
                }
            } else {
                // static_assert(false, "Unhandled type");
            }

            // TODO: better than typeid
            std::cerr << "Required field " << name << " in " << parent_name
                      << " is not of type " << typeid(T).name() << "!\n";
            abort();
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

            std::cerr << "Unknown type: " << str << "\n";
            abort();
        }

        std::unordered_map<std::string, Component>
        get_components(const Json::Value & parent, std::string_view parent_name,
                       const std::string & name) {
            Json::Value compmap;
            if (!parent.isMember("Components")) {
                std::cerr << "Required field Components of " << parent_name
                          << " is missing!\n";
                abort();
            }

            std::unordered_map<std::string, Component> components{};

            // TODO: error handling for not an object
            compmap = parent["Components"];
            if (!compmap.isObject()) {
                std::cerr << "Components field of " << parent_name
                          << " is not an object\n";
                abort();
            }
            if (compmap.empty()) {
                std::cerr << "Components field of " << parent_name
                          << " is empty, but must have at least one component\n";
                abort();
            }

            for (auto && itr = compmap.begin(); itr != compmap.end(); ++itr) {
                // TODO: Error handling for not a string?
                const std::string key = itr.key().asString();
                const Json::Value comp = *itr;

                if (!comp.isObject()) {
                    std::cerr << "Component " << key << " is not an object\n";
                    abort();
                }

                const auto rawtype = get_required<std::string>(comp, "Component", "Type");
                const Type type = from_string(rawtype);

                components[key] = Component{type};
            }

            return components;
        };

    } // namespace

    Component::Component() = default;
    Component::Component(Type type) : type{type} {};

    Configuration::Configuration() = default;

    Requirement::Requirement() = default;

    Platform::Platform() = default;

    Package::Package() = default;
    Package::Package(std::string name, std::string cps_version,
                     std::unordered_map<std::string, Component> && components)
        : name{std::move(name)}, cps_version{std::move(cps_version)},
          components{components} {};

    Package load(const std::filesystem::path & path) {
        std::ifstream file;
        file.open(path);

        Json::Value root;
        file >> root;

        return Package{
            get_required<std::string>(root, "package", "Name"),
            get_required<std::string>(root, "package", "Cps-Version"),
            get_components(root, "package", "Components"),
        };
    }
} // namespace cps_config::loader