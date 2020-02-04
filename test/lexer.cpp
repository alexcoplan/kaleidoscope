#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "lexer.hpp"
#include "helpers.hpp"

TEST_CASE("hello world", "[lexer]")
{
  std::istringstream input("hello");
  Lexer lex(input);

  REQUIRE(lex.gettok() == tok_identifier);
  REQUIRE(lex.identStr == "hello");

  REQUIRE(lex.gettok() == tok_eof);
}

TEST_CASE("identifiers and keywords", "[lexer]")
{
  std::istringstream input("a b2 abc123 def extern wibble");
  Lexer lex(input);

  REQUIRE(lex.gettok() == tok_identifier);
  REQUIRE(lex.identStr == "a");

  REQUIRE(lex.gettok() == tok_identifier);
  REQUIRE(lex.identStr == "b2");

  REQUIRE(lex.gettok() == tok_identifier);
  REQUIRE(lex.identStr == "abc123");

  REQUIRE(lex.gettok() == tok_def);
  REQUIRE(lex.gettok() == tok_extern);

  REQUIRE(lex.gettok() == tok_identifier);
  REQUIRE(lex.identStr == "wibble");

  REQUIRE(lex.gettok() == tok_eof);
}

TEST_CASE("numbers", "[lexer]")
{
  std::istringstream input("1 .5 2.25 4. 8.0 # comments ignored");
  Lexer lex(input);

  static const auto expected = make_array<double>(
    1.0,
    0.5,
    2.25,
    4.0,
    8.0
  );
  for (const double e : expected) {
    REQUIRE(lex.gettok() == tok_number);
    REQUIRE(lex.numVal == e);
  }

  REQUIRE(lex.gettok() == tok_eof);
}
