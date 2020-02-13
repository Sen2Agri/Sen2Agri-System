#ifndef TsaDataExtractor_H
#define TsaDataExtractor_H

#include "TimeSeriesAnalysisTypes.h"
#include "itkLogger.h"
#include "itkMacro.h"
#include "otbWrapperMacros.h"
#include "StatisticsInfosReaderBase.h"

class TsaDataExtractor
{
public:
    TsaDataExtractor(itk::Logger* logger);

    void SetAllowGaps(bool allow) {m_bAllowGaps = allow; }
    void SetVerbose(bool verbose) {m_bVerbose = verbose; }

    void Initialize(const std::string &ampSrc, const std::string &coheSrc, const std::string &ndviSrc, int minCoheAcqs, int curYear, const std::string &inputType);

    std::vector<InputFileLineInfoType> FilterDuplicates(const std::vector<InputFileLineInfoType> &lineInfos);
    bool ExtractAmplitudeFilesInfos(FieldInfoType &fieldInfo);
    bool ExtractCoherenceFilesInfos(FieldInfoType &fieldInfo);
    bool ExtractNdviFilesInfos(FieldInfoType &fieldInfo);
    bool CheckWeekGaps(int &vegetationStartWeek, const std::vector<InputFileLineInfoType> &inLineInfos);

    itk::Logger* GetLogger() { return m_pLogger; }

private:
    void KeepCommonDates(const std::vector<InputFileLineInfoType> &lineInfos1, const std::vector<InputFileLineInfoType> &lineInfos2,
                         std::vector<InputFileLineInfoType> &retLineInfos1, std::vector<InputFileLineInfoType> &retLineInfos2);

    itk::Logger* m_pLogger;
    bool m_bVerbose;
    bool m_bAllowGaps;

    std::unique_ptr<StatisticsInfosReaderBase> m_pAmpReader;
    std::unique_ptr<StatisticsInfosReaderBase> m_pNdviReader;
    std::unique_ptr<StatisticsInfosReaderBase> m_pCoheReader;

};

#endif
