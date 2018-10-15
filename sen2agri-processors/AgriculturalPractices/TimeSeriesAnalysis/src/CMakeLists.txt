find_package(Boost REQUIRED COMPONENTS filesystem)

include_directories(../../../Common/OTBExtensions)

otb_create_application(
  NAME           TimeSeriesAnalysis
  SOURCES        otbTimeSeriesAnalysis.cpp
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
                 StatisticsInfosXmlReader.cpp
                 StatisticsInfosXmlReader.h
                 StatisticsInfosFolderFilesReader.h
                 StatisticsInfosFolderFilesReader.cpp
                 StatisticsInfosReaderFactory.h
                 StatisticsInfosReaderFactory.cpp


  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata Sen2AgriPrdMetaWriters boost_regex)

#if(BUILD_TESTING)
#  add_subdirectory(test)
#endif()

install(TARGETS otbapp_TimeSeriesAnalysis DESTINATION usr/lib/otb/applications/)
