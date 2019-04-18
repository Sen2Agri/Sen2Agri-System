find_package(Boost REQUIRED COMPONENTS filesystem)

#include_directories(../../../Common/XmlUtils/include)
include_directories(../../ProductMetaWriters/include)

otb_create_application(
  NAME           ProductFormatter
  SOURCES        ProductFormatter.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES} Sen2AgriProductReaders Sen2AgriPrdMetaWriters)

#if(BUILD_TESTING)
#  add_subdirectory(test)
#endif()

install(TARGETS otbapp_ProductFormatter DESTINATION usr/lib/otb/applications/)
