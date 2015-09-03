otb_create_application(
  NAME           StatisticFeatures
  SOURCES        StatisticFeatures.cpp StatisticFeatures.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_StatisticFeatures DESTINATION lib/otb/applications/)
