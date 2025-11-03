#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

struct MyPass : PassInfoMixin<MyPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    errs() << "Hello from MyPass: " << F.getName() << "\n";
    return PreservedAnalyses::all();
  }
};

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    "MyPass",
    LLVM_VERSION_STRING,
    [](::llvm::PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](::llvm::StringRef Name,
           ::llvm::FunctionPassManager &FPM,
           ::llvm::ArrayRef<::llvm::PassBuilder::PipelineElement>) -> bool {
          if (Name == "my-custom-pass") {
            FPM.addPass(MyPass());
            return true;
          }
          return false;
        }
      );
    }
  };
}
