otb_create_application(
  NAME           XMLStatistics
  SOURCES        XMLStatistics.cpp FluentXML.hpp FluentXML.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_XMLStatistics DESTINATION usr/lib/otb/applications/)
