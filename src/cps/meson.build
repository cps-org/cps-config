# SPDX-License-Identifier: MIT
# Copyright © 2023-2024 Dylan Baker

conf = configuration_data()
conf.set_quoted('CPS_CONFIG_VERSION', meson.project_version())

conf_h = configure_file(
  configuration : conf,
  output : 'config.hpp',
)

conf_include_dir = include_directories('..')
