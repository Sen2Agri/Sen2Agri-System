otb_create_application(
  NAME           BandsExtractor
  SOURCES        BandsExtractor.cpp otbConcatenateVectorImagesFilter.h otbConcatenateVectorImagesFilter.txx
  LINK_LIBRARIES ${OTB_LIBRARIES} MACCSMetadata)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_BandsExtractor DESTINATION usr/lib/otb/applications/)
