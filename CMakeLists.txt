cmake_minimum_required(VERSION 3.29)
project(OWasm_vm)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

set(CMAKE_CXX_STANDARD 20)

add_executable(OWasm_vm main.cpp ${SRC_FILES})
