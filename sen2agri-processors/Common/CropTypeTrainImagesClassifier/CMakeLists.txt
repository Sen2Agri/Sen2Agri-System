otb_create_application(
  NAME           CropTypeTrainImagesClassifier
  SOURCES        otbTrainImagesClassifier.cxx otbTrainImagesClassifier.h
                 ../Filters/CropTypePreprocessing.h
  LINK_LIBRARIES ${OTB_LIBRARIES}
                 MACCSMetadata
                 TimeSeriesReader)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

include_directories(../../Common/OTBExtensions)

install(TARGETS otbapp_CropTypeTrainImagesClassifier DESTINATION usr/lib/otb/applications/)
