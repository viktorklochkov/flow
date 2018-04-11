cmake_minimum_required(VERSION 2.8)
project(flow)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_cpp_STANDARD 14)
set(CMAKE_BUILD_TYPE debug)

include_directories(${gtest_SOURCE_DIR}/include)
set(TEST_SOURCES DataContainerUnitTest.cpp
        CorrelationUnitTest.cpp
        ProfileUnitTest.cpp
        CorrelationManagerUnitTest.cpp
        EfficiencyUnitTest.cpp BootstrapSamplerUnitTest.cpp ResultContainerUnitTest.cpp)
string(REPLACE ".cpp" ".h" TEST_HEADERS "${TEST_SOURCES}")

#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS Core MathCore RIO Hist Tree Net TreePlayer)

# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
add_definitions(${ROOT_CXX_FLAGS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS})

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests gtest_main ${ROOT_LIBRARIES} FlowBase correlationflow)