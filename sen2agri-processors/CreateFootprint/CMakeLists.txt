otb_create_application(
    NAME           CreateFootprint
    SOURCES        CreateFootprint.cxx CreateMaskFromValueFunctor.hxx
    LINK_LIBRARIES ${OTB_LIBRARIES} MACCSMetadata)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_CreateFootprint DESTINATION lib/otb/applications/)
