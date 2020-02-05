#include <cstdio>
#include <memory>
#include <map>

#include "parser.hpp"

static std::unique_ptr<ExprAST> logError(const char *str)
{
  fprintf(stderr, "LogError: %s\n", str);
  return nullptr;
}

static std::unique_ptr<PrototypeAST> logErrorP(const char *str)
{
  logError(str);
  return nullptr;
}

int Parser::getNextToken()
{
  return curTok = lex.gettok();
}

int Parser::getTokPrecedence()
{
  if (!isascii(curTok))
    return -1;

  static const std::map<char, int> binopPrecedence = {
    { '<', 10 },
    { '+', 20 },
    { '-', 20 },
    { '*', 40 },
  };

  int tokPrec;
  try {
    tokPrec = binopPrecedence.at(curTok);
  } catch (const std::out_of_range &e) {
    tokPrec = -1;
  }
  return tokPrec;
}

// Expects to be acalled when the current token is a tok_number.
// numberexpr ::= number
std::unique_ptr<ExprAST> Parser::parseNumberExpr()
{
  auto result = std::make_unique<NumberExprAST>(lex.numVal);
  getNextToken();
  return std::move(result);
}

/// parenexpr ::= '(' expression ')'
//
// Expects to be called when ( has been detected.
std::unique_ptr<ExprAST> Parser::parseParenExpr()
{
  getNextToken(); // eat '('
  auto v = parseExpression();
  if (!v)
    return nullptr;

  if (curTok != ')')
    return logError("expected ')'");

  getNextToken(); // eat ')'
  return v;
}

std::unique_ptr<ExprAST> Parser::parseIdentifierExpr()
{
  // Need to save identStr here because we're about to call getNextToken()
  // which can overwrite it.
  const std::string idName = lex.identStr;

  getNextToken(); // eat identifier

  if (curTok != '(') // simple variable ref
    return std::make_unique<VariableExprAST>(idName);

  // Function call...
  getNextToken(); // eat (
  std::vector<std::unique_ptr<ExprAST>> args;
  if (curTok != ')') {
    while (1) {
      if (auto arg = parseExpression()) {
        args.push_back(std::move(arg));
      } else {
        return nullptr;
      }

      if (curTok == ')')
        break;

      if (curTok != ',')
        return logError("Expected ')' or ',' in argument list.");

      getNextToken();
    }
  }

  getNextToken(); // eat ')'
  return std::make_unique<CallExprAST>(idName, std::move(args));
}

std::unique_ptr<ExprAST> Parser::parsePrimary()
{
  switch (curTok) {
    case tok_identifier:
      return parseIdentifierExpr();
    case tok_number:
      return parseNumberExpr();
    case '(':
      return parseParenExpr();
    default:
      return logError("Unknown token when expecting an expression.");
  }
}

/// expression ::= primary binoprhs
std::unique_ptr<ExprAST> Parser::parseExpression()
{
  auto lhs = parsePrimary();
  if (!lhs)
    return nullptr;

  return parseBinOpRHS(0, std::move(lhs));
}

std::unique_ptr<ExprAST> Parser::parseBinOpRHS(int prec,
    std::unique_ptr<ExprAST> lhs)
{
  while (1) {
    // If this is a binop, find its precedence.
    int tokPrec = getTokPrecedence();

    // If this is a binop that binds at least as tightly as the current binop,
    // consume it, otherwise we are done.
    if (tokPrec < prec)
      return lhs;

    // OK, we know this is a binop.
    int binOp = curTok;
    getNextToken();

    // Parse the primary expression after the binary operator.
    auto rhs = parsePrimary();
    if (!rhs)
      return nullptr;

    // If binOp binds less tightly with the RHS than the operator after RHS,
    // let the binding operator take RHS as its LHS.
    int nextPrec = getTokPrecedence();
    if (tokPrec < nextPrec) {
      rhs = parseBinOpRHS(prec+1, std::move(rhs));
      if (!rhs)
        return nullptr;
    }

    // Merge lhs/rhs.
    lhs = std::make_unique<BinaryExprAST>(binOp, std::move(lhs), std::move(rhs));
  }
}

/// prototype ::= id '(' id* ')'
std::unique_ptr<PrototypeAST> Parser::parsePrototype()
{
  if (curTok != tok_identifier)
    return logErrorP("Expected function name in prototype");

  const std::string fnName = lex.identStr;
  getNextToken();

  if (curTok!= '(')
    return logErrorP("Expected '(' in prototype");

  // Read the list of argument names.
  std::vector<std::string> argNames;
  while (getNextToken() == tok_identifier)
    argNames.push_back(lex.identStr);

  if (curTok != ')')
    return logErrorP("Expected ')' in prototype");

  // success.
  getNextToken(); // eat ')'
  return std::make_unique<PrototypeAST>(fnName, std::move(argNames));
}

/// definition ::= 'def' prototype expression
std::unique_ptr<FunctionAST> Parser::parseDefinition()
{
  getNextToken(); // eat 'def'

  auto proto = parsePrototype();
  if (!proto)
    return nullptr;

  if (auto e = parseExpression())
    return std::make_unique<FunctionAST>(std::move(proto), std::move(e));

  return nullptr;
}

/// external ::= 'extern' prototype
std::unique_ptr<PrototypeAST> Parser::parseExtern()
{
  getNextToken(); // eat extern
  return parsePrototype();
}

/// topLevelExpr ::= expression
std::unique_ptr<FunctionAST> Parser::parseTopLevelExpr()
{
  if (auto e = parseExpression()) {
    // Make an anonymous proto.
    auto proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(proto), std::move(e));
  }
  return nullptr;
}
