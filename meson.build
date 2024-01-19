project(
  'cps-config',
  'cpp',
  version : '0.0.1',
  license : 'MIT',
  meson_version : '>= 1.0.0',
  default_options : ['cpp_std=c++17'],
)

dep_jsoncpp = dependency('jsoncpp', version : '>= 1.9')
dep_expected = dependency('tl-expected', version : '>= 1.1')
dep_fmt = dependency('fmt', version : '>= 10.1')

cps_config = executable(
  'cps-config',
  'src/main.cpp',
  'src/loader.cpp',
  'src/printer.cpp',
  dependencies : [dep_jsoncpp, dep_expected, dep_fmt],
  install : true,
)

test('pkg-config', find_program('tests/runner.py'), args: [cps_config, 'tests/cases.toml'])