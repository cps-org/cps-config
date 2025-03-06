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

test(
  'pkgconf tests',
  python_interpreter,
  args: [
    files('runner.py'),
    '@0@ pkg-config'.format(cps_config.full_path()),
    tests,
    '--tap'
  ],
  protocol: 'tap',
)
