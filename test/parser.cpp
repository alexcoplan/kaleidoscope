#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "parser.hpp"

TEST_CASE("simple expressions", "[parser]")
{
  std::istringstream input("42");
  Parser parser(input);

  auto expr = parser.parseExpression();
  REQUIRE(expr != nullptr);

  auto num = dynamic_cast<NumberExprAST *>(expr.get());
  REQUIRE(num != nullptr);
  REQUIRE(num->getVal() == 42.0);
}
