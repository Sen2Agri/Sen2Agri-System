otb_create_application(
  NAME           QualityFlagsExtractor
  SOURCES        QualityFlagsExtractor.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES} Sen2AgriProductReaders)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_QualityFlagsExtractor DESTINATION usr/lib/otb/applications/)
