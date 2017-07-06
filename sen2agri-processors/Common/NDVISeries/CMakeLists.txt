otb_create_application(
    NAME           NDVISeries
    SOURCES        NDVISeries.cpp
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

add_dependencies(otbapp_NDVISeries OTBExtensions)

install(TARGETS otbapp_NDVISeries DESTINATION usr/lib/otb/applications/)
