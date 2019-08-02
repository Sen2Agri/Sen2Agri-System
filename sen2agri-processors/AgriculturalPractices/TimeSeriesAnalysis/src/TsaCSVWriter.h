#ifndef TsaCSVWriter_h
#define TsaCSVWriter_h

#include "TimeSeriesAnalysisTypes.h"

class TsaCSVWriter
{
public:
    TsaCSVWriter();
    static std::string BuildResultsCsvFileName(const std::string &practiceName, const std::string &countryCode, int year);
    bool WriteCSVHeader(const std::string &outDir, const std::string &practiceName, const std::string &countryCode, int year);
    void WriteHarvestInfoToCsv(const FieldInfoType &fieldInfo, const HarvestInfoType &harvestInfo, const HarvestInfoType &efaHarvestInfo);

private:
    std::string GetResultsCsvFilePath(const std::string &outDir, const std::string &practiceName, const std::string &countryCode, int year);
    std::string TranslateHWeekNrDate(const std::string &strHDate);

private:
    std::ofstream m_OutFileStream;
};

#endif
