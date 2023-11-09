// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace cps_config::loader {

    /// @brief  Known Programming languages
    enum class KnownLanguages {
        C,
        CPP,
        FORTRAN,
    };

    /// @brief  Linker required
    enum class LinkLanguage {
        C,
        CPP,
    };

    /// @brief Component type
    enum class Type {
        EXECUTABLE,
        STATIC_LIBRARY,
        DYNAMIC_LIBRARY,
        MODULE,
        JAR,
        INTERFACE,
        SYMBOLIC,
    };

    using LangValues =
        std::unordered_map<KnownLanguages, std::vector<std::string>>;

    class Component {
      public:
        Component();

        LangValues compile_flags;
        // TODO: configurations
        // TODO: LangValues definitions;
        // TODO: LangValues includes;
        // TODO: std::vector<std::string> link_features;
        // TODO: std::vector<std::string> link_flags;
        // TODO: std::vector<LinkLanguage> link_languages;
        // TODO: link_libraries
        // TODO: link_location
        // TODO: link_requires
        // TODO: location
        // TODO: requires
        Type type;
    };

    class Configuration {
      public:
        Configuration();

        LangValues compile_flags;
        // TODO: LangValues definitions;
        // TODO: LangValues includes;
        // TODO: std::vector<std::string> link_features;
        // TODO: std::vector<std::string> link_flags;
        // TODO: std::vector<LinkLanguage> link_languages;
        // TODO: link_libraries
        // TODO: link_location
        // TODO: link_requires
        // TODO: location
        // TODO: requires
    };

    class Requirement {
      public:
        Requirement();

        std::vector<std::string> components;
        // TODO: Hints
    };

    /// @brief Schema version for comparison
    enum class VersionSchema {
        SIMPLE,
        CUSTOM,
        RPM,
        DPKG,
    };

    class Platform {
      public:
        Platform();

        std::optional<std::string> c_runtime_vendor;
        std::optional<std::string> c_runtime_version;
        std::optional<std::string> cpp_runtime_vendor;
        std::optional<std::string> cpp_runtime_version;
        // TODO: clr_vendor
        // TODO: clr_clr_version
        // TODO: ISA
        // TODO: jvm_vendor
        // TODO: jvm_version
        // TODO: kernel
        // TODO: kernel_version
    };

    class Package {
      public:
        Package();

        // TODO: compat-version
        std::unordered_map<std::string, Component> components;
        // TODO: configuration
        // TODO: configurations
        // TODO: cps_path
        std::string cps_version;
        // TODO: default_components
        std::string name;
        std::optional<Platform> platform;
        // TODO: requires
        std::optional<std::string> version;
        VersionSchema version_schema;
    };

} // namespace cps_config::loader