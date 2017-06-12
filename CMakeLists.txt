cmake_minimum_required(VERSION 3.7)
project(flow)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(flow ${SOURCE_FILES})

#list(APPEND CMAKE_PREFIX_PATH ${ROOTSYS})

#find_package(ROOT REQUIRED COMPONENTS RIO Net)

#include(${ROOT_USE_FILE})