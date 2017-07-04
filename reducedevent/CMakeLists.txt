cmake_minimum_required(VERSION 2.8)


project(REDUCEDTREE)
set(CMAKE_MACOSX_RPATH 1)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

find_package(ROOT REQUIRED COMPONENTS Core MathCore RIO Hist Tree Net)

include(${ROOT_USE_FILE})

include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})
add_definitions(${ROOT_CXX_FLAGS})


set (SOURCES
  AliReducedBaseEvent.cxx
  AliReducedCaloClusterInfo.cxx
  AliReducedEventPlaneInfo.cxx
  AliReducedPairInfo.cxx
  AliReducedVarCut.cxx
  AliReducedBaseTrack.cxx
  AliReducedEventCut.cxx
  AliReducedFMDInfo.cxx
  AliReducedTrackCut.cxx
  AliReducedVarManager.cxx
  AliReducedBaseTrackCut.cxx
  AliReducedEventInfo.cxx
  AliReducedInfoCut.cxx
  AliReducedTrackInfo.cxx
)

string(REPLACE ".cxx" ".h" HEADERS "${SOURCES}")

ROOT_GENERATE_DICTIONARY(G__ReducedTree ${HEADERS} LINKDEF RedLinkDef.h)
message("${ROOT_LIBRARIES}")

#---Create a shared library with generated dictionary
add_library(ReducedTree SHARED ${SOURCES} G__ReducedTree)
target_link_libraries(ReducedTree ${ROOT_LIBRARIES})

