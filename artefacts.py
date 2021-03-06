from configure import BuildEnv

def describe(env : BuildEnv) -> None:
  env.Test('test_lexer', ['test/lexer.cpp', 'lexer.cpp'])
  env.Test('test_parser', ['test/parser.cpp', 'lexer.cpp', 'parser.cpp'])
  env.Program('toy', ['toy.cpp', 'parser.cpp', 'lexer.cpp', 'codegen.cpp'])
