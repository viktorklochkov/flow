cmake_minimum_required(VERSION 3.7.2)
project(flow)
set(CMAKE_MACOSX_RPATH 1)
#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE debug)
set(SOURCES QnTask.cpp
        DataContainer.cpp
        Axis.cpp
        QnVector.cpp
        EventInfo.cpp)

string(REPLACE ".cpp" ".h" HEADERS "${SOURCES}")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Telling CMake where to find the ROOT installation.
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
list(APPEND CMAKE_PREFIX_PATH "$ENV{FLOWVECTOR}/include")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/reducedevent")

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS Core MathCore RIO Hist Tree Net TreePlayer)

# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
add_definitions(${ROOT_CXX_FLAGS})

# Adding libraries and includes
find_library(FLOWLIB NAMES FlowVector HINTS  "$ENV{FLOWVECTOR}/lib")
find_library(REDUCEDEVENT NAMES ReducedTree)
find_path(FLOWINCLUDES QnCorrectionsQnVector.h)
find_path(REDUCEDINC AliReducedEventInfo.h)
include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS} ${FLOWINCLUDES} ${REDUCEDINC})

# Generate dictionary
ROOT_GENERATE_DICTIONARY(G__Flow ${HEADERS} LINKDEF LinkDef.h)

# Create a shared library with generated dictionary
add_library(flow SHARED ${SOURCES}  G__Flow)
target_include_directories(flow PUBLIC ${CMAKE_SOURCE_DIR} ${FLOWINCLUDES} ${REDUCEDINC})
target_link_libraries(flow ${ROOT_LIBRARIES} ${FLOWLIB} ${REDUCEDEVENT})

# Create a main program using the library
add_executable(main main.cpp)
target_link_libraries(main flow ${ROOT_LIBRARIES})
