#ifndef TsaCatchCropAnalysisHandler_H
#define TsaCatchCropAnalysisHandler_H

#include "TsaEfaAnalysisBase.h"

class TsaCatchCropAnalysisHandler : public TsaEfaAnalysisBase
{
public:
    TsaCatchCropAnalysisHandler(itk::Logger* logger);

    const char * GetNameOfClass() { return "TsaCatchCropAnalysisHandler"; }

    void SetCatchMain(const std::string &val) {
        m_CatchMain= val;
        if (m_CatchMain.size() == 0) {
            itkExceptionMacro("catch main parameter was not specified for the Catch Crop practice!");
        }
    }
    void SetCatchCropIsMain(const std::string &val) { m_CatchCropIsMain= val; }
    void SetCatchPeriod(int val) { m_CatchPeriod= val; }
    void SetCatchProportion(double val) { m_CatchProportion= val; }
    void SetCatchPeriodStart(const std::string &val) { m_CatchPeriodStart= val; }
    void SetOpticalThrVegCycle(double val) { m_OpticalThrVegCycle= val; }

    virtual bool PerformAnalysis(const FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues,
                                 HarvestEvaluationInfoType &harvestEvalInfos,
                                 HarvestEvaluationInfoType &efaHarvestEvalInfo);

private:
    bool CCEfaMarkersExtraction(time_t ttDateA, time_t ttDateB, time_t weekA,
                             const FieldInfoType &fieldInfos,
                             HarvestEvaluationInfoType &ccHarvestEvalInfos);

private:
    std::string m_CatchMain;
    std::string m_CatchCropIsMain;
    int m_CatchPeriod;                  // in days (e.g. 8 weeks == 56 days)
    double m_CatchProportion;           // buffer threshold
    std::string m_CatchPeriodStart;

    double m_OpticalThrVegCycle;

};

#endif
