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
