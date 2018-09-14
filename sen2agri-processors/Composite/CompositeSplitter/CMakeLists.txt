otb_create_application(
  NAME           CompositeSplitter2
  SOURCES        CompositeSplitter2.cpp
  LINK_LIBRARIES Sen2AgriProductReaders ${OTB_LIBRARIES})
  
if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_CompositeSplitter2 DESTINATION usr/lib/otb/applications/)
