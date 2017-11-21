otb_create_application(
  NAME           MeanShiftSegmentation
  SOURCES        MeanShiftSegmentation.cpp 
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
