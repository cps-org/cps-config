# SPDX-License-Identifier: MIT
# Copyright © 2024 Dylan Baker

subdir('cps-files/lib/cps')

build_tests = get_option('tests')

python_interpreter = find_program('python', version : '>=3.11', required : build_tests, disabler : true)
runner_args = [
  files('runner.py'),
  cps_config,
  '--libdir', get_option('libdir'),
]

foreach t : [['cps integration tests', 'cps-config.toml'], ['pkg-config compatibility', 'pkg-config-compat.toml']]
  test(
    t[0],
    python_interpreter,
    args: [runner_args, meson.current_source_dir() / 'cases' / t[1]],
    protocol : 'tap',
    env : {
      'CPS_PREFIX_PATH' : meson.current_source_dir() / 'cps-files',
      'PKG_CONFIG_PATH' : meson.current_source_dir() / 'cps-files' / 'lib' / 'pkgconfig'
    },
  )
endforeach
test(
  'prefix calculation tests',
  python_interpreter,
  args: [
    runner_args,
    '--prefix', meson.project_build_root(),
    meson.current_source_dir() / 'cases' / 'cps-prefix-calculation.toml',
  ],
  protocol : 'tap',
  env : {
    'CPS_PREFIX_PATH' : meson.current_build_dir() / 'cps-files',
    'PKG_CONFIG_PATH' : meson.current_source_dir() / 'cps-files' / 'lib' / 'pkgconfig'
  },
)

dep_gtest = dependency('gtest_main', required : build_tests, disabler : true, allow_fallback : true)

foreach t : ['loader', 'version', 'utils', 'pc_parser']
  test(
    t,
    executable(
      f'@t@_test',
      f'@t@.cpp',
      dependencies : [dep_cps, dep_gtest, dep_fmt, dep_expected],
      implicit_include_directories : false,
    ),
    env: {
      'CPS_TEST_DIR' : meson.current_source_dir(),
    },
    protocol : 'gtest',
  )
endforeach


pkg_conf_suites = files(
'pkgconf/basic.toml',
'pkgconf/provides.toml',
'pkgconf/parser.toml',
'pkgconf/sysroot.toml',
'pkgconf/version.toml',
'pkgconf/framework.toml',
'pkgconf/conflicts.toml',
)

pkg_conf_suites += [
  configure_file(
    input : 'pkgconf/builtins.toml.in',
    output : '@BASENAME@',
    configuration : {'PKGCONF_VERSION' : meson.project_version()},
  ),
]

test(
  'pkgconf tests',
  find_program('pkgconf/runner.py'),
  args: [cps_config, pkg_conf_suites],
)
