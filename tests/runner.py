#!/usr/bin/env python
# SPDX-License-Identifier: MIT
# Copyright © 2024 Dylan Baker

from __future__ import annotations

import argparse
import asyncio
import os
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

_PRINT_LOCK = asyncio.Lock()


async def test(runner: str, case_: TestCase):
    cmd = [runner, case_['mode'], os.path.join(TEST_DIR, case_['cps'])] + case_['args']
    proc = await asyncio.create_subprocess_exec(
        *cmd,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.DEVNULL,
    )
    bout, _ = await proc.communicate()
    out = bout.decode().strip()

    success = proc.returncode == 0 and out == case_['expected']
    async with _PRINT_LOCK:
        print('ok' if success else 'not ok', '-', case_['name'])


async def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('runner', help="The compiled cps-config binary")
    parser.add_argument('cases', help="A toml file containing case descriptions")
    args: Arguments = parser.parse_args()

    with open(os.path.join(SOURCE_DIR, args.cases), 'rb') as f:
        tests = typing.cast('TestDescription', tomllib.load(f))

    print(f'1..{len(tests["case"])}')

    await asyncio.gather(*[test(args.runner, c) for c in tests['case']])


if __name__ == "__main__":
    asyncio.run(main())