cmake_minimum_required(VERSION 2.8)
project(flow)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_cpp_STANDARD 14)
set(CMAKE_BUILD_TYPE debug)

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

set(CORR_SOURCES
        Correlation/Correlation.cpp
        Correlation/CorrelationManager.cpp
        Correlation/Sampler.cpp
        Correlation/Correlator.cpp
        Correlation/Correlator.h
        )

set(DIFF_SOURCES
        DifferentialCorrection/EventInfo.cpp
        DifferentialCorrection/VariableManager.cpp
        DifferentialCorrection/CorrectionManager.cpp
        DifferentialCorrection/Detector.cpp
        DifferentialCorrection/Interface/DataFiller.cpp
        DifferentialCorrection/QAHistogram.cpp DifferentialCorrection/QAHistogram.h)

set(BASE_SOURCES
        Base/DataVector.cpp
        Base/DataContainer.cpp
        Base/Axis.cpp
        Base/QVector.cpp
        Base/Profile.cpp
        Base/DataContainerHelper.cpp
        Base/Efficiency.cpp
        Base/Sample.cpp
        Base/Hist.cpp
        Base/Hist.h
        Base/EventShape.cpp
        Base/EventShape.h
        )

string(REPLACE ".cpp" ".h" BASE_HEADERS "${BASE_SOURCES}")
string(REPLACE ".cpp" ".h" DIFF_HEADERS "${DIFF_SOURCES}")
string(REPLACE ".cpp" ".h" CORR_HEADERS "${CORR_SOURCES}")
string(REPLACE ".cpp" ".h" QNCORR_HEADERS "${QNCORR_SOURCES}")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Telling CMake where to find the ROOT installation.
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/lib")

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS Core MathCore MathMore RIO Hist Tree Net TreePlayer)

# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
add_definitions(${ROOT_CXX_FLAGS})

include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})

add_subdirectory(QnCorrections)

add_subdirectory(googletest)

ROOT_GENERATE_DICTIONARY(G__FlowBase ${BASE_HEADERS} LINKDEF Base/BaseLinkDef.h)
add_library(FlowBase SHARED ${BASE_SOURCES} G__FlowBase)
target_link_libraries(FlowBase ${ROOT_LIBRARIES} qncorrflow)
set_target_properties(FlowBase PROPERTIES DEBUG_POSTFIX "")


add_library(diffflow SHARED ${DIFF_SOURCES})
target_link_libraries(diffflow ${ROOT_LIBRARIES} qncorrflow FlowBase)

add_library(correlationflow SHARED ${CORR_SOURCES})
target_link_libraries(correlationflow ${ROOT_LIBRARIES} FlowBase)

add_subdirectory(test)