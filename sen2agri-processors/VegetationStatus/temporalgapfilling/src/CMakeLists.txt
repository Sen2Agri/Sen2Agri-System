# =========================================================================
# Program:   gapfilling
# Language:  C++
#
# Copyright (c) CESBIO. All rights reserved.
#
# See gapfilling-copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.  See the above copyright notices for more information.
#
# =========================================================================

project(GAPFILLING)

cmake_minimum_required(VERSION 2.8)

find_package(OTB)
if(OTB_FOUND)
  include(${OTB_USE_FILE})
else(OTB_FOUND)
  message(FATAL_ERROR
          "OTB not found. Please set OTB_DIR")
endif(OTB_FOUND)

if(NOT GAPFILLING_INSTALL_BIN_DIR)
  set(GAPFILLING_INSTALL_BIN_DIR "bin")
endif()
set(GAPFILLING_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/library PARENT_SCOPE)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/library)

option(BUILD_SHARED_LIBS "Build ITK with shared libraries." ON)

## phenotb
find_path(PHENOTB_INCLUDE_DIR phenoTypes.h)
find_library(PHENOTB_LIBRARY NAMES libOTBPheno)
message(STATUS "  PHENOTB includes: ${PHENOTB_INCLUDE_DIR}")
message(STATUS "  PHENOTB library: ${PHENOTB_LIBRARY}")
include_directories(${PHENOTB_INCLUDE_DIR})

## compiler flags
#include(${OTB_SOURCE_DIR}/CMake/OTBSetStandardCompilerFlags.cmake)
include(OTBSetStandardCompilerFlags.cmake)

## tests
include(${CMAKE_ROOT}/Modules/Dart.cmake)
set(TEMP ${GAPFILLING_BINARY_DIR}/Testing/Temporary)

## subdirs
add_subdirectory(library)
add_subdirectory(testing)
add_subdirectory(applications)
