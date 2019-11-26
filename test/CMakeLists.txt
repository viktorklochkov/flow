cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)
project(flow VERSION 1)
set(CMAKE_MACOSX_RPATH 1)

include_directories(${gtest_SOURCE_DIR}/include)
set(TEST_SOURCES
#        QVectorUnitTest.cpp
#        CorrectionUnitTest.cpp
#        StatisticUnitTest.cpp
#        BootstrapSamplerUnitTest.cpp
#        ReSampleUnitTest.cpp
#        StatsUnitTest.cpp
#        DataFrameAlgorithmUnitTest.cpp
        DataContainerUnitTest.cpp
        )
#        ParticleGeneratorUnitTest.cpp)
string(REPLACE ".cpp" ".h" TEST_HEADERS "${TEST_SOURCES}")

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests gtest gtest_main ${ROOT_LIBRARIES} Base Correction Correlation ToyMC)
add_test(NAME all_tests COMMAND tests)
