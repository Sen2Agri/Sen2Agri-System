# =========================================================================
# Program:   pĥenotb
# Language:  C++
#
# Copyright (c) CESBIO. All rights reserved.
#
# See pĥenotb-copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.  See the above copyright notices for more information.
#
# =========================================================================

project(phenotb)

cmake_minimum_required(VERSION 2.8)

find_package(OTB)
if(OTB_FOUND)
  include(${OTB_USE_FILE})
else(OTB_FOUND)
  message(FATAL_ERROR
          "OTB not found. Please set OTB_DIR")
endif(OTB_FOUND)

if(NOT PHENOTB_INSTALL_BIN_DIR)
  set(PHENOTB_INSTALL_BIN_DIR "bin")
endif()

## gapfilling
find_path(GAPFILLING_INCLUDE_DIR gapfilling.h)
message(STATUS "  GAPFILLING includes: ${GAPFILLING_INCLUDE_DIR}")
include_directories(${GAPFILLING_INCLUDE_DIR})

option(BUILD_SHARED_LIBS "Build ITK with shared libraries." ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Library)

## compiler flags
include(OTBSetStandardCompilerFlags.cmake)

include(${CMAKE_ROOT}/Modules/Dart.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(Library)
add_subdirectory(Testing)
add_subdirectory(Applications)
