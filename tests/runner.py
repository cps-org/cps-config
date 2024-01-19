#!/usr/bin/env python
# SPDX-License-Identifier: MIT
# Copyright © 2024 Dylan Baker

from __future__ import annotations

import argparse
import asyncio
import dataclasses
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
        mode: typing.Literal['pkgconf']

    class TestDescription(typing.TypedDict):

        case: list[TestCase]


TEST_DIR = os.path.dirname(__file__)
SOURCE_DIR = os.path.dirname(TEST_DIR)


@dataclasses.dataclass(slots=True)
class Result:

    name: str
    success: bool
    stdout: str | None
    stderr: str | None


async def test(runner: str, case_: TestCase) -> Result:
    cmd = [runner, case_['mode'], os.path.join(TEST_DIR, case_['cps'])] + case_['args']
    proc = await asyncio.create_subprocess_exec(
        *cmd,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE,
    )
    bout, berr = await proc.communicate()

    out = bout.decode().strip()
    err = berr.decode().strip()

    return Result(
        case_['name'],
        proc.returncode == 0 and out == case_['expected'],
        out, err)


async def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('runner', help="The compiled cps-config binary")
    parser.add_argument('cases', help="A toml file containing case descriptions")
    args: Arguments = parser.parse_args()

    with open(os.path.join(SOURCE_DIR, args.cases), 'rb') as f:
        tests = typing.cast('TestDescription', tomllib.load(f))

    results = typing.cast('list[Result]', sorted(await asyncio.gather(*[test(args.runner, c) for c in tests['case']])))
    success = True
    for r in results:
        print(f'{r.name}:', 'OK' if r.success else 'Error')
        if not r.success:
            print('  stdout:', r.stderr)
            print('  stderr:', r.stderr)
            success = False

    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(asyncio.run(main()))