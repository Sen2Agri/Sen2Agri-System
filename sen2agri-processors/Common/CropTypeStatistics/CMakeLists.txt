otb_create_application(
  NAME           CropTypeStatistics
  SOURCES        CropTypeStatistics.cpp
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
  LINK_LIBRARIES ${OTB_LIBRARIES} MACCSMetadata)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

include_directories(../OTBExtensions)

add_dependencies(otbapp_CropTypeStatistics OTBExtensions)

install(TARGETS otbapp_CropTypeStatistics DESTINATION usr/lib/otb/applications/)
