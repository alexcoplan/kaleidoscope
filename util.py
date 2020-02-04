import sys
import subprocess
from typing import List

def debug(msg : str) -> None:
  sys.stderr.write("%s\n" % msg)

def run(cmd : List[str]) -> None:
  debug("$ %s" % " ".join(cmd))
  subprocess.check_call(cmd)
