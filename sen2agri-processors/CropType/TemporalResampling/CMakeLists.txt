otb_create_application(
  NAME           TemporalResampling
  SOURCES        TemporalResampling.cpp TemporalResampling.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_TemporalResampling DESTINATION lib/otb/applications/)
