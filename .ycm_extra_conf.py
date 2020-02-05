import subprocess

llvm_cxxflags = subprocess.check_output(["llvm-config", "--cxxflags"]).split()

base_flags = [
'-Wall',
'-Wextra',
'-Wpedantic',
'-Werror',
'-I.',
'-Iinclude'
]

c_flags = [
  '-std=c11',
]

cpp_flags = base_flags + llvm_cxxflags

def FlagsForFile(filename):
  cpp_exts = ["cpp", "hpp"]
  ext = filename.split(".")[-1]
  lang_flags = cpp_flags if ext in cpp_exts else c_flags
  flags = base_flags + lang_flags
  return { 'flags': flags, 'do_cache': True }
