cmake_minimum_required(VERSION 2.8)
project(flow)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE debug)
set(SOURCES Task.cpp
        DataContainer.cpp
        Axis.cpp
        QnVector.cpp
        EventInfo.cpp
        Correlation.cpp
        CorrectionsInterface.cpp
        DetectorGenerator.cpp DetectorGenerator.h)

string(REPLACE ".cpp" ".h" HEADERS "${SOURCES}")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Telling CMake where to find the ROOT installation.
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/lib")

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS Core MathCore RIO Hist Tree Net TreePlayer)

# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
add_definitions(${ROOT_CXX_FLAGS})

# Adding libraries and includes
find_library(QNCORRECT_LIB NAMES FlowVector)
find_library(REDUCEDEVENT_LIB NAMES ReducedTree)
find_path(QNCORRECT_INC QnCorrectionsQnVector.h HINTS "${CMAKE_SOURCE_DIR}/QnCorrections")
find_path(REDUCED_INC AliReducedEventInfo.h HINTS "${CMAKE_SOURCE_DIR}/ReducedEvent")
include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS} ${QNCORRECT_INC} ${REDUCED_INC})

# Generate dictionary
ROOT_GENERATE_DICTIONARY(G__Flow ${HEADERS} LINKDEF LinkDef.h)
# Create a shared library with generated dictionary
add_library(flow SHARED ${SOURCES}  G__Flow)
target_include_directories(flow PUBLIC ${CMAKE_SOURCE_DIR} ${QNCORRECT_INC} ${REDUCEDINC})
target_link_libraries(flow ${ROOT_LIBRARIES} ${QNCORRECT_LIB} ${REDUCEDEVENT_LIB})

# Create a main program using the library
add_executable(main main.cpp)
target_link_libraries(main flow ${ROOT_LIBRARIES})
