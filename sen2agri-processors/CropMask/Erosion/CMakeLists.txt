otb_create_application(
  NAME           Erosion
  SOURCES        Erosion.cpp 
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_Erosion DESTINATION usr/lib/otb/applications/)
