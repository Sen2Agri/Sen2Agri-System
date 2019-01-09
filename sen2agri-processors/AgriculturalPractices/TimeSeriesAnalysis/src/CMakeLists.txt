find_package(Boost REQUIRED COMPONENTS filesystem)

include_directories(../../../Common/OTBExtensions)
include_directories(../../Common/include)

otb_create_application(
  NAME           TimeSeriesAnalysis
  SOURCES        otbTimeSeriesAnalysis.cpp
                 ../../Common/include/CommonFunctions.h ../../Common/include/CommonDefs.h
                 TimeSeriesAnalysisTypes.h
                 TimeSeriesAnalysisUtils.h
                 PracticesEvaluator.h
                 PracticesEvaluator.cpp
                 PracticeReaderBase.h
                 PracticeReaderFactory.h
                 PracticeReaderFactory.cpp
                 PracticeShpReader.h
                 PracticeShpReader.cpp
                 PracticeCsvReader.h
                 PracticeCsvReader.cpp
                 StatisticsInfosReaderBase.h
                 StatisticsInfosReaderFactory.h
                 StatisticsInfosReaderFactory.cpp
                 StatisticsInfosFolderFilesReader.h
                 StatisticsInfosFolderFilesReader.cpp
                 StatisticsInfosXmlReader.cpp
                 StatisticsInfosXmlReader.h
                 StatisticsInfosSingleCsvReader.h
                 StatisticsInfosSingleCsvReader.cpp


  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata Sen2AgriPrdMetaWriters boost_regex gsl)

#if(BUILD_TESTING)
#  add_subdirectory(test)
#endif()

install(TARGETS otbapp_TimeSeriesAnalysis DESTINATION usr/lib/otb/applications/)
