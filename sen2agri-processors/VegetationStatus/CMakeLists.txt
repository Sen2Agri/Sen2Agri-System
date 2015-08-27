cmake_minimum_required(VERSION 2.8)

set(GAPFILLING_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/temporalgapfilling/src/library)
set(PHENOTB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/phenotb/src/Library)
set(PHENOTB_LIBRARY OTBPheno)

add_subdirectory(otb-bv/src)
add_subdirectory(phenotb/src)
add_subdirectory(temporalgapfilling/src)
