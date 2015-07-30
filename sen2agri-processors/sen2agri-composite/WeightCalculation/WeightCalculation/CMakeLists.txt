project(WeightCalculation)

cmake_minimum_required(VERSION 2.8)

#aux_source_directory(. SRC_LIST)

FIND_PACKAGE(OTB REQUIRED)

INCLUDE(${OTB_USE_FILE})

otb_create_application(
  NAME           WeightAOT
  SOURCES        weightonaot.cpp weightaotapp.cpp 
  LINK_LIBRARIES ${OTB_LIBRARIES})

otb_create_application(
  NAME           WeightOnClouds
  SOURCES        cloudmaskbinarization.cpp cloudsinterpolation.cpp cloudweightcomputation.cpp gaussianfilter.cpp weightoncloudsapp.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES})

otb_create_application(
  NAME           TotalWeight
  SOURCES        totalweightcomputation.cpp totalweightapp.cpp 
  LINK_LIBRARIES ${OTB_LIBRARIES})
  

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

#add_executable(WeightCalculationLauncher main.cpp cloudsinterpolation.cpp)
#add_executable(WeightCalculationLauncher main.cpp weightaot.cpp cloudsinterpolation.cpp gaussianfilter.cpp WeightCloud.cpp ComputeTotalWeight.cpp)
#add_executable(${PROJECT_NAME} ${SRC_LIST})
#target_link_libraries(WeightCalculationLauncher ${OTB_LIBRARIES})


