#ifndef TsaNfcAnalysisHandler_H
#define TsaNfcAnalysisHandler_H

#include "TsaEfaAnalysisBase.h"

class TsaNfcAnalysisHandler : public TsaEfaAnalysisBase
{
public:
    TsaNfcAnalysisHandler(itk::Logger *logger);

    virtual bool PerformAnalysis(const FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues,
                                 HarvestEvaluationInfoType &harvestEvalInfos,
                                 HarvestEvaluationInfoType &efaHarvestEvalInfo);
};

#endif
