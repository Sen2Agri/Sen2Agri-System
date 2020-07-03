otb_create_application(
	NAME           OpticalFeatures
	SOURCES        OpticalFeatures.cpp
				 ../../../Common/Filters/otbCropTypeFeatureExtractionFilter.h
				 ../../../Common/Filters/otbCropTypeFeatureExtractionFilter.txx
				 ../../../Common/Filters/otbTemporalResamplingFilter.h
				 ../../../Common/Filters/otbTemporalResamplingFilter.txx
				 ../../../Common/Filters/otbTemporalMergingFilter.h
				 ../../../Common/Filters/otbTemporalMergingFilter.txx
				 ../../../Common/Filters/CropTypePreprocessing.h
				 ../../../Common/Filters/RedEdgeBandsExtractor.h
                 ../../../Common/Filters/MaskedScaleImageFilter.h
                 ../../../Common/Filters/otbStreamingStatisticsMapFromLabelImageFilter.h
                 ../../../Common/Filters/otbStreamingStatisticsMapFromLabelImageFilter.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES}
                 MACCSMetadata
                 TimeSeriesReader)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

include_directories(../../../Common/OTBExtensions)
include_directories(../../../Common/Filters)

add_dependencies(otbapp_OpticalFeatures OTBExtensions)

install(TARGETS otbapp_OpticalFeatures DESTINATION usr/lib/otb/applications/)
