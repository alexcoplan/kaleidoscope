#!/usr/bin/env python3

import argparse
import sys
import os
import platform
import copy
from enum import Enum
from collections import namedtuple
from typing import Dict, List, TextIO, Tuple, Optional

import artefacts # user's build definitions

base_flags  = "-g -Wall -Wextra -Wpedantic -Werror -fcolor-diagnostics"
base_flags += " -Wno-unused-parameter -Wno-error=unused-function"
base_flags += " -Wno-gnu-zero-variadic-macro-arguments"
base_flags += " -Wno-error=unused-private-field"
base_flags += " -Wno-unused-command-line-argument" # llvm on ubuntu
base_flags += " -Wno-unknown-warning-option" # llvm on ubuntu

llvm_config_exe = "llvm-config"
llvm_ldflags = f" `{llvm_config_exe} --ldflags --system-libs --libs core`"
llvm_cxxflags = f" `{llvm_config_exe} --cxxflags`"

ninja_vars = {
  "baseflags" : base_flags,
}

def cc_rule(name : str) -> str:
  flagvar = {
    "cc" : "cflags",
    "cxx" : "cxxflags",
  }[name]
  out = f"rule {name}\n"
  out += f"  command = ${name} -MMD -MT $out -MF $out.d ${flagvar} -c $in -o $out\n"
  out += f"  depfile = $out.d\n"
  out += f"  deps = gcc\n"
  return out

compile_rules = map(cc_rule, ["cc", "cxx"])
ninjafile_base = "cc = clang\n"
ninjafile_base += "cxx = clang++\n"
ninjafile_base += "\n" + "\n".join(compile_rules)
ninjafile_base += """
rule link
  command = $cxx $ldflags -o $out $in
  description = LINK $out

"""

class Sanitizer(Enum):
  address = 1
  undefined = 2
  fuzzer = 4

class SanitizerInfo:
  def __init__(self, arg : str) -> None:
    self.mask : int = 0
    self.cflags : str = ""
    self.ldflags : str = ""

    if len(arg) == 0:
      return

    sans = arg.split(',')
    for san in sans:
      assert san in Sanitizer.__members__, f"sanitizer {san} not supported"
      ty = Sanitizer.__members__[san]
      self.mask |= ty.value
      en_flag = f" -fsanitize={san}"
      self.cflags += en_flag
      if ty == Sanitizer.fuzzer:
        self.ldflags += " -fsanitize=fuzzer-no-link"
      else:
        self.ldflags += en_flag
      if ty == Sanitizer.undefined:
        no_recover = " -fno-sanitize-recover=undefined"
        self.cflags += no_recover
        self.ldflags += no_recover

class Program:
  def __init__(self, name : str, objects : List[str], ldflags : str = "") -> None:
    self.name = name
    self.objects = objects
    self.ldflags = ldflags

class ObjectFile:
  def __init__(self, src_file : str) -> None:
    self.name, self.ext = src_file.split(".")

  def is_cpp(self) -> bool:
    return self.ext == "cpp"

  def compile_rule(self) -> str:
    if self.ext == "c":
      return "cc"
    elif self.is_cpp():
      return "cxx"
    else:
      assert False, f"bad ext {self.ext}"

  def ninja_line(self) -> str:
    return f"{self.name}.o: {self.compile_rule()} {self.name}.{self.ext}\n"

class BuildConfig:
  def __init__(self, mode : str = 'debug', sanitizers : str = "") -> None:
    assert mode in ['debug', 'release']
    self.mode = mode
    self.san_info = SanitizerInfo(sanitizers)
    self.vars = copy.deepcopy(ninja_vars)
    self.vars["builddir"] = f"build/{self.name()}"
    self.vars["cflags"] = f"${self.name()}_baseflags -std=c11"
    self.vars["cxxflags"] = f"${self.name()}_baseflags {llvm_cxxflags}"
    self.vars["ldflags"] = f"-L${self.name()}_builddir {llvm_ldflags}"
    self.vars["baseflags"] += " -Iinclude"
    self.vars["baseflags"] += self.san_info.cflags
    self.vars["ldflags"] += self.san_info.ldflags
    if self.mode == "release":
      self.vars["baseflags"] += ' -O3'

  def name(self) -> str:
    if self.san_info.mask == 0:
      return self.mode
    san_flags = ""
    for name, item in Sanitizer.__members__.items():
      if (self.san_info.mask & item.value) != 0:
        san_flags += name[0]

    return f"{self.mode}-{san_flags}san"

class BuildEnv:
  def __init__(self, configs : List[BuildConfig]) -> None:
    self.configs = configs

    # maps object file names -> ObjectFile objects
    self.obj_map : Dict[str, ObjectFile] = {}
    self.progs : List[Program] = []


  def IsWindows(self) -> bool:
    return os.name == 'nt'

  def IsLinux(self) -> bool:
    return platform.system() == 'Linux'

  def Program(self, name : str, src : List[str], ldflags : str = "") -> None:
    objects = []
    for f in src:
      obj = ObjectFile(f)
      objects.append(obj)
      if obj.name not in self.obj_map:
        self.obj_map[obj.name] = obj

    need_libcpp = any(map(lambda x: x.is_cpp(), objects))
    if need_libcpp:
      spacer = " " if len(ldflags) > 0 else ""
      maybe_std = "std" if self.IsLinux() else ""
      ldflags += f"{spacer}-l{maybe_std}c++" # XXX: win32?

    obj_names = list(map(lambda x: x.name, objects))
    self.progs.append(Program(name, obj_names, ldflags=ldflags))

  def Test(self, name : str, src : List[str]) -> None:
    return self.Program("test/%s" % name, src)

  def FuzzTarget(self, name : str, src : List[str]) -> None:
    return self.Program(f"fuzz/{name}", src, ldflags="-fsanitize=fuzzer")

  def write_ninja(self, fp : TextIO) -> None:
    fp.write("# auto-generated by configure.py\n")

    fp.write(ninjafile_base)

    reconf_cmd = "./configure.py"
    if len(sys.argv) > 1:
      conf_args = " ".join(sys.argv[1:])
      reconf_cmd += f" {conf_args}"

    fp.write("rule reconf\n")
    fp.write(f"  command = {reconf_cmd}\n")
    fp.write(f"  generator = 1\n")

    fp.write("\n# re-configure if necessary\n")
    fp.write("build build.ninja : reconf configure.py artefacts.py\n\n")

    for config in self.configs:
      fp.write("###\n")
      fp.write(f"### *** start config '{config.name()}' ***\n")
      fp.write("###\n")

      fp.write("\n# vars\n")
      for k,v in config.vars.items():
        fp.write(f"{config.name()}_{k} = {v}\n")

      builddir = f"${config.name()}_builddir"
      ldflags = f"${config.name()}_ldflags"

      fp.write("\n# objects\n")
      for obj in self.obj_map.values():
        fp.write(f"build {builddir}/{obj.ninja_line()}")
        fp.write(f"  cflags = ${config.name()}_cflags\n")
        fp.write(f"  cxxflags = ${config.name()}_cxxflags\n")

      fp.write("\n# executables\n")
      for prog in self.progs:
        obj_line = " ".join(map(lambda x: f"{builddir}/{x}.o", prog.objects))
        ext = ".exe" if self.IsWindows() else ""
        fp.write(f"build {builddir}/{prog.name}{ext}: link {obj_line}\n")
        fp.write(f"  ldflags = {prog.ldflags} {ldflags}\n")

      fp.write("\n")

def configure_build(args : argparse.Namespace) -> None:
  ausan = "address,undefined"
  default_configs = [
    BuildConfig(),
    BuildConfig(mode='release'),
    BuildConfig(sanitizers=ausan),
    BuildConfig(mode='release', sanitizers=ausan),
  ]
  env = BuildEnv(default_configs)
  artefacts.describe(env)
  with open("build.ninja", "w") as f:
    env.write_ninja(f)

if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('--sanitizers', '--san', dest='sanitizers', default=None)
  parser.add_argument('--config', default='debug')
  args = parser.parse_args()
  configure_build(args)
