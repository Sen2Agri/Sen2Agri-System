#ifndef TsaCSVWriter_h
#define TsaCSVWriter_h

#include "TimeSeriesAnalysisTypes.h"
#include <map>

class TsaCSVWriter
{
public:
    TsaCSVWriter();
    static std::string BuildResultsCsvFileName(const std::string &practiceName, const std::string &countryCode, int year);
    bool WriteCSVHeader(const std::string &outDir, const std::string &practiceName, const std::string &countryCode, int year);
    void WriteHarvestInfoToCsv(const FieldInfoType &fieldInfo, const HarvestEvaluationInfoType &harvestEvalInfo,
                               const HarvestEvaluationInfoType &efaHarvestEvalInfo);

private:
    std::string GetResultsCsvFilePath(const std::string &outDir, const std::string &practiceName, const std::string &countryCode, int year);
    std::string TranslateHWeekNrDate(const std::string &strHDate);

    std::string GetHWS1Gaps(const FieldInfoType &fieldInfo, const HarvestEvaluationInfoType &harvestEvalInfo,
                            const HarvestEvaluationInfoType &efaHarvestEvalInfo);
    std::string GetHQuality(const FieldInfoType &fieldInfo, const HarvestEvaluationInfoType &harvestEvalInfo,
                            const HarvestEvaluationInfoType &efaHarvestEvalInfo);
    std::string GetCQuality(const FieldInfoType &fieldInfo, const HarvestEvaluationInfoType &harvestEvalInfo,
                            const HarvestEvaluationInfoType &efaHarvestEvalInfo);

private:
    std::ofstream m_OutFileStream;
    std::map<std::string, int> m_IndexPossibleVals;
    std::map<std::string, int> m_HWeekInvalidVals;
};

#endif
