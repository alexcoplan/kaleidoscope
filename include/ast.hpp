#pragma once

#include <string>
#include <vector>

// Forward declarations.
struct CodeGenContext;
namespace llvm {
  class Value;
  class Function;
};

/// ExprAST - base class for all expression nodes
struct ExprAST {
  virtual ~ExprAST() {}

  /// Construct a textual representation of the AST fragment,
  /// intended for testing and debugging.
  virtual std::string describe() const = 0;
  virtual llvm::Value *codegen(CodeGenContext &ctx) const = 0;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
  double val;

public:
  NumberExprAST(double val) : val(val) {}
  double getVal() const { return val; }
  virtual std::string describe() const;
  virtual llvm::Value *codegen(CodeGenContext &ctx) const;
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
  std::string name;

public:
  VariableExprAST(const std::string &name) : name(name) {}
  const std::string &getName() const { return name; }
  virtual std::string describe() const;
  virtual llvm::Value *codegen(CodeGenContext &ctx) const;
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
  char getOp() const { return op; }
  const ExprAST *getLHS() const { return lhs.get(); }
  const ExprAST *getRHS() const { return rhs.get(); }
  virtual std::string describe() const;
  virtual llvm::Value *codegen(CodeGenContext &ctx) const;
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
  std::string callee;
  std::vector<std::unique_ptr<ExprAST>> args;

public:
  CallExprAST(const std::string &callee,
      std::vector<std::unique_ptr<ExprAST>> args) :
    callee(callee), args(std::move(args)) {}
  virtual std::string describe() const;
  virtual llvm::Value *codegen(CodeGenContext &ctx) const;
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
  llvm::Function *codegen(CodeGenContext &ctx) const;
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
  std::unique_ptr<PrototypeAST> proto;
  std::unique_ptr<ExprAST> body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> proto,
      std::unique_ptr<ExprAST> body) :
    proto(std::move(proto)), body(std::move(body)) {}
  llvm::Function *codegen(CodeGenContext &ctx) const;
};
