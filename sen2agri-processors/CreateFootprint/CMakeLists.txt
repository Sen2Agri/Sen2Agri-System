otb_create_application(
    NAME           CreateFootprint
    SOURCES        CreateFootprint.cxx CreateMaskFromValueFunctor.hxx
    LINK_LIBRARIES ${OTB_LIBRARIES} Sen2AgriProductReaders)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_CreateFootprint DESTINATION usr/lib/otb/applications/)
