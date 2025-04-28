# SPDX-License-Identifier: MIT
# Copyright © 2023-2024 Dylan Baker

project(
  'cps-config',
  'cpp',
  version : files('VERSION'),
  license : 'MIT',
  meson_version : '>= 0.64',
  default_options : ['cpp_std=c++17', 'buildtype=debugoptimized', 'warning_level=2'],
)

dep_cli11 = dependency('CLI11', version: '>=2.1')
dep_expected = dependency('tl-expected', version : '>= 1.0', modules : ['tl::expected'])
dep_json = dependency('nlohmann_json', version : '>= 3.7')
dep_fmt = dependency('fmt', version : '>= 8')

cpp = meson.get_compiler('cpp')

warn_args = cpp.get_supported_arguments(
  '-Werror=shadow',
  '-Werror=implicit-fallthrough',
  '-Werror=missing-field-initializers',
)

subdir('src')

cps_config = executable(
  'cps-config',
  'src/cps-config/main.cpp',
  conf_h,
  dependencies : [dep_cps, dep_fmt, dep_expected, dep_cli11],
  install : true,
  implicit_include_directories : false,
)

subdir('tests')
