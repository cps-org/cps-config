// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#include "loader.hpp"
#include <fmt/core.h>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <tl/expected.hpp>

namespace loader {

    namespace {

        void handle_error(std::string_view str) {
            fmt::print(stderr, str);
            abort();
        };

        template <typename T>
        tl::expected<T, std::string> get_required(const Json::Value & parent,
                                                  std::string_view parent_name,
                                                  const std::string & name) {
            if (!parent.isMember(name)) {
                // TODO: it would be nice to have the parent name…
                return fmt::format("Required field {} in {} is missing!", name,
                                   parent_name);
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
            return fmt::format("Required field {} in {} is not of type {}!",
                               name, parent_name, typeid(T).name());
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

                const Type type =
                    get_required<std::string>(comp, "Component", "Type")
                        .map_error(handle_error)
                        .map(from_string)
                        .value();

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
            get_required<std::string>(root, "package", "Name")
                .map_error(handle_error)
                .value(),
            get_required<std::string>(root, "package", "Cps-Version")
                .map_error(handle_error)
                .value(),
            get_components(root, "package", "Components")
                .map_error(handle_error)
                .value(),
        };
    }
} // namespace cps_config::loader