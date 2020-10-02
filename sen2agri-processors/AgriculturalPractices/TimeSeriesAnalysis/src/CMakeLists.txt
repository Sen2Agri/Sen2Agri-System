find_package(Boost REQUIRED COMPONENTS regex)

include_directories(../../../Common/OTBExtensions)
include_directories(../../Common/include)
IF (WIN32)
    link_directories(C:/OTB-release-xdk-win32/lib)
ENDIF(WIN32)

otb_create_application(
  NAME           TimeSeriesAnalysis
  SOURCES        otbTimeSeriesAnalysis.cpp
                 ../../Common/include/CommonFunctions.h
                 ../../Common/include/CommonDefs.h
                 ../../Common/include/PracticeReaderBase.h
                 ../../Common/include/PracticeReaderFactory.h
                 ../../Common/src/PracticeReaderFactory.cpp
                 ../../Common/include/PracticeShpReader.h
                 ../../Common/src/PracticeShpReader.cpp
                 ../../Common/include/PracticeCsvReader.h
                 ../../Common/src/PracticeCsvReader.cpp
                 TimeSeriesAnalysisTypes.h
                 TimeSeriesAnalysisUtils.h
                 StatisticsInfosReaderBase.h
                 StatisticsInfosReaderFactory.h
                 StatisticsInfosReaderFactory.cpp
                 StatisticsInfosFolderFilesReader.h
                 StatisticsInfosFolderFilesReader.cpp
                 StatisticsInfosXmlReader.cpp
                 StatisticsInfosXmlReader.h
                 StatisticsInfosSingleCsvReader.h
                 StatisticsInfosSingleCsvReader.cpp
                 Markers1CsvReader.h
                 Markers1CsvReader.cpp
                 TsaCSVWriter.h
                 TsaCSVWriter.cpp
                 TsaPlotsWriter.h
                 TsaPlotsWriter.cpp
                 TsaContinuousFileWriter.h
                 TsaContinuousFileWriter.cpp
                 TsaDebugPrinter.h
                 TsaDebugPrinter.cpp
                 TsaPrevPrdReader.h
                 TsaPrevPrdReader.cpp
#                 TsaCountryPluginsFactory.h
#                 TsaCountryPluginIntf.h
#                 TsaCountryDefaultPlugin.h
#                 TsaCountryDefaultPlugin.cpp
#                 TsaCountryCzePlugin.h
#                 TsaCountryCzePlugin.cpp
#                 TsaCountryRouPlugin.h
#                 TsaCountryRouPlugin.cpp
#                 TsaCountryLtuPlugin.h
#                 TsaCountryLtuPlugin.cpp
#                 TsaCountryItaPlugin.h
#                 TsaCountryItaPlugin.cpp
#                 TsaCountryEspPlugin.h
#                 TsaCountryEspPlugin.cpp
                 TsaEfaAnalysisBase.h
                 TsaEfaAnalysisBase.cpp
                 TsaDataPreProcessor.h
                 TsaDataPreProcessor.cpp
                 TsaDataExtractor.h
                 TsaDataExtractor.cpp
                 TsaHelperFunctions.h
                 TsaCatchCropAnalysisHandler.h
                 TsaCatchCropAnalysisHandler.cpp
                 TsaFallowAnalysisHandler.cpp
                 TsaFallowAnalysisHandler.h
                 TsaNfcAnalysisHandler.h
                 TsaNfcAnalysisHandler.cpp
                 TsaHarvestOnlyAnalysisHandler.cpp
                 TsaHarvestOnlyAnalysisHandler.h

  LINK_LIBRARIES ${OTBExtensions} ${OTB_LIBRARIES} ${OTBCommon_LIBRARIES} ${OTBITK_LIBRARIES} MACCSMetadata Sen2AgriPrdMetaWriters ${Boost_LIBRARIES} gsl)

#if(BUILD_TESTING)
#  add_subdirectory(test)
#endif()

install(TARGETS otbapp_TimeSeriesAnalysis DESTINATION usr/lib/otb/applications/)
