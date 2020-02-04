import sys
import subprocess

def debug(msg):
  sys.stderr.write("%s\n" % msg)

def run(cmd):
  debug("$ %s" % " ".join(cmd))
  subprocess.check_call(cmd)
