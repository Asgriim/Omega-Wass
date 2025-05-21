cmake_minimum_required(VERSION 3.29)
project(OWasm_vm)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

set(CMAKE_CXX_STANDARD 20)

add_executable(omega-wass main.cpp ${SRC_FILES})
set(CMAKE_CXX_COMPILER /usr/bin/clang++)

add_library(matx SHARED lib/matrix.c)
set_target_properties(matx PROPERTIES
        VERSION 1.0
        SOVERSION 1
)

target_link_libraries(omega-wass matx)

install(TARGETS matx
        DESTINATION lib
)