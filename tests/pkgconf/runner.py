#!/usr/bin/env python3

# Copyright (c) 2016 pkgconf authors (see AUTHORS).
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# This software is provided 'as is' and without any warranty, express or
# implied.  In no event shall the authors be liable for any damages arising
# from the use of this software.

"""Simple runner for test cases."""

from __future__ import annotations
import argparse
import asyncio
import dataclasses
import os
import sys
import typing

try:
	import tomllib
except ImportError:
	import tomli as tomllib  # type: ignore

if typing.TYPE_CHECKING:

	class TestDefinition(typing.TypedDict, total=False):
		exitcode: int
		stdout: str
		stdout_contains: str
		stderr: str
		args: typing.List[str]
		env: typing.Dict[str, str]

	class TestSetup(typing.TypedDict, total=False):
		env: typing.Dict[str, str]

	class Arguments(typing.Protocol):
		suites: typing.List[str]
		pkgconf: str
		verbose: bool


TEST_ROOT = os.path.dirname(os.path.abspath(__file__))


@dataclasses.dataclass
class Result:

	name: str
	success: bool = True
	reason: typing.Optional[str] = None


def interpolate(input: str) -> str:
	return input.format(
		test_root=TEST_ROOT,
	)


async def run_test(pkgconf: str, name: str, suite: str, test: TestDefinition, verbose: bool, setup: TestSetup) -> Result:

	env: typing.Dict[str, str] = {}
	if (renv := setup.get('env')) is not None:
		env.update({k: interpolate(v) for k, v in renv.items()})
	if (renv := test.get('env', None)) is not None:
		env.update({k: interpolate(v) for k, v in renv.items()})

	proc = await asyncio.create_subprocess_exec(
		pkgconf, 'pkg-config', *[interpolate(a) for a in test.get('args', [])],
		stdout=asyncio.subprocess.PIPE,
		stderr=asyncio.subprocess.PIPE,
		env=env)

	rout, rerr = await proc.communicate()
	out = rout.decode() if rout is not None else '<no stdout>'
	err = rerr.decode() if rerr is not None else '<no stderr>'

	result = Result(name)

	if (ret := test.get('exitcode', 0)) and proc.returncode != ret:
		result.reason = f"Return code was {proc.returncode}, but expected {ret}"

	# We either check contains, or we check match, but not both
	if (exp := test.get('stdout_contains')) is not None:
		exp = interpolate(exp)
		if exp not in out:
			result.reason = f"Stdout was {out!r}, which does not contain {exp!r}"
	elif (exp := test.get('stdout', '')) != '<ignore>':
		exp = interpolate(exp)
		if out != exp:
			result.reason = f"Stdout was {out!r}, but expected {exp!r}"

	if (exp := test.get('stderr', '')) != '<ignore>':
		exp = interpolate(exp)
		if err != exp:
			result.reason = f"Stderr was {err!r}, but expected {exp!r}"

	result.success = result.reason is None

	if result.success:
		if verbose:
			print(f"{suite}.{name}: passed")
	else:
		print(f"{suite}.{name}: failed\n  reason: {result.reason}")

	return result


async def run(args: Arguments) -> bool:
	tests: typing.List[typing.Coroutine[typing.Any, typing.Any, Result]] = []
	for suitef in args.suites:
		with open(suitef, 'rb') as f:
			suite: typing.Dict[str, TestDefinition] = tomllib.load(f)

		setup: TestSetup = suite.pop('setup')
		suite_name = os.path.splitext(os.path.basename(suitef))[0]

		tests.extend(
			[run_test(args.pkgconf, n, suite_name, s, args.verbose, setup) for n, s in suite.items()])

	results = await asyncio.gather(*tests)
	totals = {'failed': 0, 'success': 0}
	for r in results:
		if r.success:
			totals['success'] += 1
		else:
			totals['failed'] += 1

	print(f'\nTotals\n  Passed: {totals["success"]}\n  Failed: {totals["failed"]}')

	return totals['failed'] == 0


def main() -> None:
	parser = argparse.ArgumentParser()
	parser.add_argument('pkgconf', help="Path to built pkgconf executable")
	parser.add_argument('suites', nargs="+", help="One or more toml test suite definition")
	parser.add_argument('-v', '--verbose', action='store_true',
						help="Print more information while running")
	args: Arguments = parser.parse_args()

	loop = asyncio.new_event_loop()
	success = loop.run_until_complete(run(args))

	sys.exit(int(not success))


if __name__ == "__main__":
	main()
