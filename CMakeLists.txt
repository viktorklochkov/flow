cmake_minimum_required(VERSION 3.8)
cmake_policy(SET CMP0048 NEW)
project(flow VERSION 1.5 LANGUAGES CXX)

#set(CMAKE_CXX_STANDARD 11)
if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
endif (APPLE)

add_compile_options("-Wall" "-Wextra"
                    "$<$<CONFIG:RELEASE>:-O3;-march=native;-ftree-vectorize;-ffast-math>"
                    "$<$<CONFIG:DEBUG>:-O0;-O0>")
if ("${CMAKE_COMPILER_ID}" STREQUAL  "Clang")
    add_compile_options("-stdlib=libc++")
endif()

set(QNCORR_SOURCES
        QnCorrections/CorrectionOnInputData.cpp
        QnCorrections/EventClassVariablesSet.cpp
        QnCorrections/CorrectionOnQvector.cpp
        QnCorrections/CorrectionHistogram.cpp
        QnCorrections/CorrectionStepBase.cpp
        QnCorrections/CorrectionHistogramBase.cpp
        QnCorrections/CorrectionsSetOnInputData.cpp
        QnCorrections/CorrectionHistogramChannelized.cpp
        QnCorrections/CorrectionsSetOnQvector.cpp
        QnCorrections/CorrectionHistogramChannelizedSparse.cpp
        QnCorrections/CutAbove.cpp
        QnCorrections/CorrectionHistogramSparse.cpp
        QnCorrections/CutBelow.cpp
        QnCorrections/GainEqualization.cpp
        QnCorrections/CutOutside.cpp
        QnCorrections/CorrectionLog.cpp
        QnCorrections/CutSetBit.cpp
        QnCorrections/CorrectionCalculator.cpp
        QnCorrections/CutValue.cpp
        QnCorrections/CorrectionProfile.cpp
        QnCorrections/CutWithin.cpp
        QnCorrections/CorrectionProfile3DCorrelations.cpp
        QnCorrections/CutsBase.cpp
        QnCorrections/CorrectionProfileChannelized.cpp
        QnCorrections/CutsSet.cpp
        QnCorrections/CorrectionProfileChannelizedIngress.cpp
        QnCorrections/CorrectionDataVector.cpp
        QnCorrections/CorrectionProfileComponents.cpp
        QnCorrections/CorrectionDataVectorChannelized.cpp
        QnCorrections/CorrectionProfileCorrelationComponents.cpp
        QnCorrections/CorrectionDetector.cpp
        QnCorrections/CorrectionProfileCorrelationComponentsHarmonics.cpp
        QnCorrections/DetectorConfiguration.cpp
        QnCorrections/CorrectionQnVector.cpp
        QnCorrections/DetectorConfigurationChannels.cpp
        QnCorrections/Alignment.cpp
        QnCorrections/DetectorConfigurationTracks.cpp
        QnCorrections/CorrectionQnVectorBuild.cpp
        QnCorrections/CorrectionDetectorConfigurationsSet.cpp
        QnCorrections/Recentering.cpp
        QnCorrections/EventClassVariable.cpp
        QnCorrections/TwistAndRescale.cpp
        )

set(CORR_SOURCES
        Correlation/Correlation.cpp
        Correlation/CorrelationManager.cpp
        Correlation/Sampler.cpp
        Correlation/Correlator.cpp
        )

set(DIFF_SOURCES
        Correction/CorrectionManager.cpp
        )

set(BASE_SOURCES
        Base/Axis.cpp
        Base/QVector.cpp
        Base/Profile.cpp
        Base/DataContainerHelper.cpp
        Base/Efficiency.cpp
        Base/Sample.cpp
        Base/EventShape.cpp
        )

set(BASE_HEADERS DataContainer.h
        DataVector.h
        DataContainerHelper.h
        Axis.h
        Profile.h
        Efficiency.h
        QVector.h
        Sample.h
        )

set(QNCORR_HEADERS CorrectionOnInputData.h
        EventClassVariablesSet.h
        CorrectionOnQvector.h
        CorrectionHistogram.h
        CorrectionStepBase.h
        CorrectionHistogramBase.h
        CorrectionsSetOnInputData.h
        CorrectionHistogramChannelized.h
        CorrectionsSetOnQvector.h
        CorrectionHistogramChannelizedSparse.h
        CutAbove.h
        CorrectionHistogramSparse.h
        CutBelow.h
        CutOutside.h
        CutSetBit.h
        CorrectionCalculator.h
        CutValue.h
        CorrectionProfile.h
        CutWithin.h
        CorrectionProfile3DCorrelations.h
        CutsBase.h
        CorrectionProfileChannelized.h
        CutsSet.h
        CorrectionProfileChannelizedIngress.h
        CorrectionDataVector.h
        CorrectionProfileComponents.h
        CorrectionDataVectorChannelized.h
        CorrectionProfileCorrelationComponents.h
        CorrectionDetector.h
        CorrectionProfileCorrelationComponentsHarmonics.h
        DetectorConfigurationChannels.h
        DetectorConfigurationTracks.h
        CorrectionQnVectorBuild.h
        DetectorConfigurationsSet.h
        EventClassVariable.h
        DetectorConfiguration.h
        GainEqualization.h
        CorrectionLog.h
        CorrectionQnVector.h
        Alignment.h
        Recentering.h
        TwistAndRescale.h
        )

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Googletest
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif ()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
if (result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif ()
# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src ${CMAKE_BINARY_DIR}/googletest-build EXCLUDE_FROM_ALL)

# ROOT
find_package(ROOT REQUIRED COMPONENTS Core MathCore RIO Hist Tree Net TreePlayer)
include(${ROOT_USE_FILE})

message(STATUS "Using ROOT: ${ROOT_VERSION} <${ROOT_CONFIG}>")

include_directories(QnCorrections Base QnCorrections/include Base/include)
ROOT_GENERATE_DICTIONARY(G__QnCorrections ${QNCORR_HEADERS} LINKDEF QnCorrections/QnLinkDef.h OPTIONS "-noIncludePaths")
add_library(QnCorrections SHARED ${QNCORR_SOURCES} G__QnCorrections)
add_library(Qn::QnCorrections ALIAS QnCorrections)
target_compile_definitions(QnCorrections PUBLIC "-DUSE_ROOT")
#target_compile_features(QnCorrections PUBLIC cxx_std_11)
target_include_directories(QnCorrections
        PRIVATE
        ${ROOT_INCLUDE_DIRS}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/QnCorrections/include>
        )
target_link_libraries(QnCorrections PRIVATE ${ROOT_LIBRARIES})
#add_custom_command(TARGET QnCorrections PRE_BUILD COMMAND ${CMAKE_COMMAND}
#        -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/QnCorrections/include ${CMAKE_BINARY_DIR}/include)

ROOT_GENERATE_DICTIONARY(G__Base ${BASE_HEADERS} DEPENDENCIES QnCorrections LINKDEF Base/BaseLinkDef.h OPTIONS "-noIncludePaths")
add_library(Base SHARED ${BASE_SOURCES} G__Base)
add_library(Qn::Base ALIAS Base)
target_compile_definitions(Base PUBLIC "-DUSE_ROOT")
#target_compile_features(Base PUBLIC cxx_std_11)
target_include_directories(Base
        PRIVATE
        ${ROOT_INCLUDE_DIRS}
        QnCorrections/include
        ${CMAKE_CURRENT_BINARY_DIR}/common
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Base/include>
        )
target_link_libraries(Base PUBLIC QnCorrections PRIVATE ${ROOT_LIBRARIES})

add_library(Correction SHARED ${DIFF_SOURCES})
add_library(Qn::Correction ALIAS Correction)
target_compile_definitions(Correction PUBLIC "-DUSE_ROOT")
target_include_directories(Correction
        PRIVATE
        ${ROOT_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/Correction
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Correction/include>

        )
target_link_libraries(Correction PUBLIC Base PRIVATE ${ROOT_LIBRARIES})

add_library(Correlation SHARED ${CORR_SOURCES})
add_library(Qn::Correlation ALIAS Correlation)
target_compile_definitions(Correlation PUBLIC "-DUSE_ROOT")
target_include_directories(Correlation
        PRIVATE
        ${ROOT_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/Correlation
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Correlation/include>
        )
target_link_libraries(Correlation PUBLIC Base PRIVATE ${ROOT_LIBRARIES})

add_subdirectory(test)

# Install configuration

install(DIRECTORY QnCorrections/include/ Base/include/ Correction/include/ Correlation/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# Installion of ROOT dictionaries
install(FILES ${CMAKE_BINARY_DIR}/libBase_rdict.pcm DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_BINARY_DIR}/libBase.rootmap DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_BINARY_DIR}/libQnCorrections_rdict.pcm DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_BINARY_DIR}/libQnCorrections.rootmap DESTINATION ${CMAKE_INSTALL_LIBDIR})



install(TARGETS Base Correction Correlation QnCorrections
        EXPORT QnTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

set(INSTALL_CONFIGDIR lib/cmake/Qn)
install(EXPORT QnTargets
        FILE QnTargets.cmake
        NAMESPACE Qn::
        DESTINATION ${INSTALL_CONFIGDIR}
        )
#Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/QnConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/QnConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/lib/cmake/QnConfig.cmake
        INSTALL_DESTINATION ${INSTALL_CONFIGDIR})
#Install the config, configversion
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/lib/cmake/QnConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/QnConfigVersion.cmake
        DESTINATION ${INSTALL_CONFIGDIR})

export(TARGETS Base Correction Correlation QnCorrections NAMESPACE Qn:: FILE QnTargets.cmake)
export(PACKAGE Qn)