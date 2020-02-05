#pragma once

#include <string>
#include <vector>

/// ExprAST - base class for all expression nodes
struct ExprAST {
  virtual ~ExprAST() {}
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
  double val;

public:
  NumberExprAST(double val) : val(val) {}
  double getVal() const { return val; }
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
  std::string name;

public:
  VariableExprAST(const std::string &name) : name(name) {}
  const std::string &getName() const { return name; }
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
  char op;
  std::unique_ptr<ExprAST> lhs, rhs;

public:
  BinaryExprAST(char op_,
      std::unique_ptr<ExprAST> lhs,
      std::unique_ptr<ExprAST> rhs) :
    op(op_),
    lhs(std::move(lhs)),
    rhs(std::move(rhs)) {}
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
  std::string callee;
  std::vector<std::unique_ptr<ExprAST>> args;

public:
  CallExprAST(const std::string &callee,
      std::vector<std::unique_ptr<ExprAST>> args) :
    callee(callee), args(std::move(args)) {}
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly
/// the number of arguments the function takes).
class PrototypeAST {
  std::string name;
  std::vector<std::string> args;

public:
  PrototypeAST(const std::string &name, std::vector<std::string> args) :
    name(name), args(std::move(args)) {}

  const std::string &getName() const { return name; }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
  std::unique_ptr<PrototypeAST> proto;
  std::unique_ptr<ExprAST> body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> proto,
      std::unique_ptr<ExprAST> body) :
    proto(std::move(proto)), body(std::move(body)) {}
};
