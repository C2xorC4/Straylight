# Straylight

### Requirements
LLVM v21.1.x

### LLVM-Project Source Build

Source build of LLVM-Project inside a *nix-based system. This can be done via docker, WSL, MSYS, etc. Inside your chosen system you will need to clone the llvm-project repository:
```bash
git clone --depth 1 https://github.com/llvm/llvm-project.git
```

Next, you will need to prep for a source build. The absolute minimal build target is clang:
```bash
cmake -G Ninja \
-DCMAKE_BUILD_TYPE=Release \
-DLLVM_ENABLE_PROJECTS="clang" \
-DLLVM_ENABLE_PLUGINS=ON \
-DLLVM_LINK_LLVM_DYLIB=ON \
-DLLVM_BUILD_LLVM_DYLIB=ON \
../llvm
```

or you can include additional content for future development:
```bash
cmake -G Ninja \
-DCMAKE_BUILD_TYPE=Release \
-DLLVM_ENABLE_PROJECTS="clang;lld;lldb;compiler-rt" \
-DLLVM_ENABLE_PLUGINS=ON \
-DLLVM_LINK_LLVM_DYLIB=ON \
-DLLVM_BUILD_LLVM_DYLIB=ON \
../llvm
```

Unfortunately during initial testing, there were quite a few issues when attempting to link passes to individual libraries, so building llvm in monolithic seems to be the way to go.

If this is being executed within a linux subsystem such as WSL, etc, you will likely want to cap the number of threads with `-j12` or similar. If you find that your terminal or build processes are being closed, or otherwise terminated, you're likely hitting a memory ceiling and need to lower your synchronous jobs count. For my WSL 2 Kali installation on a workstation with 32GB, too much higher than `-j12` appears to hit that cap and prematurely terminate the build.

```bash
ninja -j12
```

Once your build completes, install it:
```bash
sudo ninja install
```
