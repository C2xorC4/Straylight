#!/bin/bash
cd build && rm -rf * && cmake -DLLVM_DIR=/usr/local/lib/cmake/llvm .. && make -j

ldd MyPass.so | grep LLVM  # Should show multiple LLVM*.so
mkdir build
cd build

#echo 'From /opt/source/llvm-project:'
#echo 'void functionName() {} void functionName2() {} ' | /opt/source/llvm-project/build/bin/clang -x c - -emit-llvm -S -O2 -o test.ll
#/opt/source/llvm-project/build/bin/opt -load-pass-plugin /opt/source/llvm-passes/MyPass/build/MyPass.so -passes="my-custom-pass" t.ll -o optimized.ll

echo 'From installed bins:'
echo 'void functionName() {} void functionName2() {} ' | clang -x c - -emit-llvm -S -O2 -o test.ll
opt -load-pass-plugin /opt/source/llvm-passes/MyPass/build/MyPass.so -passes="my-custom-pass" test.ll -o optimized.ll
