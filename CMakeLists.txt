cmake_minimum_required(VERSION 3.20.0)
project(dyninst-instrument LANGUAGES CXX)

find_package(Dyninst REQUIRED COMPONENTS dyninstAPI dynC_API parseAPI symtabAPI patchAPI)

set(CMAKE_MODULE_PATH
  "${Dyninst_DIR}"
  "${Dyninst_DIR}/Modules"
  ${CMAKE_MODULE_PATH}
)

# Import the Dyninst components
find_package(Dyninst REQUIRED COMPONENTS dyninstAPI dynC_API parseAPI symtabAPI patchAPI)
# Read the cache generated from building Dyninst
load_cache(${Dyninst_DIR}
  # READ_WITH_PREFIX DYNINST_
  Boost_LIBRARIES
  Boost_INCLUDE_DIRS
  Boost_LIBRARY_DIRS
  Boost_DEFINES
  TBB_INCLUDE_DIRS
)

add_executable(demo-instrument demo-instrument.cpp)

target_include_directories(demo-instrument PRIVATE
  ${DYNINST_INCLUDE_DIR}
  ${DYNINST_Boost_INCLUDE_DIRS}
  ${DYNINST_TBB_INCLUDE_DIRS}
)

target_link_libraries(demo-instrument PRIVATE
  Dyninst::symtabAPI
  Dyninst::dyninstAPI
)

target_compile_definitions(demo-instrument PRIVATE ${DYNINST_Boost_DEFINITIONS})
target_link_directories(demo-instrument PRIVATE ${DYNINST_Boost_LIBRARY_DIRS})


add_executable(dyninst-instrument dyninst-instrument.cpp)
target_link_libraries(dyninst-instrument Dyninst::dyninstAPI)


add_executable(endpgm-instrument endpgm-instrument.cpp)

target_include_directories(endpgm-instrument PRIVATE
  ${DYNINST_INCLUDE_DIR}
  ${DYNINST_Boost_INCLUDE_DIRS}
  ${DYNINST_TBB_INCLUDE_DIRS}
)

target_link_libraries(endpgm-instrument PRIVATE
  Dyninst::symtabAPI
  Dyninst::dyninstAPI
)
