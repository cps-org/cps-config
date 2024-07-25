#!/usr/bin/env python
# SPDX-License-Identifier: MIT
# Copyright © 2024 Dylan Baker

from __future__ import annotations

import argparse
import asyncio
import contextlib
import dataclasses
import enum
import os
import shutil
import sys
import tempfile
import tomllib
import typing

if typing.TYPE_CHECKING:

    class Arguments(typing.Protocol):

        runner: str
        cases: str
        libdir: str
        prefix: str | None

    class TestCase(typing.TypedDict):

        name: str
        cps: str
        args: list[str]
        expected: str
        mode: typing.NotRequired[typing.Literal['pkgconf']]
        returncode: typing.NotRequired[int]

    class TestDescription(typing.TypedDict):

        case: list[TestCase]


SOURCE_DIR = os.path.normpath(os.path.dirname(os.path.dirname(__file__)))
_PRINT_LOCK = asyncio.Lock()


@enum.unique
class Status(enum.Enum):

    PASS = enum.auto()
    FAIL = enum.auto()
    TIMEOUT = enum.auto()


@dataclasses.dataclass
class Result:

    name: str
    status: Status
    stdout: str
    stderr: str
    returncode: int | None
    expected: str
    command: list[str]


def unordered_compare(out: str, expected: str) -> bool:
    if out == expected:
        return True

    out_parts = out.split()
    expected_parts = expected.split()
    return sorted(out_parts) == sorted(expected_parts)


async def test(args: Arguments, case_: TestCase) -> Result:
    prefix = args.prefix or SOURCE_DIR

    cmd = [args.runner] + case_['args']
    cmd.append(case_['cps'].replace('{prefix}', os.path.join(prefix, args.libdir, 'cps')))
    if 'mode' in case_:
        cmd.extend([f"--format={case_['mode']}"])

    expected = case_['expected'].format(prefix=prefix, libdir=args.libdir)

    try:
        async with asyncio.timeout(5):
            proc = await asyncio.create_subprocess_exec(
                *cmd,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE,
            )
        bout, berr = await proc.communicate()
        out = bout.decode().strip()
        err = berr.decode().strip()

        success = proc.returncode == case_.get('returncode', 0) and unordered_compare(out, expected)
        result = Status.PASS if success else Status.FAIL
        returncode = proc.returncode
    except asyncio.TimeoutError:
        out = ''
        err = 'Timed out after 5 seconds'
        result = Status.TIMEOUT
        returncode = None

    async with _PRINT_LOCK:
        print('ok' if result is Status.PASS else 'not ok', '-', case_['name'])

    return Result(case_['name'], result, out, err, returncode, expected, cmd)


async def run_tests(args: Arguments, tests: TestDescription) -> bool:
    print(f'1..{len(tests["case"])}')

    results = typing.cast(
        'list[Result]',
        await asyncio.gather(*[test(args, c) for c in tests['case']]))

    encountered_failure: bool = False

    for r in results:
        if r.status is not Status.PASS:
            print(f'{r.name}:', file=sys.stderr)
            print('  result:', 'timeout' if r.status is Status.TIMEOUT else 'fail', file=sys.stderr)
            print('  returncode:', r.returncode, file=sys.stderr)
            print('  stdout:  ', r.stdout, file=sys.stderr)
            print('  expected:', r.expected, file=sys.stderr)
            print('  stderr:', r.stderr, file=sys.stderr)
            print('  command:', ' '.join(r.command), file=sys.stderr)
            print('\n', file=sys.stderr)

            encountered_failure = True

    return encountered_failure


async def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('runner', help="The compiled cps-config binary")
    parser.add_argument('cases', help="A toml file containing case descriptions")
    parser.add_argument('--libdir', default='lib', help="the build system configured libdir")
    parser.add_argument('--prefix', default=None, help="The prefix tests are realtive to")
    args: Arguments = parser.parse_args()

    with open(os.path.join(SOURCE_DIR, args.cases), 'rb') as f:
        tests = typing.cast('TestDescription', tomllib.load(f))

    with contextlib.ExitStack() as stack:
        # If the libdir is not "lib" (which tests assume), create a symlink to
        # the expected libdir
        if args.libdir != 'lib' or args.prefix:
            prefix = str(os.environ['CPS_PREFIX_PATH'])
            tmpdir = tempfile.mkdtemp()
            stack.callback(shutil.rmtree, tmpdir)
            os.environ['CPS_PREFIX_PATH'] = tmpdir

            # Also override the prefix, which is used to calculate @prefix@
            args.prefix = tmpdir

            # Handle libdir with multiple paths, like lib/x86_64-linux-gnu
            root, libdir = os.path.split(args.libdir)
            if root:
                tmpdir = os.path.join(tmpdir, root)
                os.makedirs(tmpdir, exist_ok=True)

            source = os.path.join(prefix, 'lib')
            dest = os.path.join(tmpdir, libdir)
            os.symlink(source, dest)
            stack.callback(os.unlink, dest)

        failed = await run_tests(args, tests)

    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    asyncio.run(main())
