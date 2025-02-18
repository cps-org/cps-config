tests = files(
  'basic.toml',
  'conflicts.toml',
  'framework.toml',
  'parser.toml',
  'provides.toml',
  #'regress.toml',
  #'requires.toml',
  #'symlink.toml',
  'sysroot.toml',
  'version.toml',
)

tests += [configure_file(
  configuration : {'PKGCONF_VERSION' : meson.project_version()},
  input : 'builtins.toml.in',
  output : '@BASENAME@',
)]

test('tests', find_program('runner.py'), args: [pkgconf_exe, tests])
