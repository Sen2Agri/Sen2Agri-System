otb_create_application(
    NAME           ContinuousColorMapping
    SOURCES        ContinuousColorMapping.cxx ContinuousColorMappingFunctor.hxx ContinuousColorMappingFilter.hxx
    LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_ContinuousColorMapping DESTINATION usr/lib/otb/applications/)
