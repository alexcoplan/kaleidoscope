#pragma once

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"

struct CodeGenContext {
  llvm::LLVMContext llvmCtx;
  llvm::IRBuilder<> builder;
  std::unique_ptr<llvm::Module> module;
  std::map<std::string, llvm::Value *> namedValues; // symbol table

  CodeGenContext() :
    builder(llvmCtx),
    module(new llvm::Module("my cool jit", llvmCtx)) {}
};
