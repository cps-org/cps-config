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

Meson currently is configured with support to build all of it's dependencies
from source if they are not otherwise detected, by fetching at configure time.
If this is not desirable, calling:

```sh
meson setup builddir --wrap-mode=nofallback
```

Will disable this behavior. This will require that all dependencies have been
installed and are discoverable at configure time.

## Status

CPS-config is currently in alpha status. Some things work, others do not.

Bug reports and other issues are welcome.

Bug fix and feature PRs are welcome. For larger features please open an issue to
help coordinate work.