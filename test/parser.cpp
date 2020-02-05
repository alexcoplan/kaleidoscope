#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "parser.hpp"

TEST_CASE("meaning of life", "[parser]")
{
  std::istringstream input("42");
  Parser parser(input);

  auto expr = parser.parseExpression();
  REQUIRE(expr != nullptr);

  auto num = dynamic_cast<NumberExprAST *>(expr.get());
  REQUIRE(num != nullptr);
  REQUIRE(num->getVal() == 42.0);
}

TEST_CASE("addition", "[parser]")
{
  std::istringstream input("4 + 8");
  Parser parser(input);

  auto expr = parser.parseExpression();
  REQUIRE(expr != nullptr);

  auto binexp = dynamic_cast<BinaryExprAST *>(expr.get());
  REQUIRE(binexp != nullptr);

  REQUIRE(binexp->getOp() == '+');

  auto lhs = binexp->getLHS();
  REQUIRE(lhs != nullptr);

  auto left_num = dynamic_cast<const NumberExprAST *>(lhs);
  REQUIRE(left_num != nullptr);
  REQUIRE(left_num->getVal() == 4.0);

  auto rhs = binexp->getRHS();
  REQUIRE(rhs != nullptr);

  auto right_num = dynamic_cast<const NumberExprAST *>(rhs);
  REQUIRE(right_num != nullptr);
  REQUIRE(right_num->getVal() == 8.0);
}
