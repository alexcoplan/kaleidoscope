import subprocess
import os
import pprint

llvm_dir = "../../llvm-project/build/bin"
clang_exe = f"{llvm_dir}/clang"
opt_exe = f"{llvm_dir}/opt"
infile = "ajc_vargs.c"

clang_base_args = [clang_exe, "-c", "-S", "-Wall", "-Werror", infile]
opt_base_args = [opt_exe, "-S"]

class Config:
  def __init__(self, name, clang_args=[], opt_args=None, asm=False):
    self.name = name
    self.clang_args = clang_args
    self.opt_args = opt_args
    self.emit_asm = asm

prepare_O1 = ["-O1", "-Xclang", "-disable-llvm-passes"]

configs = [
  Config("base"),
  Config("O1", ["-O1"]),
  Config("prepareO1", prepare_O1),
  Config("prepareO1m2r", prepare_O1, ["--mem2reg"]),
  Config("armO1", ["-O1", "-target", "arm", "-mfloat-abi=hard"], asm=True),
]

def run(cmd):
  cmd_s = " ".join(cmd)
  print(cmd_s)
  subprocess.check_call(cmd)

cname, _ = infile.split('.')
for config in configs:
  llname = f"{cname}.{config.name}.ll"
  if os.path.exists(llname):
    print(f"Cleaning up old file: {llname}")
    os.unlink(llname)

  cmd = clang_base_args + ["-o", llname] + config.clang_args
  if not config.emit_asm:
    cmd += ["-emit-llvm"]

  run(cmd)

  if config.opt_args is not None:
    opt_cmd = opt_base_args + ["-o", llname, llname] + config.opt_args
    run(opt_cmd)

  with open(llname, 'r') as orig:
    ir = orig.read()
  with open(llname, 'w') as new:
    clang_arg_str = " ".join(cmd)
    new.write(f"; irexplore: clang args: {clang_arg_str}\n")
    if config.opt_args is not None:
      opt_arg_str = " ".join(opt_cmd)
      new.write(f"; irexplore: opt args: {opt_arg_str}\n")
    new.write("\n")
    new.write(ir)
