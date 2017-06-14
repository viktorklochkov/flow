cmake_minimum_required(VERSION 3.7)
project(flow)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp
        Task.cpp
        Task.h
        QnAnalysisVector.cpp
        QnAnalysisVector.h
        LinkDef.h)

set(CMAKE_VERBOSE_MAKEFILE ON)

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
list(APPEND CMAKE_PREFIX_PATH "$ENV{FLOWVECTOR}/include")
message("${CMAKE_PREFIX_PATH}")

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS MathCore RIO Hist Tree Net)

#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

add_definitions(${ROOT_CXX_FLAGS})
find_library(FLOWLIB    NAMES FlowVector HINTS "$ENV{FLOWVECTOR}/lib")
find_path(FLOWINCLUDES QnCorrectionsQnVector.h)
include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS} ${FLOWINCLUDES})


ROOT_GENERATE_DICTIONARY(G__Flow QnAnalysisVector.h LINKDEF LinkDef.h)
#---Create a shared library with generated dictionary
add_library(flowvector SHARED QnAnalysisVector.cpp G__Flow)
target_include_directories(flowvector PUBLIC ${FLOWINCLUDES})
target_link_libraries(flowvector ${ROOT_LIBRARIES} ${FLOWLIB})
#---Create shared library containing task
add_library(flow SHARED Task.cpp)
target_include_directories(flow PUBLIC ${FLOWINCLUDES})
target_link_libraries(flow ${FLOWLIB} ${ROOT_LIBRARIES} flowvector)

#---Create  a main program using the library
add_executable(main main.cpp)
target_link_libraries(main flowvector)
