otb_create_application(
  NAME           TotalWeight
  SOURCES        totalweightcomputation.cpp totalweightapp.cpp
  LINK_LIBRARIES Sen2AgriProductReaders Sen2AgriCommonUtils ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_TotalWeight DESTINATION usr/lib/otb/applications/)
