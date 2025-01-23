cmake_minimum_required(VERSION 3.15)
project(pattern)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

include(Paths)
include(Compiler)
include(Patterns)


###########################################################################################
# TARGETS
###########################################################################################
add_executable(pimpl examples/pimpl/main.cc)
add_executable(reflect examples/reflect/main.cc)
add_executable(reflect-init examples/reflect-init/main.cc)
add_executable(visitor examples/visitor/main.cc)
add_executable(visitor-generic examples/visitor-generic/main.cc)
add_executable(reflect-inheritance examples/reflect-inheritance/main.cc)
add_executable(self-registering-factory examples/self-registering-factory/main.cc)
add_executable(xml-reflect-inheritance examples/xml-reflect-inheritance/main.cc)
add_executable(pimpl-reflect-inheritance examples/pimpl-reflect-inheritance/main.cc)
add_executable(pimpl-reflect-inheritance-multilevel examples/pimpl-reflect-inheritance-multilevel/main.cc)
target_compile_definitions(pimpl-reflect-inheritance-multilevel PUBLIC PATTERN_SHOW_SELF_REGISTERING)
add_executable(pimpl-reflect-inheritance-init examples/pimpl-reflect-inheritance-init/main.cc)
add_executable(visitor-reflectable examples/visitor-reflectable/main.cc)
add_executable(visitor-pimpl examples/visitor-pimpl/main.cc)
add_executable(visitor-generic-pimpl examples/visitor-generic-pimpl/main.cc)
add_executable(commandline examples/commandline/main.cc)


add_plugin(self-registering-plugin-circle examples/self-registering-plugin/circle.cc)
add_executable(self-registering-plugin examples/self-registering-plugin/main.cc)

