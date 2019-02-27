cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)
project(flow VERSION 1)
set(CMAKE_MACOSX_RPATH 1)

include_directories(${gtest_SOURCE_DIR}/include)
set(TEST_SOURCES
        BootstrapSamplerUnitTest.cpp
        SampleUnitTest.cpp
        CorrelationManagerUnitTest.cpp
        DataContainerUnitTest.cpp
        ProductUnitTest.cpp
        StatsUnitTest.cpp
        )
string(REPLACE ".cpp" ".h" TEST_HEADERS "${TEST_SOURCES}")

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests gtest gtest_main ${ROOT_LIBRARIES} Base Correlation QnCorrections Correction)