otb_create_application(
  NAME           CropTypeImageClassifier
  SOURCES        CropTypeImageClassifier.cpp
                 ../Filters/otbCropTypeFeatureExtractionFilter.h
                 ../Filters/otbCropTypeFeatureExtractionFilter.txx
                 ../Filters/otbTemporalResamplingFilter.h
                 ../Filters/otbTemporalResamplingFilter.txx
                 ../Filters/otbTemporalMergingFilter.h
                 ../Filters/otbTemporalMergingFilter.txx
                 ../Filters/otbSpotMaskFilter.h
                 ../Filters/otbSpotMaskFilter.txx
                 ../Filters/otbLandsatMaskFilter.h
                 ../Filters/otbLandsatMaskFilter.txx
                 ../Filters/otbSentinelMaskFilter.h
                 ../Filters/otbSentinelMaskFilter.txx
                 ../Filters/CropTypePreprocessing.h
                 ../Filters/TimeSeriesReader.h
  LINK_LIBRARIES ${OTB_LIBRARIES} MACCSMetadata)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

include_directories(../OTBExtensions)

add_dependencies(otbapp_CropTypeImageClassifier OTBExtensions)

install(TARGETS otbapp_CropTypeImageClassifier DESTINATION usr/lib/otb/applications/)
