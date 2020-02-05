#pragma once

#include "lexer.hpp"
#include "ast.hpp"

class Parser {
  int curTok;
  Lexer lex;

  int getNextToken();
  int getTokPrecedence();
  std::unique_ptr<ExprAST> parseNumberExpr();
  std::unique_ptr<ExprAST> parseParenExpr();
  std::unique_ptr<ExprAST> parseIdentifierExpr();
  std::unique_ptr<ExprAST> parsePrimary();
  std::unique_ptr<ExprAST> parseBinOpRHS(int prec, std::unique_ptr<ExprAST> lhs);
  std::unique_ptr<PrototypeAST> parsePrototype();
  std::unique_ptr<FunctionAST> parseDefinition();
  std::unique_ptr<PrototypeAST> parseExtern();
  std::unique_ptr<FunctionAST> parseTopLevelExpr();

public:
  std::unique_ptr<ExprAST> parseExpression();
  Parser(std::istream &input) : lex(input) {
    getNextToken(); // prime with the first token
  }
};
