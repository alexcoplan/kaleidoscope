#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "parser.hpp"

static llvm::Value *stub()
{
  assert(false && "stub!");
  return nullptr;
}

// codegen stubs:
llvm::Value *NumberExprAST::codegen(CodeGenContext &) const { return stub(); }
llvm::Value *VariableExprAST::codegen(CodeGenContext &) const { return stub(); }
llvm::Value *BinaryExprAST::codegen(CodeGenContext &) const { return stub(); }
llvm::Value *CallExprAST::codegen(CodeGenContext &) const { return stub(); }

TEST_CASE("meaning of life", "[parser]")
{
  std::istringstream input("42");
  Parser parser(input);

  auto expr = parser.parseExpression();
  REQUIRE(expr != nullptr);

  REQUIRE(expr->describe() == "42");
}

TEST_CASE("addition", "[parser]")
{
  std::istringstream input("4 + 8");
  Parser parser(input);

  auto expr = parser.parseExpression();
  REQUIRE(expr != nullptr);
  REQUIRE(expr->describe() == "+(4,8)");

  input = std::istringstream("3 ^ 2");
  auto bad = parser.parseDefinition();
  REQUIRE(bad == nullptr);
}
