
otb_create_application(
  NAME           BandsExtractor
  SOURCES        BandsExtractor.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
