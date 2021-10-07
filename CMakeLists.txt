project(pattern)
cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

include(Paths)
include(Compiler)


###########################################################################################
# TARGETS
###########################################################################################
add_executable(pimpl examples/pimpl/main.cc)
add_executable(reflect examples/reflect/main.cc)
add_executable(self-registering-factory examples/self-registering-factory/main.cc)
