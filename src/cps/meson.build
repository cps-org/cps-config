# SPDX-License-Identifier: MIT
# Copyright © 2023-2024 Dylan Baker

conf = configuration_data()
conf.set_quoted('CPS_VERSION', meson.project_version())
foreach f : ['unreachable']
  conf.set10('CPS_USE_BUILTIN_@0@'.format(f.to_upper()), cpp.has_function(f))
endforeach

conf_h = configure_file(
  configuration : conf,
  output : 'config.hpp',
)

conf_include_dir = include_directories('..')