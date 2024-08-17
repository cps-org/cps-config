# SPDX-License-Identifier: MIT
# Copyright © 2023-2024 Dylan Baker

subdir('cps')

cps_include_dir = include_directories('.')

libcps = static_library(
  'cps',
  'cps/env.cpp',
  'cps/loader.cpp',
  'cps/platform.cpp',
  'cps/printer.cpp',
  'cps/search.cpp',
  'cps/utils.cpp',
  'cps/version.cpp',
  conf_h,
  dependencies : [dep_json, dep_expected, dep_fmt],
  cpp_args : warn_args,
  include_directories: [cps_include_dir, conf_include_dir],
)

dep_cps = declare_dependency(
  link_with : [libcps],
  include_directories: [cps_include_dir, conf_include_dir],
)
