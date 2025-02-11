// Copyright © 2023-2025 Dylan Baker
// Copyright © 2024 Bret Brown
// SPDX-License-Identifier: MIT

#pragma once

#include "cps/version.hpp"

#include <tl/expected.hpp>

#include <filesystem>
#include <istream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace cps::loader {

    /// @brief  Known Programming languages
    enum class KnownLanguages {
        c,
        cxx,
        fortran,
    };

    /// @brief  Linker required
    enum class LinkLanguage {
        c,
        cxx,
    };

    /// @brief Component type
    enum class Type {
        executable,
        archive,
        dylib,
        module,
        jar,
        interface,
        symbolic,
        unknown,
    };

    class Define {
      public:
        Define(std::string name);
        Define(std::string name, std::string value);

        std::string get_name() const;
        std::optional<std::string> get_value() const;

      private:
        std::string name;
        std::optional<std::string> value;
    };

    using LangValues = std::unordered_map<KnownLanguages, std::vector<std::string>>;

    using Defines = std::unordered_map<KnownLanguages, std::vector<Define>>;

    struct Component {
        Type type;
        LangValues compile_flags;
        LangValues includes;
        Defines definitions;
        // TODO: configurations
        // TODO: std::vector<std::string> link_features;
        std::vector<std::string> link_flags;
        // TODO: std::vector<LinkLanguage> link_languages;
        std::vector<std::string> link_libraries;
        std::vector<std::string> link_requires;
        std::optional<std::string> location;
        std::optional<std::string> link_location;
        std::vector<std::string> require; // requires is a keyword
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
        Requirement(std::vector<std::string> components);
        Requirement(std::vector<std::string> && components, std::optional<std::string> && version);

        std::vector<std::string> components;
        // TODO: Hints
        std::optional<std::string> version;
    };

    using Requires = std::unordered_map<std::string, Requirement>;

    class Platform {
      public:
        Platform();

        // TODO: std::optional<std::string> c_runtime_vendor;
        // TODO: std::optional<std::string> c_runtime_version;
        // TODO: std::optional<std::string> cpp_runtime_vendor;
        // TODO: std::optional<std::string> cpp_runtime_version;
        // TODO: clr_vendor
        // TODO: clr_clr_version
        // TODO: ISA
        // TODO: jvm_vendor
        // TODO: jvm_version
        // TODO: kernel
        // TODO: kernel_version
    };

    struct Package {
        std::string name;
        std::string cps_version;
        std::unordered_map<std::string, Component> components;
        std::optional<std::string> compat_version;
        // TODO: configuration
        // TODO: configurations
        std::optional<std::string> cps_path;
        std::string prefix;
        std::string filename;
        std::optional<std::vector<std::string>> default_components;
        std::optional<Platform> platform;
        Requires require; // Requires is a keyword
        std::optional<std::string> version;
        version::Schema version_schema;
    };

    constexpr inline std::string_view CPS_VERSION = "0.13.0";

    tl::expected<Package, std::string> load(std::istream & input_buffer, const std::filesystem::path & filename);

} // namespace cps::loader
