#pragma once

#include <istream>

enum Token {
  tok_eof = -1,

  tok_def = -2,
  tok_extern = -3,

  tok_identifier = -4,
  tok_number = -5,
};

class Lexer {
  std::istream &input;
  int lastChar = ' ';

public:
  std::string identStr;
  double numVal;

  Lexer(std::istream &input) : input(input) {}
  int gettok();
};
