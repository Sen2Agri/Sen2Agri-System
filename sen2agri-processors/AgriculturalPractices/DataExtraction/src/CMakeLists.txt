find_package(Boost REQUIRED COMPONENTS filesystem)

include_directories(../../../Common/OTBExtensions)
include_directories(../../Common/include)
find_package(Boost REQUIRED COMPONENTS regex)

otb_create_application(
  NAME           AgricPractDataExtraction
  SOURCES        otbAgricPractDataExtraction.cxx
                 otbPersistentSamplingFilterBase.h
                 otbPersistentSamplingFilterBase.txx
                 otbOGRDataToClassStatisticsFilter.h
                 otbOGRDataToClassStatisticsFilter.txx
                 otbMaskedIteratorDecorator.h
                 otbMaskedIteratorDecorator.txx
                 otbStopwatch.h
                 otbStopwatch.cxx
                 otbAgricPractDataExtrFileWriter2.h
                 otbAgricPractDataExtrFileWriter2.txx
                 DeclarationsInfo.h
                 DeclarationsInfo.cpp
                 ../../Common/include/CommonFunctions.h
                 ../../Common/include/CommonDefs.h
                 ../../Common/include/AttributeEntry.h
                 ../../Common/include/GSAAAttributesTablesReaderBase.h
                 ../../Common/src/GSAAAttributesTablesReaderFactory.cpp
                 ../../Common/include/GSAAAttributesTablesReaderFactory.h
                 ../../Common/src/GSAACsvAttributesTablesReader.cpp
                 ../../Common/include/GSAACsvAttributesTablesReader.h
                 ../../Common/src/GSAAShpAttributesTablesReader.cpp
                 ../../Common/include/GSAAShpAttributesTablesReader.h
                 ../../Common/include/DeclarationsInfoBase.h
                 ../../Common/src/DeclarationsInfoBase.cpp
                 ../../Common/include/PracticeReaderFactory.h
                 ../../Common/src/PracticeReaderFactory.cpp
                 ../../Common/include/PracticeShpReader.h
                 ../../Common/src/PracticeShpReader.cpp
                 ../../Common/include/PracticeCsvReader.h
                 ../../Common/src/PracticeCsvReader.cpp

  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata Sen2AgriPrdMetaWriters ${Boost_LIBRARIES} Sen2AgriCommonUtils)

otb_create_application(
  NAME           AgricPractMergeDataExtractionFiles
  SOURCES        otbAgricPractMergeDataExtractionFiles.cxx
                 ../../Common/include/CommonFunctions.h ../../Common/include/CommonDefs.h
  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata ${Boost_LIBRARIES})

otb_create_application(
  NAME           IndexAgricPracticesXml
  SOURCES        otbIndexAgricPracticesXml.cxx
                 ../../Common/include/CommonFunctions.h ../../Common/include/CommonDefs.h
  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata ${Boost_LIBRARIES})

#otb_create_application(
#  NAME           AgricPracticesXmlToTxtFiles
#  SOURCES        CommonFunctions.h otbAgricPracticesXmlToTxtFiles.cxx
#  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata ${Boost_LIBRARIES})

#if(BUILD_TESTING)
#  add_subdirectory(test)
#endif()

install(TARGETS otbapp_AgricPractDataExtraction DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_AgricPractMergeDataExtractionFiles DESTINATION usr/lib/otb/applications/)
install(TARGETS otbapp_IndexAgricPracticesXml DESTINATION usr/lib/otb/applications/)
#install(TARGETS otbapp_AgricPracticesXmlToTxtFiles DESTINATION usr/lib/otb/applications/)
