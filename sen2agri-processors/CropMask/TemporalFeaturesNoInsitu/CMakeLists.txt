otb_create_application(
  NAME           TemporalFeaturesNoInsitu
  SOURCES        TemporalFeaturesNoInsitu.cpp TemporalFeaturesNoInsitu.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
