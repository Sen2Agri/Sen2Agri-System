#ifndef TsaContinuousFileWriter_h
#define TsaContinuousFileWriter_h

#include "TimeSeriesAnalysisTypes.h"

class TsaContinuousFileWriter
{
public:
    TsaContinuousFileWriter();
    void SetEnabled(bool enable) { m_bResultContinuousProduct = enable; }
    void CreateContinousProductFile(const std::string &outDir, const std::string &practiceName, const std::string &countryCode, int year);
    void WriteContinousToCsv(const FieldInfoType &fieldInfo, const std::vector<MergedAllValInfosType> &allMergedVals);

private:
    std::string GetContinousProductCsvFilePath(const std::string &outDir, const std::string &practiceName,
                                               const std::string &countryCode, int year);

private:
    bool m_bResultContinuousProduct;
    std::ofstream m_OutContinousPrdFileStream;
};

#endif
