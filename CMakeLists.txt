cmake_minimum_required(VERSION 3.13.0)
project(dyninst-instrument LANGUAGES CXX)

find_package(Dyninst 12.3.0 REQUIRED COMPONENTS dyninstAPI dynC_API parseAPI symtabAPI patchAPI)

add_executable(dyninst-instrument dyninst-instrument.cpp)
target_link_libraries(dyninst-instrument Dyninst::dyninstAPI)
