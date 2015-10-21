project(sen2agri-processors)

cmake_minimum_required(VERSION 2.8)

include(CTest)

find_package(OTB REQUIRED)

include(${OTB_USE_FILE})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -march=native")

add_definitions(-DVCL_CAN_STATIC_CONST_INIT_FLOAT=0)

add_subdirectory(MACCSMetadata)
add_subdirectory(Composite)
add_subdirectory(CropType)
add_subdirectory(CropMask)
add_subdirectory(VegetationStatus)
add_subdirectory(DEM-WB)
add_subdirectory(CreateFootprint)
