project(sen2agri-processors)

cmake_minimum_required(VERSION 2.8)

include(CTest)

find_package(OTB REQUIRED)

include(${OTB_USE_FILE})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -flto -mno-avx -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wno-missing-field-initializers")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_definitions(-DVCL_CAN_STATIC_CONST_INIT_FLOAT=0)

add_subdirectory(MACCSMetadata)
add_subdirectory(Composite)
add_subdirectory(CropType)
add_subdirectory(CropMask)
add_subdirectory(VegetationStatus)
add_subdirectory(DEM-WB)
add_subdirectory(CreateFootprint)
add_subdirectory(Common)
add_subdirectory(ContinuousColorMapping)
#add_subdirectory(AppDimensionalityReduction)

install(FILES ../scripts/sen2agri_common.py
              ../scripts/split_shapefile.py
        DESTINATION usr/lib/python2.7/site-packages/)

install(FILES dist/sen2agri-disable-thp.conf
        DESTINATION usr/lib/tmpfiles.d)

install(FILES dist/50-open-files.conf
        DESTINATION etc/security.d/limits.d)
