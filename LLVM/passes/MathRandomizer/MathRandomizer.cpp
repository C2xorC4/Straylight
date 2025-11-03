// MathObfuscator.cpp
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/MathExtras.h"  // countTrailingZeros
#include <random>

using namespace llvm;

// Manual count trailing zeros for uint64_t (LLVM 21 compatible)
static uint64_t countTrailingZeros(uint64_t Value) {
  if (Value == 0) return 64;
  return __builtin_ctzll(Value);  // Built-in for uint64_t
}

struct MathObfuscator : PassInfoMixin<MathObfuscator> {
  std::mt19937 rng;
  std::uniform_int_distribution<int> dist;

  MathObfuscator() : dist(0, 2) {
    std::random_device rd;
    rng.seed(rd());
  }

  bool isSameLoadSource(Value *LHS, Value *RHS) {
    auto *Load1 = dyn_cast<LoadInst>(LHS);
    auto *Load2 = dyn_cast<LoadInst>(RHS);
    return Load1 && Load2 && Load1->getPointerOperand() == Load2->getPointerOperand();
  }

  Value* getBaseValue(Value *LHS, Value *RHS) {
    if (LHS == RHS) return LHS;
    if (isSameLoadSource(LHS, RHS)) return LHS;
    return nullptr;
  }

  Value* obfuscateAddSame(IRBuilder<>& Builder, Value* X) {
    int choice = dist(rng);
    Type* Ty = X->getType();

    switch (choice) {
      case 0: {
        errs() << "x + x → x * 2\n";
        return Builder.CreateMul(X, ConstantInt::get(Ty, 2));
      }
      case 1: {
        errs() << "x + x → x << 1\n";
        return Builder.CreateShl(X, ConstantInt::get(Ty, 1));
      }
      case 2: {
        errs() << "x + x → (x * 3) - x\n";
        Value* X3 = Builder.CreateMul(X, ConstantInt::get(Ty, 3));
        return Builder.CreateSub(X3, X);
      }
    }
    return nullptr;
  }

  Value* obfuscateAddDiff(IRBuilder<>& Builder, Value* A, Value* B) {
    int choice = dist(rng);

    switch (choice) {
      case 0: {
        errs() << "a + b → a - (-b)\n";
        Value* NegB = Builder.CreateNeg(B);
        return Builder.CreateSub(A, NegB);
      }
      case 1: {
        errs() << "a + b → (a ^ b) ^ (a ^ b)\n";
        Value* AB = Builder.CreateXor(A, B);
        return Builder.CreateXor(AB, AB);
      }
      case 2: {
        errs() << "a + b → a * 1 + b * 1\n";
        Value* A1 = Builder.CreateMul(A, ConstantInt::get(A->getType(), 1));
        Value* B1 = Builder.CreateMul(B, ConstantInt::get(B->getType(), 1));
        return Builder.CreateAdd(A1, B1);
      }
    }
    return nullptr;
  }

  Value* obfuscateSubSame(IRBuilder<>& Builder, Value* X) {
    int choice = dist(rng);
    Type* Ty = X->getType();

    switch (choice) {
      case 0: {
        errs() << "x - x → 0\n";
        return ConstantInt::get(Ty, 0);
      }
      case 1: {
        errs() << "x - x → x ^ x\n";
        return Builder.CreateXor(X, X);
      }
      case 2: {
        errs() << "x - x → x + (~x + 1)\n";
        Value* NotX = Builder.CreateNot(X);
        Value* NegX = Builder.CreateAdd(NotX, ConstantInt::get(Ty, 1));
        return Builder.CreateAdd(X, NegX);
      }
    }
    return nullptr;
  }

  Value* obfuscateSubDiff(IRBuilder<>& Builder, Value* A, Value* B) {
    errs() << "a - b → a + (~b + 1)\n";
    Value* NotB = Builder.CreateNot(B);
    Value* NegB = Builder.CreateAdd(NotB, ConstantInt::get(B->getType(), 1));
    return Builder.CreateAdd(A, NegB);
  }

  Value* obfuscateMulSame(IRBuilder<>& Builder, Value* X) {
    int choice = dist(rng);
    Type* Ty = X->getType();

    switch (choice) {
      case 0: {
        errs() << "x * x → x << 1\n";
        return Builder.CreateShl(X, ConstantInt::get(Ty, 1));
      }
      case 1: {
        errs() << "x * x → x + x\n";
        return Builder.CreateAdd(X, X);
      }
      case 2: {
        errs() << "x * x → x * 2\n";
        return Builder.CreateMul(X, ConstantInt::get(Ty, 2));
      }
    }
    return nullptr;
  }

  Value* obfuscateMulDiff(IRBuilder<>& Builder, Value* A, Value* B) {
    ConstantInt *CB = dyn_cast<ConstantInt>(B);
    if (CB && isPowerOf2_64(CB->getZExtValue())) {
      uint64_t Shift = countTrailingZeros(CB->getZExtValue());
      errs() << "a * " << CB->getZExtValue() << " → a << " << Shift << "\n";
      return Builder.CreateShl(A, ConstantInt::get(A->getType(), Shift));
    }
    return nullptr;
  }

  Value* obfuscateDivSame(IRBuilder<>& Builder, Value* X) {
    int choice = dist(rng);
    Type* Ty = X->getType();

    switch (choice) {
      case 0: {
        errs() << "x / x → 1\n";
        return ConstantInt::get(Ty, 1);
      }
      case 1: {
        errs() << "x / x → x * (1/x)\n";
        Value* One = ConstantInt::get(Ty, 1);
        Value* Inv = Builder.CreateSDiv(One, X);
        return Builder.CreateMul(X, Inv);
      }
      case 2: {
        errs() << "x / x → x - (x - 1)\n";
        Value* Sub = Builder.CreateSub(X, ConstantInt::get(Ty, 1));
        return Builder.CreateSub(X, Sub);
      }
    }
    return nullptr;
  }

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    bool Modified = false;
    IRBuilder<> Builder(F.getContext());

    for (BasicBlock &BB : F) {
      for (auto It = BB.begin(); It != BB.end(); ) {
        Instruction &I = *It;
        ++It;

        if (auto *BinOp = dyn_cast<BinaryOperator>(&I)) {
          Value *LHS = BinOp->getOperand(0);
          Value *RHS = BinOp->getOperand(1);

          if (!LHS->getType()->isIntegerTy()) continue;

          Builder.SetInsertPoint(BinOp);
          Value *NewVal = nullptr;

          if (LHS == RHS || isSameLoadSource(LHS, RHS)) {
            Value *Base = LHS == RHS ? LHS : LHS;
            switch (BinOp->getOpcode()) {
              case Instruction::Add:  NewVal = obfuscateAddSame(Builder, Base); break;
              case Instruction::Sub:  NewVal = obfuscateSubSame(Builder, Base); break;
              case Instruction::Mul:  NewVal = obfuscateMulSame(Builder, Base); break;
              case Instruction::SDiv: NewVal = obfuscateDivSame(Builder, Base); break;
            }
          } else {
            switch (BinOp->getOpcode()) {
              case Instruction::Add:  NewVal = obfuscateAddDiff(Builder, LHS, RHS); break;
              case Instruction::Sub:  NewVal = obfuscateSubDiff(Builder, LHS, RHS); break;
              case Instruction::Mul:  NewVal = obfuscateMulDiff(Builder, LHS, RHS); break;
            }
          }

          if (NewVal) {
            BinOp->replaceAllUsesWith(NewVal);
            BinOp->eraseFromParent();
            Modified = true;
          }
        }
      }
    }

    return Modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }
};

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    "MathObfuscator",
    LLVM_VERSION_STRING,
    [](::llvm::PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](::llvm::StringRef Name,
           ::llvm::FunctionPassManager &FPM,
           ::llvm::ArrayRef<::llvm::PassBuilder::PipelineElement>) -> bool {
          if (Name == "math-obfuscator") {
            FPM.addPass(MathObfuscator());
            return true;
          }
          return false;
        }
      );
    }
  };
}
