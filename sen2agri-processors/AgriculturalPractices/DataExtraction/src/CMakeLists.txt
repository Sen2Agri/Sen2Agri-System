find_package(Boost REQUIRED COMPONENTS filesystem)

include_directories(../../../Common/OTBExtensions)

otb_create_application(
  NAME           PolygonClassStatistics
  SOURCES        otbPolygonClassStatistics.cxx otbPersistentSamplingFilterBase.h otbPersistentSamplingFilterBase.txx
                 otbOGRDataToClassStatisticsFilter.h otbOGRDataToClassStatisticsFilter.txx
                 otbMaskedIteratorDecorator.h otbMaskedIteratorDecorator.txx otbStopwatch.h otbStopwatch.cxx
                 otbStatisticsXMLFileWriter.h otbStatisticsXMLFileWriter.txx
                 otbAgricPractDataExtrFileWriter2.h otbAgricPractDataExtrFileWriter2.txx
                 CommonFunctions.h
  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata Sen2AgriPrdMetaWriters boost_regex)

otb_create_application(
  NAME           AgricPractMergeDataExtractionFiles
  SOURCES        CommonFunctions.h otbAgricPractMergeDataExtractionFiles.cxx
  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata boost_regex)

otb_create_application(
  NAME           LPISDataSelection
  SOURCES        CommonFunctions.h otbLPISDataSelection.cxx
  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata boost_regex)

#if(BUILD_TESTING)
#  add_subdirectory(test)
#endif()

install(TARGETS otbapp_PolygonClassStatistics DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_AgricPractMergeDataExtractionFiles DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_LPISDataSelection DESTINATION usr/lib/otb/applications/)
