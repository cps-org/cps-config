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
dep_fmt = dependency('fmt', version : '>= 10.1')
dep_cxxopts = dependency('cxxopts', version : '>=3.1')

cpp = meson.get_compiler('cpp')

conf = configuration_data()
conf.set_quoted('VERSION', meson.project_version())
foreach f : ['unreachable']
  conf.set10('HAVE_@0@'.format(f.to_upper()), cpp.has_function(f))
endforeach

conf_h = configure_file(
  configuration : conf,
  output : 'cps-config-config.hpp',
)

warn_args = cpp.get_supported_arguments(
  '-Werror=shadow',
  '-Werror=implicit-fallthrough',
)

libcps = static_library(
  'cps',
  'src/loader.cpp',
  'src/printer.cpp',
  'src/search.cpp',
  'src/utils.cpp',
  'src/version.cpp',
  conf_h,
  dependencies : [dep_jsoncpp, dep_expected, dep_fmt],
  cpp_args : warn_args,
)

cps_config = executable(
  'cps-config',
  'src/main.cpp',
  conf_h,
  link_with : libcps,
  dependencies : [dep_fmt, dep_expected, dep_cxxopts],
  install : true,
)

test(
  'pkg-config compatibility',
  find_program('tests/runner.py', required : false, disabler : true),
  args: [cps_config, 'tests/cases.toml'],
  protocol : 'tap',
  env : {'CPS_PATH' : meson.current_source_dir() / 'tests' / 'cases' },
)

dep_gtest = dependency('gtest_main', required : false, disabler : true, allow_fallback : true)

foreach t : ['version', 'utils']
  test(
    t,
    executable(
      f'@t@_test',
      f'tests/@t@.cpp',
      dependencies : [dep_gtest, dep_fmt, dep_expected],
      link_with : libcps,
      include_directories : include_directories('src'),
    ),
    protocol : 'gtest',
  )
endforeach
