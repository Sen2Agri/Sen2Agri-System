otb_create_application(
  NAME           CropMaskImageClassifier
  SOURCES        CropMaskImageClassifier.cpp
                 ../Filters/otbCropTypeFeatureExtractionFilter.h
                 ../Filters/otbCropTypeFeatureExtractionFilter.txx
                 ../Filters/otbTemporalResamplingFilter.h
                 ../Filters/otbTemporalResamplingFilter.txx
                 ../Filters/otbTemporalMergingFilter.h
                 ../Filters/otbTemporalMergingFilter.txx
                 ../Filters/CropMaskPreprocessing.h
  LINK_LIBRARIES ${OTB_LIBRARIES}
                 MACCSMetadata
                 TimeSeriesReader)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

include_directories(../OTBExtensions)
include_directories(../Filters)

add_dependencies(otbapp_CropMaskImageClassifier OTBExtensions)

install(TARGETS otbapp_CropMaskImageClassifier DESTINATION usr/lib/otb/applications/)
