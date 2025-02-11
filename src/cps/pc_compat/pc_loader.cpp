// SPDX-License-Identifier: MIT
// Copyright © 2024 Haowen Liu

#include "cps/pc_compat/pc_loader.hpp"

#include "cps/error.hpp"
#include "cps/pc_compat/pc.parser.hpp"
#include "cps/utils.hpp"
#include "cps/version.hpp"

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <variant>

#include <fmt/format.h>
#include <tl/expected.hpp>

namespace cps::pc_compat {

    namespace fs = std::filesystem;

    std::ostream & operator<<(std::ostream & ost, const std::optional<VersionOperation> & version_operation) {
        if (!version_operation) {
            return ost;
        }
        switch (*version_operation) {
        case VersionOperation::eq:
            ost << "=";
            break;
        case VersionOperation::ne:
            ost << "!=";
            break;
        case VersionOperation::lt:
            ost << "<";
            break;
        case VersionOperation::le:
            ost << "<=";
            break;
        case VersionOperation::gt:
            ost << ">";
            break;
        case VersionOperation::ge:
            ost << ">=";
            break;
        }
        return ost;
    }

    std::ostream & operator<<(std::ostream & ost, const PackageRequirement & package_requirement) {
        ost << package_requirement.package;
        if (package_requirement.operation) {
            ost << " " << *package_requirement.operation;
        }
        if (package_requirement.version) {
            ost << " " << *package_requirement.version;
        }
        return ost;
    }

    std::ostream & operator<<(std::ostream & ost, const PackageRequirements & package_requirements) {
        // Print trailing comma, but it's fine since this is only used for parser debug output
        for (const auto & package_requirement : package_requirements) {
            ost << package_requirement << ", ";
        }
        return ost;
    }

    std::ostream & operator<<(std::ostream & ost, const PcPropertyValue & property_value) {
        if (std::holds_alternative<std::string>(property_value)) {
            ost << std::get<std::string>(property_value);
            return ost;
        }
        ost << std::get<PackageRequirements>(property_value);
        return ost;
    }

    PcLoader::PcLoader() = default;

    tl::expected<loader::Package, std::string> PcLoader::load(std::istream & istream, fs::path const & filename) {
        scan_begin(istream);
        yy::parser parse(*this);
        // To debug parser, uncomment the following line
        // TODO: add a way to enable debug output without rebuilding
        // parse.set_debug_level(true);
        if (const int result = parse(); result != 0) {
            throw std::runtime_error("Failed to parse the given pkg-config file.");
        }

        std::string name = CPS_TRY(get_property("Name").and_then(get_string));

        loader::LangValues compile_flags;
        if (auto compile_flags_input = get_property("Cflags").and_then(get_string)) {
            auto compile_flags_vec = utils::split(*compile_flags_input);
            compile_flags.emplace(loader::KnownLanguages::c, compile_flags_vec);
            compile_flags.emplace(loader::KnownLanguages::cxx, compile_flags_vec);
            compile_flags.emplace(loader::KnownLanguages::fortran, compile_flags_vec);
        }

        std::vector<std::string> link_flags;
        if (auto link_flags_input = get_property("Libs").and_then(get_string)) {
            link_flags = cps::utils::split(*link_flags_input);
        }

        std::vector<std::string> require;
        if (auto requires_input = get_property("Requires").and_then(get_package_requirements)) {
            std::transform(requires_input->begin(), requires_input->end(), std::back_inserter(require),
                           [](const PackageRequirement & requirement) { return requirement.package; });
        }

        std::unordered_map<std::string, loader::Component> components;
        components.emplace(
            name, loader::Component{.type = loader::Type::unknown,
                                    .compile_flags = compile_flags,
                                    .includes = loader::LangValues{},
                                    .definitions = loader::Defines{},
                                    .link_flags = link_flags,
                                    .link_libraries = {},
                                    // TODO: Currently lib location is hard coded to appease assertions. This would
                                    // need to implement linker-like search to replicate current behavior.
                                    .location = fmt::format("@prefix@/lib/{}.a", name),
                                    .link_location = std::nullopt,
                                    .require = require});

        const auto version = CPS_TRY(get_property("Version").and_then(get_string));

         return loader::Package{.name = name,
                               .cps_version = std::string{loader::CPS_VERSION},
                               .components = components,
                               // TODO: consider how pkg-config actually handles version and 
                               .compat_version = version,
                               // TODO: treat PREFIX in pc file specially and translate it to @prefix@
                               .cps_path = std::nullopt,
                               .filename = filename.string(),
                               .default_components = std::vector{name},
                               .platform = std::nullopt,
                               .require = {}, // TODO: Parse requires
                               .version = version,
                               .version_schema = version::Schema::custom};
    }

    tl::expected<PcPropertyValue, std::string> PcLoader::get_property(const std::string & property_name) const {
        if (const auto it = properties.find(property_name); it != properties.end()) {
            return it->second;
        }
        return tl::make_unexpected(fmt::format("Property {} is not specified", property_name));
    }

    tl::expected<std::string, std::string> PcLoader::get_string(const PcPropertyValue & property_value) {
        if (std::holds_alternative<std::string>(property_value)) {
            return std::get<std::string>(property_value);
        }
        return tl::make_unexpected("Property expected to hold literal or fragment list");
    }

    tl::expected<PackageRequirements, std::string>
    PcLoader::get_package_requirements(const PcPropertyValue & property_value) {
        if (std::holds_alternative<PackageRequirements>(property_value)) {
            return std::get<PackageRequirements>(property_value);
        }
        return tl::make_unexpected("Property expected to hold package requirement list");
    }

    tl::expected<loader::Package, std::string> load(std::istream & istream, fs::path const & filename) {
        PcLoader loader;
        return loader.load(istream, filename);
    }

} // namespace cps::pc_compat
