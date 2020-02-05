#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Verifier.h"

#include "ast.hpp"
#include "codegen.hpp"

static llvm::Value *logErrorV(const char *str)
{
  fprintf(stderr, "codegen error: %s\n", str);
  return nullptr;
}

static llvm::Function *logErrorF(const char *str)
{
  logErrorV(str);
  return nullptr;
}

// AP stands for 'arbitrary precision',
llvm::Value *NumberExprAST::codegen(CodeGenContext &ctx) const
{
  return llvm::ConstantFP::get(ctx.llvmCtx, llvm::APFloat(val));
}

llvm::Value *VariableExprAST::codegen(CodeGenContext &ctx) const
{
  llvm::Value *v = ctx.namedValues[name];
  if (!v)
    logErrorV("Unknown variable name");

  return v;
}

llvm::Value *BinaryExprAST::codegen(CodeGenContext &ctx) const
{
  llvm::Value *left = lhs->codegen(ctx);
  llvm::Value *right = rhs->codegen(ctx);
  if (!left || !right)
    return nullptr;

  switch (op) {
    case '+':
      return ctx.builder.CreateFAdd(left, right, "addtmp");
    case '-':
      return ctx.builder.CreateFSub(left, right, "subtmp");
    case '*':
      return ctx.builder.CreateFMul(left, right, "multmp");
    case '<':
      left = ctx.builder.CreateFCmpULT(left, right, "cmptmp");
      // convert i1 0/1 to double 0.0 or 1.0
      return ctx.builder.CreateUIToFP(left,
          llvm::Type::getDoubleTy(ctx.llvmCtx),
          "booltmp");
    default:
      return logErrorV("invalid binary operator");
  }
}

llvm::Value *CallExprAST::codegen(CodeGenContext &ctx) const
{
  llvm::Function *calleeF = ctx.module->getFunction(callee);
  if (!calleeF)
    return logErrorV("Unknown function referenced");

  if (calleeF->arg_size() != args.size())
    return logErrorV("Incorrect # of args passed");

  const unsigned num_args = args.size();
  std::vector<llvm::Value *> argsV;

  for (unsigned i = 0; i < num_args; ++i) {
    argsV.push_back(args[i]->codegen(ctx));
    if (!argsV.back()) // if the last call to codegen() failed...
      return nullptr;
  }

  return ctx.builder.CreateCall(calleeF, argsV, "calltmp");
}

llvm::Function *PrototypeAST::codegen(CodeGenContext &ctx) const
{
  // Construct the function type: double(double, double, ...)
  std::vector<llvm::Type *> doubles(args.size(),
      llvm::Type::getDoubleTy(ctx.llvmCtx));

  llvm::FunctionType *ft =
    llvm::FunctionType::get(llvm::Type::getDoubleTy(ctx.llvmCtx),
        doubles,
        false /* not varargs */);

  llvm::Function *f = llvm::Function::Create(ft,
      llvm::Function::ExternalLinkage, name, ctx.module.get());

  // Set names for all arguments, should make the IR more readable.
  unsigned i = 0;
  for (auto &arg : f->args())
    arg.setName(args[i++]);

  return f;
}

llvm::Function *FunctionAST::codegen(CodeGenContext &ctx) const
{
  // First, check for an existing function from a previous 'extern' declaration.
  llvm::Function *theFunction = ctx.module->getFunction(proto->getName());

  if (!theFunction)
    theFunction = proto->codegen(ctx);

  if (!theFunction)
    return nullptr; // codegen failed

  if (!theFunction->empty())
    return logErrorF("Function cannot be redefined.");

  // Create a new basic block to start insertion into.
  //
  // For now, we don't have any control flow, so we can just insert directly
  // into the one basic block!
  llvm::BasicBlock *bb = llvm::BasicBlock::Create(ctx.llvmCtx,
      "entry", theFunction);
  ctx.builder.SetInsertPoint(bb);

  // Record the function arguments in the NamedValues map.
  ctx.namedValues.clear();
  for (auto &arg : theFunction->args())
    ctx.namedValues[arg.getName()] = &arg;

  if (llvm::Value *retVal = body->codegen(ctx)) {
    // Finish off the function.
    ctx.builder.CreateRet(retVal);

    // Validate the generate code, checking for consistency.
    llvm::verifyFunction(*theFunction);
    return theFunction;
  }

  // Error in codegen of body, remove function.
  theFunction->eraseFromParent();
  return nullptr;
}
