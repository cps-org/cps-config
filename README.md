# CPS-Config

CPS-Config has been designed to be used a drop-in replacement for pkg-config,
with at least some features from pkgconf, as well as it's own features.

## Building

You will need at least a C++ compiler, the Meson build system, and Ninja. It is
currently only tested on Linux.

Basic instructions are:
```sh
meson setup builddir
ninja -C builddir test
```

Running the functional tests requires Python >= 3.11

Meson currently is configured with support to build all of its dependencies
from source if they are not otherwise detected by fetching them at configure time.
If this is not desirable, calling:

```sh
meson setup builddir --wrap-mode=nofallback
```

Will disable this behavior. This will require that all dependencies have been
installed and are discoverable at configure time.

Test execution can be controlled by setting the `test` option to Meson:
```sh
meson setup builddir -Dtests=disabled
```
Or, alternatively, to enable tests:
```sh
meson configure builddir -Dtests=enabled
```
This is a Meson [feature](https://mesonbuild.com/Build-options.html#features)
option, which can have a state of `disabled`, `enabled`, or `auto`, as described
in the linked documentation. The default is `auto`, which is usually desirable
for end users.

## Status

CPS-config is currently in alpha status. Some things work, others do not.

Bug reports and other issues are welcome.

Bug fix and feature PRs are welcome. For larger features please open an issue to
help coordinate work.
