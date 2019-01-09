find_package(Boost REQUIRED COMPONENTS filesystem)

include_directories(../../../Common/OTBExtensions)
include_directories(../../Common/include)

otb_create_application(
  NAME           AgricPractDataExtraction
  SOURCES        otbAgricPractDataExtraction.cxx otbPersistentSamplingFilterBase.h otbPersistentSamplingFilterBase.txx
                 otbOGRDataToClassStatisticsFilter.h otbOGRDataToClassStatisticsFilter.txx
                 otbMaskedIteratorDecorator.h otbMaskedIteratorDecorator.txx otbStopwatch.h otbStopwatch.cxx
                 otbAgricPractDataExtrFileWriter2.h otbAgricPractDataExtrFileWriter2.txx
                 ../../Common/include/CommonFunctions.h ../../Common/include/CommonDefs.h
  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata Sen2AgriPrdMetaWriters boost_regex Sen2AgriCommonUtils)

otb_create_application(
  NAME           AgricPractMergeDataExtractionFiles
  SOURCES        otbAgricPractMergeDataExtractionFiles.cxx
                 ../../Common/include/CommonFunctions.h ../../Common/include/CommonDefs.h
  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata boost_regex)

otb_create_application(
  NAME           IndexAgricPracticesXml
  SOURCES        otbIndexAgricPracticesXml.cxx
                 ../../Common/include/CommonFunctions.h ../../Common/include/CommonDefs.h
  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata boost_regex)

#otb_create_application(
#  NAME           AgricPracticesXmlToTxtFiles
#  SOURCES        CommonFunctions.h otbAgricPracticesXmlToTxtFiles.cxx
#  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata boost_regex)

#if(BUILD_TESTING)
#  add_subdirectory(test)
#endif()

install(TARGETS otbapp_AgricPractDataExtraction DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_AgricPractMergeDataExtractionFiles DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_IndexAgricPracticesXml DESTINATION usr/lib/otb/applications/)
#install(TARGETS otbapp_AgricPracticesXmlToTxtFiles DESTINATION usr/lib/otb/applications/)
