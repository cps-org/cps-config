project(
  'cps-config',
  'cpp',
  version : '0.0.1',
  license : 'MIT',
  meson_version : '>= 1.0.0',
  default_options : ['cpp_std=c++17'],
)

dep_jsoncpp = dependency('jsoncpp', version : '>= 1.9')

executable(
  'cps-config',
  'src/main.cpp',
  'src/loader.cpp',
  dependencies : [dep_jsoncpp],
  install : true,
)