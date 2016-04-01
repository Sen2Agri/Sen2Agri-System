otb_create_application(
  NAME           FeatureExtractionOld
  SOURCES        FeatureExtractionOld.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_FeatureExtractionOld DESTINATION usr/lib/otb/applications/)
