cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)
project(flow VERSION 1)
set(CMAKE_MACOSX_RPATH 1)
#set(CMAKE_BUILD_TYPE DEBUG)
#if(NOT CMAKE_BUILD_TYPE)
#    set(CMAKE_BUILD_TYPE RELEASE)
#endif()
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fsanitize=address -fno-omit-frame-pointer")
#set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

set(EXE_SOURCES
        CorrelationTask.cpp
        CorrectionTask.cpp
        main.cpp)

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
        DifferentialCorrection/QAHistogram.cpp 
        DifferentialCorrection/VariableCutBase.cpp
        )

set(BASE_SOURCES
        Base/DataVector.cpp
        Base/DataContainer.cpp
        Base/Axis.cpp
        Base/QVector.cpp
        Base/Profile.cpp
        Base/DataContainerHelper.cpp
        Base/Efficiency.cpp
        Base/Sample.cpp
        Base/EventShape.cpp
        )

string(REPLACE ".cpp" ".h" BASE_HEADERS "${BASE_SOURCES}")
string(REPLACE ".cpp" ".h" DIFF_HEADERS "${DIFF_SOURCES}")
string(REPLACE ".cpp" ".h" CORR_HEADERS "${CORR_SOURCES}")
string(REPLACE ".cpp" ".h" EXE_HEADERS "${EXE_SOURCES}")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Telling CMake where to find the ROOT installation.
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS Core MathCore MathMore RIO Hist Tree Net TreePlayer)

# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
add_definitions(${ROOT_CXX_FLAGS})
set(CMAKE_CXX_STANDARD 14)

# Adding libraries and includes
include_directories(${CMAKE_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})

add_subdirectory(QnCorrections)
add_subdirectory(ReducedEvent)

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()
# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
        ${CMAKE_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)
# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()

ROOT_GENERATE_DICTIONARY(G__FlowBase ${BASE_HEADERS} LINKDEF Base/BaseLinkDef.h)
add_library(FlowBase SHARED ${BASE_SOURCES} G__FlowBase)
target_link_libraries(FlowBase ${ROOT_LIBRARIES} qncorrflow)
add_library(diffflow SHARED ${DIFF_SOURCES})
target_link_libraries(diffflow ${ROOT_LIBRARIES} qncorrflow FlowBase reducedev)
add_library(correlationflow SHARED ${CORR_SOURCES})
target_link_libraries(correlationflow ${ROOT_LIBRARIES} FlowBase)

# Create a main program using the library
add_executable(analysis ${EXE_SOURCES})
target_link_libraries(analysis correlationflow diffflow FlowBase qncorrflow reducedev ${ROOT_LIBRARIES})

install(TARGETS correlationflow diffflow FlowBase  DESTINATION lib)
install(FILES ${CMAKE_BINARY_DIR}/libFlowBase_rdict.pcm DESTINATION lib)
install(FILES ${CMAKE_BINARY_DIR}/libFlowBase.rootmap DESTINATION lib)
install(FILES ${BASE_HEADERS} ${CORR_HEADERS} ${DIFF_HEADERS} ${DIFF_HEADERS} DESTINATION include)
if (APPLE)
    set_property(
            TARGET analysis
            PROPERTY INSTALL_RPATH
            "@loader_path/../lib")
endif ()
if (UNIX AND NOT APPLE)
    set_property(
            TARGET analysis
            PROPERTY INSTALL_RPATH
            "\$ORIGIN/../lib")
endif ()
install(TARGETS analysis DESTINATION bin)
add_subdirectory(test)