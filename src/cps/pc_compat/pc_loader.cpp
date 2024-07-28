// SPDX-License-Identifier: MIT
// Copyright © 2024 Haowen Liu

#include "cps/pc_compat/pc_loader.hpp"

#include "cps/error.hpp"
#include "cps/pc_compat/pc.parser.hpp"
#include "cps/utils.hpp"
#include "cps/version.hpp"

#include <fmt/format.h>

namespace cps::pc_compat {

    PcLoader::PcLoader() = default;

    tl::expected<loader::Package, std::string> PcLoader::load(std::istream & istream, std::string const & filename) {
        scan_begin(istream);
        yy::parser parse(*this);
        // To debug parser, uncomment the following line
        // TODO: add a way to enable debug output without rebuilding
        // parse.set_debug_level(true);
        if (const int result = parse(); result != 0) {
            throw std::runtime_error("Failed to parse the given pkg-config file.");
        }

        const auto get_property = [this](const std::string & property_name) -> tl::expected<std::string, std::string> {
            const auto it = properties.find(property_name);
            if (it == properties.end()) {
                return tl::make_unexpected(fmt::format("Pkg-config property {} is not defined.", property_name));
            }
            return it->second;
        };

        std::string name = CPS_TRY(get_property("Name"));
        loader::LangValues compile_flags;
        if (auto compile_flags_input = get_property("Cflags")) {
            auto compile_flags_vec = utils::split(*compile_flags_input);
            compile_flags.emplace(loader::KnownLanguages::c, compile_flags_vec);
            compile_flags.emplace(loader::KnownLanguages::cxx, compile_flags_vec);
            compile_flags.emplace(loader::KnownLanguages::fortran, compile_flags_vec);
        }
        std::vector<std::string> link_flags;
        if (auto link_flags_input = get_property("Libs")) {
            link_flags = cps::utils::split(*link_flags_input);
        }
        std::unordered_map<std::string, loader::Component> components;
        components.emplace(name, loader::Component{
                                     .type = loader::Type::unknown,
                                     .compile_flags = compile_flags,
                                     .includes = loader::LangValues{},
                                     .definitions = loader::Defines{},
                                     .link_flags = link_flags,
                                     .link_libraries = {},
                                     // TODO: Currently lib location is hard coded to appease assertions. This would
                                     // need to implement linker-like search to replicate current behavior.
                                     .location = fmt::format("@prefix@/lib/{}.a", name),
                                     .link_location = std::nullopt,
                                     .require = {} // TODO: Parse requires
                                 });

        const auto version = CPS_TRY(get_property("Version").and_then(get_string));

        return loader::Package{.name = name,
                               .cps_version = std::string{loader::CPS_VERSION},
                               .components = components,
                               // TODO: treat PREFIX in pc file specially and translate it to @prefix@
                               .cps_path = std::nullopt,
                               .filename = filename,
                               .default_components = std::vector{name},
                               .platform = std::nullopt,
                               .require = {}, // TODO: Parse requires
                               .version = version,
                               .version_schema = version::Schema::custom};
    }

    tl::expected<std::string, std::string> PcLoader::get_property(const std::string & property_name) const {
        if (const auto it = properties.find(property_name); it != properties.end()) {
            return it->second;
        }
        return tl::make_unexpected(fmt::format("Property {} is not specified", property_name));
    }

    tl::expected<loader::Package, std::string> load(std::istream & istream, std::string const & filename) {
        PcLoader loader;
        return loader.load(istream, filename);
    }

} // namespace cps::pc_compat
