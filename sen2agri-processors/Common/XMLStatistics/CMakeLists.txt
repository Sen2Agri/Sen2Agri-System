otb_create_application(
  NAME           XMLStatistics
  SOURCES        XMLStatistics.cpp FluentXML.hpp FluentXML.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
