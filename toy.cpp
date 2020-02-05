#include "codegen.hpp"
#include "parser.hpp"

#include <iostream>

class Repl
{
  Parser parser;
  CodeGenContext cg;

  void handleDefinition();
  void handleExtern();
  void handleTopLevelExpr();
  void loop();

public:
  Repl() : parser(std::cin) {}
  void run();
};

void Repl::handleDefinition()
{
  if (auto fnAST = parser.parseDefinition()) {
    if (auto *fnIR = fnAST->codegen(cg)) {
      fprintf(stderr, "Read function def:");
      fnIR->print(llvm::errs());
    }
  } else {
    // Skip token for error recover.
    parser.getNextToken();
  }
}

void Repl::handleExtern()
{
  if (auto protoAST = parser.parseExtern()) {
    if (auto *fnIR = protoAST->codegen(cg)) {
      fprintf(stderr, "Read extern: ");
      fnIR->print(llvm::errs());
    }
  } else {
    // Skip for recovery.
    parser.getNextToken();
  }
}

void Repl::handleTopLevelExpr()
{
  if (auto fnAST = parser.parseTopLevelExpr()) {
    if (auto *fnIR = fnAST->codegen(cg)) {
      fprintf(stderr, "Read top-level expression: ");
      fnIR->print(llvm::errs());
    }
  } else {
    // Skip for recovery.
    parser.getNextToken();
  }
}

void Repl::loop()
{
  while (true) {
    const int tok = parser.peekTok();
    if (tok != tok_eof) {
      fprintf(stderr, "ready> ");
    }

    switch (parser.peekTok()) {
      case tok_eof:
        return;
      case ';': // ignore top-level semicolons
        parser.getNextToken();
        break;
      case tok_def:
        handleDefinition();
        break;
      case tok_extern:
        handleExtern();
        break;
      default:
        handleTopLevelExpr();
        break;
    }
  }
}

void Repl::run()
{
  loop();
  fprintf(stderr, "All done. Here's your LLVM module:\n\n");
  cg.module->print(llvm::errs(), nullptr);
}

int main()
{
  fprintf(stderr, "ready> ");
  Repl repl;
  repl.run();
  return 0;
}
