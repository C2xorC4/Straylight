cmake_minimum_required(VERSION 3.20)
project(MathRandomizer LANGUAGES CXX)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17)

add_library(MathRandomizer SHARED MathRandomizer.cpp)

# LINK TO libLLVM.so (monolithic)
target_link_libraries(MathRandomizer PRIVATE
  ${LLVM_LIBRARY_DIRS}/libLLVM.so
)

set_target_properties(MathRandomizer PROPERTIES
  PREFIX ""
  SUFFIX ".so"
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
