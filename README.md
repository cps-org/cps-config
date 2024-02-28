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

## Status

CPS-config is currently in alpha status. Some things work, others do not.

Bug reports and other issues are welcome.

Bug fix and feature PRs are welcome. For larger features please open an issue to
help coordinate work.