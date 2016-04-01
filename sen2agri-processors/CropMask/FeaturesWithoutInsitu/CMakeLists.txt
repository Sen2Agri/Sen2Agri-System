otb_create_application(
  NAME           FeaturesWithoutInsitu
  SOURCES        FeaturesWithoutInsitu.cpp FeaturesWithoutInsitu.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_FeaturesWithoutInsitu DESTINATION usr/lib/otb/applications/)
