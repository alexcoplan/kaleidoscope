#!/usr/bin/env python3

from util import debug, run
import os

def run_tests() -> None:
  run(["ninja"])
  debug("")

  build_dir = "build"
  configs = os.listdir(build_dir)
  for config in configs:
    debug(f"Checking config {config}")
    test_dir = f"{build_dir}/{config}/test"
    test_exes = os.listdir(test_dir)
    for exe in test_exes:
      if exe.endswith(".o"):
        continue
      run([f"{test_dir}/{exe}"])
      debug("")

if __name__ == '__main__':
  run_tests()
