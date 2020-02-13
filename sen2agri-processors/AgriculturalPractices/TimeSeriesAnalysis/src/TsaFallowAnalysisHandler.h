#ifndef TsaFallowAnalysisHandler_H
#define TsaFallowAnalysisHandler_H

#include "TsaEfaAnalysisBase.h"

class TsaFallowAnalysisHandler : public TsaEfaAnalysisBase
{
public:
    TsaFallowAnalysisHandler(itk::Logger* logger);

    void SetMarkersStartDate(const std::string &val) { m_flMarkersStartDateStr = val; }
    void SetMarkersEndDate(const std::string &val) { m_flMarkersEndDateStr = val; }

    virtual bool PerformAnalysis(const FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues,
                                 HarvestEvaluationInfoType &harvestEvalInfos,
                                 HarvestEvaluationInfoType &efaHarvestEvalInfo);

private:
    std::string m_flMarkersStartDateStr;
    std::string m_flMarkersEndDateStr;
};

#endif
