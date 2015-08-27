otb_create_application(
  NAME           TemporalFeatures
  SOURCES        TemporalFeatures.cpp TemporalFeatures.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_TemporalFeatures DESTINATION lib/otb/applications/)
