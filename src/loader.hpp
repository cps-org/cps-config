// Copyright © 2023 Dylan Baker
// SPDX-License-Identifier: MIT

#pragma once

#include "error.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <tl/expected.hpp>
#include <unordered_map>
#include <vector>

namespace loader {

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
        ARCHIVE,
        DYLIB,
        MODULE,
        JAR,
        INTERFACE,
        SYMBOLIC,
    };

    class Define {
      public:
        Define(std::string name);
        Define(std::string name, std::string value);
        Define(std::string name, bool define);

        bool is_undefine() const;
        bool is_define() const;
        std::string get_name() const;
        std::string get_value() const;

      private:
        std::string name;
        std::string value;
        bool define;
    };

    using LangValues =
        std::unordered_map<KnownLanguages, std::vector<std::string>>;

    using Defines = std::unordered_map<KnownLanguages, std::vector<Define>>;

    class Component {
      public:
        Component();
        Component(Type type, LangValues cflags, LangValues includes,
                  Defines defines, std::vector<std::string> link_libraries,
                  std::optional<std::string> location,
                  std::optional<std::string> link_location);

        Type type;
        LangValues compile_flags;
        LangValues includes;
        Defines defines;
        // TODO: configurations
        // TODO: std::vector<std::string> link_features;
        // TODO: std::vector<std::string> link_flags;
        // TODO: std::vector<LinkLanguage> link_languages;
        std::vector<std::string> link_libraries;
        // TODO: link_requires
        std::optional<std::string> location;
        std::optional<std::string> link_location;
        // TODO: requires
    };

    class Configuration {
      public:
        Configuration();
        Configuration(LangValues cflags);

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
        Package(std::string name, std::string cps_version,
                std::unordered_map<std::string, Component> && components,
                std::optional<std::vector<std::string>> && default_comps);

        std::string name;
        std::string cps_version;
        std::unordered_map<std::string, Component> components;
        // TODO: compat-version
        // TODO: configuration
        // TODO: configurations
        // TODO: cps_path
        std::optional<std::vector<std::string>> default_components;
        std::optional<Platform> platform;
        // TODO: requires
        std::optional<std::string> version;
        VersionSchema version_schema;
    };

    tl::expected<Package, std::string> load(const std::filesystem::path & path);

} // namespace loader