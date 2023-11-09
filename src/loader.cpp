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
        };

    } // namespace

    Component::Component() = default;

    Configuration::Configuration() = default;

    Requirement::Requirement() = default;

    Platform::Platform() = default;

    Package::Package() = default;
    Package::Package(std::string name, std::string cps_version)
        : name{std::move(name)}, cps_version{std::move(cps_version)} {};

    Package load(const std::filesystem::path & path) {
        std::ifstream file;
        file.open(path);

        Json::Value root;
        file >> root;

        return Package{
            get_required<std::string>(root, "package", "Name"),
            get_required<std::string>(root, "package", "Cps-Version")};
    }
} // namespace cps_config::loader