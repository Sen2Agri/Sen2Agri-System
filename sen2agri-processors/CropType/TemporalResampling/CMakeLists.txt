otb_create_application(
  NAME           TemporalResampling
  SOURCES        TemporalResampling.cpp TemporalResampling.hxx TemporalMerging.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

