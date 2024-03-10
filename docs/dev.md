# Developer Information

## Git

### Pull Requests

A PR should be, mostly, on a single topic. A common exception to this might be
for a small PR to include small bug fixes. If a PR becomes very large, it is
helpful to split it into several smaller PRs, if this is possible. It is fine in
that case to base the final PR on the earlier ones, with a note such as "Based
on #1, #2, and #6" in the PR description.

### Commits

Each commit should make one logical change. This makes it easier to review the
changes and to bisect for regressions. Each commit must build successfully
and should pass all tests. This may mean marking some tests as expected failures.

A git commit should have a descriptive subject line between 50 and 72
characters in length. If it is not possible to be under 72 characters, be as close as possible.
It is helpful to preface the subject with a short description of what is
affected. For example:

```
meson: bump gtest requirement from 1.12 to 1.15
```

After the subject should come a more detailed description of the change. This
should include the reason for the change. It may also include implementation
details, design decisions, and issues that may not be resolved or may need to be
resolved in the future. Lines in this section should be wrapped to 72
characters whenever possible. A common exception to this rule
would be to accommodate a string that is itself longer than 72 characters.

For example:

```
meson: bump gtest requirement from 1.12 to 1.15

gtest 1.13 fixes a bug on Windows that is affecting our CI, see
google/googletest#1234.

gtest 1.14 adds a new feature that allows for faster test execution that will be
used in the next patch
```

Finally, if a commit fixes any github issues, they should add "Fixes" notes like so:

```
Fixes: #7
```

### Merging and rebasing

Rebasing should generally be preferred to merging. Rebases create a linear
history that is easy to bisect over and easy for a human to understand. In a
project like cps-config that lacks large subsystems, this is easy to maintain.
Merges also do not go through CI, and as such it's possible to create a state
where main before the merge builds successfully, and the topic branch before the
merge builds successfully, but the merge commit itself is broken.

Back merges (merging the main branch back into a topic branch) should never
happen. This makes bisects particularly hard and the history very messy. A
rebase should be done in this case.

## Editors

Use [editorconfig](https://editorconfig.org). Many editors support editorconfig
natively. Others have plugins. Otherwise, configure your editor to use the same
settings.

## All files

All source files should have an [SPDX license
header](https://spdx.dev/learn/handling-license-info/), along with the relevant
copyright holder information, as long as they have a comment syntax.

For example, in a C or C++ file:
```cpp
// SPDX-License-Identifier: MIT
// Copyright 2024 Copyright Holder
```

Plain data files do not. CPS files, in particular, are JSON, which does not have
comments. As such, CPS files do not need any SPDX or Copyright information in
them.

## Meson

Use the following naming conventions:

- dependencies (found by `dependency()` or `compiler.find_library()`, and
  produced by `declare_dependency()`) should be prefixed with `dep_`, ex:
  `dep_fmt`
- external programs (`find_program()`) should be prefixed with `prog_`

use spaces around `:`, ex: `dependency('gtest', version : '>= 1.5')`

always drop arguments to a new line when they do not fit on one:

```meson
project(
  'cps-config',
  'cpp',
  version : '0.0.1',
)
```

## C and C++

The C++ style is based on the LLVM coding standard, with the following (known)
changes:
  - whitespace: Always use 4 spaces for indentation
  - spaces around `&` and `*` in types: `const auto & foo = something();`

Prefer C++ (`//`) comments to C-style (`/* */`) in general.

`#defines` should be all uppercase, and be prefaced with `CPS_`. such as
`CPS_USE_UNDEFINED`

Compiled code should be able to build without warning or errors in most cases.
This can be tested be enabling warnings as errors. For example, with Meson:
```sh
# for a new build directory:
meson setup builddir -Dwerror=true
# to reconfigure an existing build directory:
meson configure builddir -DWerror=true
```

clang-format should be used. cps-config currently uses clang-format version 16.

Meson provides a helper target for this, which may be helpful:
```sh
meson setup builddir
ninja -C builddir clang-format
```

## Tests

Complex implementation details may use compiled unit tests, like gtests. Unit tests
are generally faster and easier to debug than functional tests.

Functional tests should use `tests/runner.py` by adding test cases to
`tests/cases.toml`  along with changes to the accompanying `.cps` files.

All bug fixes must have a regression test. New features must have appropriate
tests.
