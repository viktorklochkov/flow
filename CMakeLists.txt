cmake_minimum_required(VERSION 3.8)
cmake_policy(SET CMP0048 NEW)
project(flow VERSION 1 LANGUAGES CXX)

#set(CMAKE_CXX_STANDARD 14)
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
        QnCorrections/QnCorrectionsCorrectionOnInputData.cpp
        QnCorrections/QnCorrectionsEventClassVariablesSet.cpp
        QnCorrections/QnCorrectionsCorrectionOnQvector.cpp
        QnCorrections/QnCorrectionsHistogram.cpp
        QnCorrections/QnCorrectionsCorrectionStepBase.cpp
        QnCorrections/QnCorrectionsHistogramBase.cpp
        QnCorrections/QnCorrectionsCorrectionsSetOnInputData.cpp
        QnCorrections/QnCorrectionsHistogramChannelized.cpp
        QnCorrections/QnCorrectionsCorrectionsSetOnQvector.cpp
        QnCorrections/QnCorrectionsHistogramChannelizedSparse.cpp
        QnCorrections/QnCorrectionsCutAbove.cpp
        QnCorrections/QnCorrectionsHistogramSparse.cpp
        QnCorrections/QnCorrectionsCutBelow.cpp
        QnCorrections/GainEqualization.cpp
        QnCorrections/QnCorrectionsCutOutside.cpp
        QnCorrections/QnCorrectionsLog.cpp
        QnCorrections/QnCorrectionsCutSetBit.cpp
        QnCorrections/QnCorrectionsManager.cpp
        QnCorrections/QnCorrectionsCutValue.cpp
        QnCorrections/QnCorrectionsProfile.cpp
        QnCorrections/QnCorrectionsCutWithin.cpp
        QnCorrections/QnCorrectionsProfile3DCorrelations.cpp
        QnCorrections/QnCorrectionsCutsBase.cpp
        QnCorrections/QnCorrectionsProfileChannelized.cpp
        QnCorrections/QnCorrectionsCutsSet.cpp
        QnCorrections/QnCorrectionsProfileChannelizedIngress.cpp
        QnCorrections/QnCorrectionsDataVector.cpp
        QnCorrections/QnCorrectionsProfileComponents.cpp
        QnCorrections/QnCorrectionsDataVectorChannelized.cpp
        QnCorrections/QnCorrectionsProfileCorrelationComponents.cpp
        QnCorrections/QnCorrectionsDetector.cpp
        QnCorrections/QnCorrectionsProfileCorrelationComponentsHarmonics.cpp
        QnCorrections/DetectorConfiguration.cpp
        QnCorrections/QnCorrectionsQnVector.cpp
        QnCorrections/DetectorConfigurationChannels.cpp
        QnCorrections/Alignment.cpp
        QnCorrections/QnCorrectionsDetectorConfigurationTracks.cpp
        QnCorrections/QnCorrectionsQnVectorBuild.cpp
        QnCorrections/QnCorrectionsDetectorConfigurationsSet.cpp
        QnCorrections/Recentering.cpp
        QnCorrections/QnCorrectionsEventClassVariable.cpp
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

set(BASE_HEADERS Base/include/DataContainer.h
        Base/include/DataVector.h
        Base/include/Axis.h
        Base/include/DataVector.h
        Base/include/Profile.h
        Base/include/Efficiency.h
        Base/include/QVector.h
        Base/include/Sample.h
        )

set(QNCORR_HEADERS QnCorrections/include/QnCorrectionsCorrectionOnInputData.h
        QnCorrections/include/QnCorrectionsEventClassVariablesSet.h
        QnCorrections/include/QnCorrectionsCorrectionOnQvector.h
        QnCorrections/include/QnCorrectionsHistogram.h
        QnCorrections/include/QnCorrectionsCorrectionStepBase.h
        QnCorrections/include/QnCorrectionsHistogramBase.h
        QnCorrections/include/QnCorrectionsCorrectionsSetOnInputData.h
        QnCorrections/include/QnCorrectionsHistogramChannelized.h
        QnCorrections/include/QnCorrectionsCorrectionsSetOnQvector.h
        QnCorrections/include/QnCorrectionsHistogramChannelizedSparse.h
        QnCorrections/include/QnCorrectionsCutAbove.h
        QnCorrections/include/QnCorrectionsHistogramSparse.h
        QnCorrections/include/QnCorrectionsCutBelow.h
        QnCorrections/include/QnCorrectionsCutOutside.h
        QnCorrections/include/QnCorrectionsCutSetBit.h
        QnCorrections/include/QnCorrectionsManager.h
        QnCorrections/include/QnCorrectionsCutValue.h
        QnCorrections/include/QnCorrectionsProfile.h
        QnCorrections/include/QnCorrectionsCutWithin.h
        QnCorrections/include/QnCorrectionsProfile3DCorrelations.h
        QnCorrections/include/QnCorrectionsCutsBase.h
        QnCorrections/include/QnCorrectionsProfileChannelized.h
        QnCorrections/include/QnCorrectionsCutsSet.h
        QnCorrections/include/QnCorrectionsProfileChannelizedIngress.h
        QnCorrections/include/QnCorrectionsDataVector.h
        QnCorrections/include/QnCorrectionsProfileComponents.h
        QnCorrections/include/QnCorrectionsDataVectorChannelized.h
        QnCorrections/include/QnCorrectionsProfileCorrelationComponents.h
        QnCorrections/include/QnCorrectionsDetector.h
        QnCorrections/include/QnCorrectionsProfileCorrelationComponentsHarmonics.h
        QnCorrections/include/DetectorConfigurationChannels.h
        QnCorrections/include/QnCorrectionsDetectorConfigurationTracks.h
        QnCorrections/include/QnCorrectionsQnVectorBuild.h
        QnCorrections/include/QnCorrectionsDetectorConfigurationsSet.h
        QnCorrections/include/QnCorrectionsEventClassVariable.h
        QnCorrections/include/DetectorConfiguration.h
        QnCorrections/include/GainEqualization.h
        QnCorrections/include/QnCorrectionsLog.h
        QnCorrections/include/QnCorrectionsQnVector.h
        QnCorrections/include/Alignment.h
        QnCorrections/include/Recentering.h
        QnCorrections/include/TwistAndRescale.h
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

include_directories(QnCorrections/include)
ROOT_GENERATE_DICTIONARY(G__QnCorrections ${QNCORR_HEADERS} LINKDEF QnCorrections/QnLinkDef.h)
add_library(QnCorrections SHARED ${QNCORR_SOURCES} G__QnCorrections)
add_library(Qn::QnCorrections ALIAS QnCorrections)
target_compile_definitions(QnCorrections PUBLIC "-DUSE_ROOT")
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

ROOT_GENERATE_DICTIONARY(G__Base ${BASE_HEADERS} DEPENDENCIES QnCorrections LINKDEF Base/BaseLinkDef.h)
add_library(Base SHARED ${BASE_SOURCES} G__Base)
add_library(Qn::Base ALIAS Base)
target_compile_definitions(Base PUBLIC "-DUSE_ROOT")
target_compile_features(Base PUBLIC cxx_std_14)
target_include_directories(Base
        PRIVATE
        ${ROOT_INCLUDE_DIRS}
        QnCorrections/include
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

#add_subdirectory(test)

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
        DESTINATION ${INSTALL_CONFIGDIR}/
        )
#Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/QnConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)
#Install the config, configversion
install(FILES
        ${CMAKE_CURRENT_LIST_DIR}/cmake/QnConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/QnConfigVersion.cmake
        DESTINATION ${INSTALL_CONFIGDIR})

export(TARGETS Base Correction Correlation QnCorrections NAMESPACE Qn:: FILE QnTargets.cmake)
export(PACKAGE Qn)