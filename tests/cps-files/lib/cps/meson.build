# SPDX-License-Identifier: MIT
# Copyright © 2024-2025 Dylan Baker

conf = configuration_data()
conf.set('libdir', get_option('libdir'))
# This is a work around for a bug fixed in Meson 1.7.0
# See: https://github.com/mesonbuild/meson/issues/13665
conf.set('prefix', '@prefix@')

foreach infile : ['cps-path-set.cps.in']
  configure_file(
    configuration : conf,
    input : infile,
    output : '@BASENAME@',
    format : 'cmake',
  )
endforeach
