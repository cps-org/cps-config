
conf = configuration_data()
conf.set_quoted('CPS_VERSION', meson.project_version())
foreach f : ['USE_BUILTIN_UNREACHABLE']
  conf.set10('CPS_@0@'.format(f.to_upper()), cpp.has_function(f))
endforeach

conf_h = configure_file(
  configuration : conf,
  output : 'config.hpp',
)

conf_include_dir = include_directories('..')