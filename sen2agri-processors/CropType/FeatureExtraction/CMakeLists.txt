otb_create_application(
  NAME           FeatureExtraction
  SOURCES        FeatureExtraction.cpp FeatureExtraction.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_FeatureExtraction DESTINATION usr/lib/otb/applications/)
