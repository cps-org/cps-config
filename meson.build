project(
  'cps-config',
  'cpp',
  version : '0.0.1',
  license : 'MIT',
  meson_version : '>= 1.0.0',
  default_options : ['cpp_std=c++17'],
)

dep_jsoncpp = dependency('jsoncpp', version : '>= 1.9')
dep_expected = dependency('tl-expected', version : '>= 1.0', modules : ['tl::expected'])
dep_fmt = dependency('fmt', version : '>= 10.1')

cpp = meson.get_compiler('cpp')

conf = configuration_data()
conf.set_quoted('VERSION', meson.project_version())
foreach f : ['unreachable']
  conf.set10(f'HAVE_@f@'.to_upper(), cpp.has_function(f))
endforeach

conf_h = configure_file(
  configuration : conf,
  output : 'cps-config-config.hpp',
)

warn_args = cpp.get_supported_arguments(
  '-Wshadow',
)

cps_config = executable(
  'cps-config',
  'src/main.cpp',
  'src/loader.cpp',
  'src/printer.cpp',
  'src/search.cpp',
  'src/utils.cpp',
  conf_h,
  cpp_args : warn_args,
  dependencies : [dep_jsoncpp, dep_expected, dep_fmt],
  install : true,
)

test('pkg-config', find_program('tests/runner.py'), args: [cps_config, 'tests/cases.toml'], protocol : 'tap')