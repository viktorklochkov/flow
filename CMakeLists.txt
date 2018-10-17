cmake_minimum_required(VERSION 3.8)
cmake_policy(SET CMP0048 NEW)
project(flow VERSION 1)
set(CMAKE_MACOSX_RPATH 1)

set(CMAKE_CXX_STANDARD 14)

# By default build optimized code with debug symbols.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RELWITHDEBINFO)
endif()
set(CMAKE_ALLOWED_BUILD_TYPES DEBUG RELEASE RELWITHDEBINFO)
if(NOT CMAKE_BUILD_TYPE IN_LIST CMAKE_ALLOWED_BUILD_TYPES)
    message(FATAL_ERROR "Invalid build type ${CMAKE_BUILD_TYPE}. Use one of: ${CMAKE_ALLOWED_BUILD_TYPES}.")
endif()
# Set default flags for the different build types.
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -DDEBUG -D__DEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -ftree-vectorize -ffast-math -DNODEBUG")
message(STATUS "Using CXX flags for ${CMAKE_BUILD_TYPE}: ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")


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
        DifferentialCorrection/VariableManager.cpp
        DifferentialCorrection/CorrectionManager.cpp
        DifferentialCorrection/Interface/DataFiller.cpp
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

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

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

# ROOT
find_package(ROOT REQUIRED COMPONENTS Core MathCore RIO Hist Tree Net TreePlayer)
include(${ROOT_USE_FILE})
if(ROOT_FOUND)
    message(STATUS "Using ROOT: ${ROOT_VERSION} <${ROOT_CONFIG}>")

    add_subdirectory(QnCorrections)

    ROOT_GENERATE_DICTIONARY(G__FlowBase ${BASE_HEADERS} ${QNCORR_HEADERS} LINKDEF Base/BaseLinkDef.h)
    add_library(FlowBase SHARED ${BASE_SOURCES} G__FlowBase)
    target_compile_definitions(FlowBase PUBLIC "-DUSE_ROOT")
    target_include_directories(FlowBase PRIVATE ${ROOT_INCLUDE_DIRS})
    target_include_directories(FlowBase PUBLIC Base)
    target_link_libraries(FlowBase QnCorrections ${ROOT_LIBRARIES})

    add_library(DifferentialCorrection SHARED ${DIFF_SOURCES})
    target_compile_definitions(DifferentialCorrection PUBLIC "-DUSE_ROOT")
    target_include_directories(DifferentialCorrection PRIVATE ${ROOT_INCLUDE_DIRS})
    target_include_directories(DifferentialCorrection PUBLIC DifferentialCorrection DifferentialCorrection/Interface)
    target_link_libraries(DifferentialCorrection QnCorrections FlowBase ${ROOT_LIBRARIES})

    add_library(Correlation SHARED ${CORR_SOURCES})
    target_compile_definitions(Correlation PUBLIC "-DUSE_ROOT")
    target_include_directories(Correlation PRIVATE ${ROOT_INCLUDE_DIRS})
    target_include_directories(Correlation PUBLIC Correlation)
    target_link_libraries(Correlation FlowBase ${ROOT_LIBRARIES})

    add_subdirectory(test)

endif()