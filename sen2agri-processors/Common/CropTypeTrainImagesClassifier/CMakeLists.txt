otb_create_application(
  NAME           CropTypeTrainImagesClassifier
  SOURCES        otbTrainImagesClassifier.cxx otbTrainImagesClassifier.h
  LINK_LIBRARIES ${OTB_LIBRARIES} MACCSMetadata)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

include_directories(../../Common/OTBExtensions)

add_dependencies(otbapp_BandsExtractor OTBExtensions)

install(TARGETS otbapp_CropTypeTrainImagesClassifier DESTINATION usr/lib/otb/applications/)
