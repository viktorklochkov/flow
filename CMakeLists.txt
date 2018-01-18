cmake_minimum_required(VERSION 2.8)
project(flow)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE debug)

set(EXE_SOURCES
        SimpleTask.cpp
        TestTask.cpp
        main.cpp)

set(CORR_SOURCES
        Correlation/Correlation.cpp
        Correlation/CorrelationManager.cpp
        )

set(DIFF_SOURCES
        DifferentialCorrection/EventInfo.cpp
        DifferentialCorrection/VariableManager.cpp
        DifferentialCorrection/CorrectionManager.cpp
        DifferentialCorrection/Detector.cpp
        DifferentialCorrection/Interface/DataFiller.cpp)

set(BASE_SOURCES
        Base/DataVector.cpp
        Base/DataContainer.cpp
        Base/Axis.cpp
        Base/QVector.cpp
        Base/Stats.cpp
        Base/DataContainerHelper.cpp)

string(REPLACE ".cpp" ".h" BASE_HEADERS "${BASE_SOURCES}")
string(REPLACE ".cpp" ".h" DIFF_HEADERS "${DIFF_SOURCES}")
string(REPLACE ".cpp" ".h" CORR_HEADERS "${CORR_SOURCES}")
string(REPLACE ".cpp" ".h" EXE_HEADERS "${EXE_SOURCES}")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

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
find_library(REDUCEDEVENT_LIB NAMES ReducedEvent)
find_path(BASE_INC DataContainer.h HINTS "${CMAKE_SOURCE_DIR}/Base")
find_path(CORR_INC Correlation.h HINTS "${CMAKE_SOURCE_DIR}/Correlation")
find_path(DIFF_INC CorrectionManager.h HINTS "${CMAKE_SOURCE_DIR}/DifferentialCorrection")
find_path(QNCORRECT_INC QnCorrectionsQnVector.h HINTS "${CMAKE_SOURCE_DIR}/QnCorrections")
find_path(REDUCED_INC AliReducedEventInfo.h HINTS "${CMAKE_SOURCE_DIR}/ReducedEvent")
include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS} ${QNCORRECT_INC} ${REDUCED_INC} ${BASE_INC} ${CORR_INC} ${DIFF_INC})

ROOT_GENERATE_DICTIONARY(G__BaseFlow ${BASE_HEADERS} LINKDEF Base/BaseLinkDef.h)
add_library(baseflow SHARED ${BASE_SOURCES} G__BaseFlow)
target_link_libraries(baseflow ${ROOT_LIBRARIES} ${QNCORRECT_LIB})

add_library(diffflow SHARED ${DIFF_SOURCES})
target_link_libraries(diffflow ${ROOT_LIBRARIES} ${QNCORRECT_LIB} baseflow)

add_library(correlationflow SHARED ${CORR_SOURCES})
target_link_libraries(correlationflow ${ROOT_LIBRARIES} baseflow)

## Generate dictionary
#ROOT_GENERATE_DICTIONARY(G__Flow ${HEADERS} LINKDEF LinkDef.h)
## Create a shared library with generated dictionary
#add_library(flow SHARED ${SOURCES} G__Flow)
#target_include_directories(flow PUBLIC ${CMAKE_SOURCE_DIR} ${QNCORRECT_INC} ${REDUCEDINC})
#target_link_libraries(flow ${ROOT_LIBRARIES} ${QNCORRECT_LIB} ${REDUCEDEVENT_LIB})
#
# Create a main program using the library
add_executable(main ${EXE_SOURCES})
target_link_libraries(main correlationflow diffflow baseflow ${ROOT_LIBRARIES} ${QNCORRECT_LIB} ${REDUCEDEVENT_LIB})
target_include_directories(main PUBLIC ${EXE_HEADERS} ${QNCORRECT_INC} ${REDUCED_INC} )
