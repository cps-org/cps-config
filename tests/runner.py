#!/usr/bin/env python
# SPDX-License-Identifier: MIT
# Copyright © 2024 Dylan Baker

from __future__ import annotations

import argparse
import asyncio
import dataclasses
import enum
import os
import sys
import tomllib
import typing

if typing.TYPE_CHECKING:

    class Arguments(typing.Protocol):

        runner: str
        cases: str

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
PREFIX = os.path.join(SOURCE_DIR, 'tests/cps-files')
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

def unordered_compare(out, expected):
    if out == expected:
        return True

    out_parts = out.split()
    expected_parts = expected.split()
    return sorted(out_parts) == sorted(expected_parts)

async def test(runner: str, case_: TestCase) -> Result:
    cmd = [runner] + case_['args'] + [case_['cps']]
    if 'mode' in case_:
        cmd.extend([f"--format={case_['mode']}"])

    expected = case_['expected'].format(prefix=PREFIX)

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


async def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('runner', help="The compiled cps-config binary")
    parser.add_argument('cases', help="A toml file containing case descriptions")
    args: Arguments = parser.parse_args()

    with open(os.path.join(SOURCE_DIR, args.cases), 'rb') as f:
        tests = typing.cast('TestDescription', tomllib.load(f))

    print(f'1..{len(tests["case"])}')

    results = typing.cast(
        'list[Result]',
        await asyncio.gather(*[test(args.runner, c) for c in tests['case']]))

    encountered_failure: bool = False

    for r in results:
        if r.status is not Status.PASS:
            print(f'{r.name}:', file=sys.stderr)
            print('  result:', 'timeout' if r.status is Status.TIMEOUT else 'fail')
            print('  returncode:', r.returncode, file=sys.stderr)
            print('  stdout:  ', r.stdout, file=sys.stderr)
            print('  expected:', r.expected, file=sys.stderr)
            print('  stderr:', r.stderr, file=sys.stderr)
            print('  command:', ' '.join(r.command), file=sys.stderr)
            print('\n')

            encountered_failure = True

    if encountered_failure:
        exit(1)

if __name__ == "__main__":
    asyncio.run(main())
