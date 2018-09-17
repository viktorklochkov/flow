cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)
project(flow VERSION 1)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_cpp_STANDARD 14)
set(CMAKE_BUILD_TYPE debug)

set(QNCORR_SOURCES
        QnCorrectionsCorrectionOnInputData.cpp
        QnCorrectionsEventClassVariablesSet.cpp
        QnCorrectionsCorrectionOnQvector.cpp
        QnCorrectionsHistogram.cpp
        QnCorrectionsCorrectionStepBase.cpp
        QnCorrectionsHistogramBase.cpp
        QnCorrectionsCorrectionsSetOnInputData.cpp
        QnCorrectionsHistogramChannelized.cpp
        QnCorrectionsCorrectionsSetOnQvector.cpp
        QnCorrectionsHistogramChannelizedSparse.cpp
        QnCorrectionsCutAbove.cpp
        QnCorrectionsHistogramSparse.cpp
        QnCorrectionsCutBelow.cpp
        QnCorrectionsInputGainEqualization.cpp
        QnCorrectionsCutOutside.cpp
        QnCorrectionsLog.cpp
        QnCorrectionsCutSetBit.cpp
        QnCorrectionsManager.cpp
        QnCorrectionsCutValue.cpp
        QnCorrectionsProfile.cpp
        QnCorrectionsCutWithin.cpp
        QnCorrectionsProfile3DCorrelations.cpp
        QnCorrectionsCutsBase.cpp
        QnCorrectionsProfileChannelized.cpp
        QnCorrectionsCutsSet.cpp
        QnCorrectionsProfileChannelizedIngress.cpp
        QnCorrectionsDataVector.cpp
        QnCorrectionsProfileComponents.cpp
        QnCorrectionsDataVectorChannelized.cpp
        QnCorrectionsProfileCorrelationComponents.cpp
        QnCorrectionsDetector.cpp
        QnCorrectionsProfileCorrelationComponentsHarmonics.cpp
        QnCorrectionsDetectorConfigurationBase.cpp
        QnCorrectionsQnVector.cpp
        QnCorrectionsDetectorConfigurationChannels.cpp
        QnCorrectionsQnVectorAlignment.cpp
        QnCorrectionsDetectorConfigurationTracks.cpp
        QnCorrectionsQnVectorBuild.cpp
        QnCorrectionsDetectorConfigurationsSet.cpp
        QnCorrectionsQnVectorRecentering.cpp
        QnCorrectionsEventClassVariable.cpp
        QnCorrectionsQnVectorTwistAndRescale.cpp)

string(REPLACE ".cpp" ".h" QNCORR_HEADERS "${QNCORR_SOURCES}")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Telling CMake where to find the ROOT installation.
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS Core MathCore RIO Hist Tree Net TreePlayer)

# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
add_definitions(${ROOT_CXX_FLAGS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})

ROOT_GENERATE_DICTIONARY(G__QnCorrFlow ${QNCORR_HEADERS} LINKDEF QnLinkDef.h)
add_library(qncorrflow SHARED ${QNCORR_SOURCES} G__QnCorrFlow)
set_target_properties(qncorrflow PROPERTIES DEBUG_POSTFIX "")
target_link_libraries(qncorrflow ${ROOT_LIBRARIES})
