cmake_minimum_required(VERSION 2.8)
project(flow)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_cpp_STANDARD 14)
#set(CMAKE_BUILD_TYPE debug)

set(SOURCES AliReducedBaseEvent.cpp
        AliReducedCaloClusterInfo.cpp
        AliReducedEventPlaneInfo.cpp
        AliReducedPairInfo.cpp
        AliReducedVarCut.cpp
        AliReducedBaseTrack.cpp
        AliReducedEventCut.cpp
        AliReducedFMDInfo.cpp
        AliReducedTrackCut.cpp
        AliReducedVarManager.cpp
        AliReducedBaseTrackCut.cpp
        AliReducedEventInfo.cpp
        AliReducedInfoCut.cpp
        AliReducedTrackInfo.cpp)

string(REPLACE ".cpp" ".h" HEADERS "${SOURCES}")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Telling CMake where to find the ROOT installation.
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS Core MathCore RIO Hist)

# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
add_definitions(${ROOT_CXX_FLAGS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})

ROOT_GENERATE_DICTIONARY(G__ReducedEv ${HEADERS} LINKDEF ReducedEventLinkDef.h)
add_library(reducedev SHARED ${SOURCES} G__ReducedEv)
target_link_libraries(reducedev ${ROOT_LIBRARIES} )

install(TARGETS reducedev DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libReducedEv.rootmap DESTINATION lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libReducedEv_rdict.pcm DESTINATION lib)
