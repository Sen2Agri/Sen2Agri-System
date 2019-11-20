#ifndef TsaDebugPrinter_h
#define TsaDebugPrinter_h

#include "TimeSeriesAnalysisTypes.h"

class TsaDebugPrinter
{
public:
    TsaDebugPrinter();
    void SetDebugMode(bool enable) { m_bDebugMode = enable; }

    void PrintFieldGeneralInfos(const FieldInfoType &fieldInfos);
    void PrintMergedValues(const std::vector<MergedAllValInfosType> &mergedVals, double ampThrValue);
    void PrintAmplitudeInfos(const FieldInfoType &fieldInfos);
    void PrintAmpGroupedMeanValues(const std::vector<GroupedMeanValInfosType> &values);
    void PrintNdviInfos(const FieldInfoType &fieldInfos);
    void PrintNdviGroupedMeanValues(const std::vector<GroupedMeanValInfosType> &values);
    void PrintCoherenceInfos(const FieldInfoType &fieldInfos);
    void PrintCoherenceGroupedMeanValues(const std::vector<GroupedMeanValInfosType> &values);
    void PrintHarvestEvaluation(const FieldInfoType &fieldInfo, HarvestEvaluationInfoType &harvestInfo);
    void PrintEfaMarkers(const std::vector<MergedAllValInfosType> &allMergedValues,
                         const std::vector<EfaMarkersInfoType> &efaMarkers);

private:
    std::string GetPlotsFilePath(const std::string &outDir, const std::string &practiceName, const std::string &countryCode, int year);

private:
    bool m_bDebugMode;
};

#endif
