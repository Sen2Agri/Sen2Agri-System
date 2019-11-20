#ifndef TsaPlotsWriter_h
#define TsaPlotsWriter_h

#include "TimeSeriesAnalysisTypes.h"

class TsaPlotsWriter
{
public:
    TsaPlotsWriter();
    void SetEnabled(bool enable) { m_bPlotOutputGraph = enable; }
    void CreatePlotsFile(const std::string &outDir, const std::string &practiceName, const std::string &countryCode, int year);
    void ClosePlotsFile();
    void WritePlotEntry(const FieldInfoType &fieldInfos, const HarvestEvaluationInfoType &harvestInfo, const HarvestEvaluationInfoType &efaHarvestInfo, bool hasEfaInfos);

private:
    std::string GetPlotsFilePath(const std::string &outDir, const std::string &practiceName, const std::string &countryCode, int year);

private:
    bool m_bPlotOutputGraph;
    std::ofstream m_OutPlotsFileStream;
    std::ofstream m_OutPlotsIdxFileStream;
    uintmax_t m_OutPlotsIdxCurIdx;


};

#endif
