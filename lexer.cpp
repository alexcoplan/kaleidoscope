#include "lexer.hpp"

#include <string>
#include <istream>

int Lexer::gettok()
{
  while (isspace(lastChar))
    lastChar = input.get();

  if (isalpha(lastChar)) {
    // identifier: [a-zA-Z][a-zA-Z0-9]*
    identStr = lastChar;
    while (isalnum((lastChar = input.get())))
      identStr += lastChar;

    if (identStr == "def")
      return tok_def;
    if (identStr == "extern")
      return tok_extern;

    return tok_identifier;
  }

  if (isdigit(lastChar) || lastChar == '.') {
    // number: [0-9.]+
    std::string numStr;
    do {
      numStr += lastChar;
      lastChar = input.get();
    } while (isdigit(lastChar) || lastChar == '.');

    numVal = strtod(numStr.c_str(), 0);
    return tok_number;
  }

  if (lastChar == '#') {
    // Comment until end of line.
    do {
      lastChar = input.get();
    } while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

    // If there's more input, re-lex until we get something
    // other than a comment.
    if (lastChar != EOF)
      return gettok();
  }

  // Check for EOF, don't eat it.
  if (lastChar == EOF)
    return tok_eof;

  // Otherwise, just return the character as its ascii value.
  int thisChar = lastChar;
  lastChar = input.get();
  return thisChar;
}
