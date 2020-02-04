from configure import BuildEnv

def describe(env : BuildEnv) -> None:
  env.Test('test_lexer', ['test/lexer.cpp', 'lexer.cpp'])
