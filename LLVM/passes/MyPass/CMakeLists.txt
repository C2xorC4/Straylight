cmake_minimum_required(VERSION 3.20)
project(MyPass LANGUAGES CXX)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

# Include dirs & definitions
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build plugin
add_library(MyPass SHARED MyPass.cpp)

# LINK TO MONOLITHIC libLLVM.so
target_link_libraries(MyPass PRIVATE
  ${LLVM_LIBRARY_DIRS}/libLLVM.so
)

# Plugin naming
set_target_properties(MyPass PROPERTIES
  PREFIX ""
  SUFFIX ".so"
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
