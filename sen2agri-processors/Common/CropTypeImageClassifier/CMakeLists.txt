otb_create_application(
  NAME           CropTypeImageClassifier
  SOURCES        CropTypeImageClassifier.cpp
                 ../Filters/otbCropTypeFeatureExtractionFilter.h
                 ../Filters/otbCropTypeFeatureExtractionFilter.txx
                 ../Filters/otbTemporalResamplingFilter.h
                 ../Filters/otbTemporalResamplingFilter.txx
                 ../Filters/otbTemporalMergingFilter.h
                 ../Filters/otbTemporalMergingFilter.txx
                 ../Filters/CropTypePreprocessing.h
  LINK_LIBRARIES ${OTB_LIBRARIES}
                 MACCSMetadata
                 TimeSeriesReader)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

include_directories(../OTBExtensions)
include_directories(../Filters)

add_dependencies(otbapp_CropTypeImageClassifier OTBExtensions)

install(TARGETS otbapp_CropTypeImageClassifier DESTINATION usr/lib/otb/applications/)
