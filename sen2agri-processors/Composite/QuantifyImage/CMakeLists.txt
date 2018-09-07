otb_create_application(
  NAME           QuantifyImage
  SOURCES        QuantifyImage.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES} Sen2AgriProductReaders Sen2AgriCommonUtils)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_QuantifyImage DESTINATION usr/lib/otb/applications/)
