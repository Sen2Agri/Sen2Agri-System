find_package(Boost REQUIRED COMPONENTS filesystem)

include_directories(../../../Common/OTBExtensions)

otb_create_application(
  NAME           PolygonClassStatistics
  SOURCES        otbPolygonClassStatistics.cxx otbPersistentSamplingFilterBase.h otbPersistentSamplingFilterBase.txx otbOGRDataToClassStatisticsFilter.h otbOGRDataToClassStatisticsFilter.txx otbMaskedIteratorDecorator.h otbMaskedIteratorDecorator.txx otbStopwatch.h otbStopwatch.cxx otbStatisticsXMLFileWriter.h otbStatisticsXMLFileWriter.txx
  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata Sen2AgriPrdMetaWriters)

#if(BUILD_TESTING)
#  add_subdirectory(test)
#endif()

