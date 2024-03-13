# SPDX-License-Identifier: MIT
# Copyright © 2023-2024 Dylan Baker

project(
  'cps-config',
  'cpp',
  version : '0.0.1',
  license : 'MIT',
  meson_version : '>= 0.64',
  default_options : ['cpp_std=c++17', 'buildtype=debugoptimized', 'warning_level=2'],
)

dep_jsoncpp = dependency('jsoncpp', version : '>= 1.9')
dep_expected = dependency('tl-expected', version : '>= 1.0', modules : ['tl::expected'])
dep_fmt = dependency('fmt', version : '>= 8')
dep_cxxopts = dependency('cxxopts', version : '>=3.0')

cpp = meson.get_compiler('cpp')

warn_args = cpp.get_supported_arguments(
  '-Werror=shadow',
  '-Werror=implicit-fallthrough',
)

subdir('src')

cps_config = executable(
  'cps-config',
  'src/cps-config/main.cpp',
  conf_h,
  dependencies : [dep_cps, dep_fmt, dep_expected, dep_cxxopts],
  install : true,
)

build_tests = get_option('tests')

test(
  'pkg-config compatibility',
  find_program('python', version : '>=3.11', required : build_tests, disabler : true),
  args: [files('tests/runner.py'), cps_config, 'tests/cases.toml'],
  protocol : 'tap',
  env : {'CPS_PATH' : meson.current_source_dir() / 'tests' / 'cases' },
)

dep_gtest = dependency('gtest_main', required : build_tests, disabler : true, allow_fallback : true)

foreach t : ['version', 'utils']
  test(
    t,
    executable(
      f'@t@_test',
      f'tests/@t@.cpp',
      dependencies : [dep_cps, dep_gtest, dep_fmt, dep_expected],
    ),
    protocol : 'gtest',
  )
endforeach
