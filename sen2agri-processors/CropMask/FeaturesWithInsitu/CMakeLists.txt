otb_create_application(
  NAME           FeaturesWithInsitu
  SOURCES        FeaturesWithInsitu.cpp FeaturesWithInsitu.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_FeaturesWithInsitu DESTINATION usr/lib/otb/applications/)
