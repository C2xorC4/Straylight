Build llvm using 
```bash
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_ENABLE_PLUGINS=ON -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_LINK_LLVM_DYLIB=ON -DLLVM_BUILD_LLVM_DYLIB=ON ../llvm
ninja install
```

/MyPass contains a basic template pass and is intended as a brief example of how to build a LLVM pass utilizing 
the modular source build. It is just a starting point to understand the build and linking requirements. 
See the included .sh for build command sequence.
