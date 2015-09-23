otb_create_application(
  NAME           SpectralFeatures
  SOURCES        SpectralFeatures.cpp SpectralFeatures.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_SpectralFeatures DESTINATION lib/otb/applications/)
