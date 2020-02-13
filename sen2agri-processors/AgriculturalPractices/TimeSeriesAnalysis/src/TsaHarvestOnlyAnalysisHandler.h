#ifndef TsaHarvestOnlyAnalysisHandler_H
#define TsaHarvestOnlyAnalysisHandler_H

#include "itkLogger.h"
#include "itkMacro.h"
#include "otbWrapperMacros.h"

#include "TimeSeriesAnalysisTypes.h"
#include "TsaPrevPrdReader.h"
#include "TsaDebugPrinter.h"

class TsaHarvestOnlyAnalysisHandler
{
public:
    TsaHarvestOnlyAnalysisHandler(itk::Logger* logger);

    void SetPrevPracticeFileName(const std::string &prevPrdDir, const std::string &prevFileName);

    void SetOpticalThrVegCycle(double val) { m_OpticalThrVegCycle= val; }
    void SetNdviDown(double val) { m_NdviDown = val; }
    void SetNdviUp(double val) { m_NdviUp = val; }
    void SetNdviStep(double val) { m_ndviStep = val; }
    void SetOpticalThresholdMinimum(double val) { m_OpticalThresholdMinimum = val; }

    void SetCohThrBase(double val) { m_CohThrBase = val; }
    void SetCohThrHigh(double val) { m_CohThrHigh = val; }
    void SetCohThrAbs(double val) { m_CohThrAbs = val; }

    void SetAmpThrMinimum(double val) { m_AmpThrMinimum = val; }

    void SetUseStdDevInAmpThrValComp(bool val) { m_UseStdDevInAmpThrValComp = val; }
    void SetOpticalThrBufDenominator(int val) { m_OpticalThrBufDenominator = val; }
    void SetAmpThrBreakDenominator(int val) { m_AmpThrBreakDenominator = val; }
    void SetAmpThrValDenominator(int val) { m_AmpThrValDenominator = val; }

    void SetLimitAcqDate(time_t val) { m_ttLimitAcqDate = val; }

    void SetShortenVegWeeks(bool val) { m_bShortenVegWeeks = val;}

    void PerformHarvestEvaluation(FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &allMergedValues,
                                         HarvestEvaluationInfoType &harvestInfos);

private:
    void CheckVegetationStart(FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues);
    void UpdateMarker1Infos(FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues);
    void UpdateMarker2Infos(const FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues);
    void UpdateMarker3Infos(const FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues,
                        double &ampThrValue);
    void UpdateMarker4Infos(const FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues,
                            double &ampThrValue);
    void UpdateMarker5Infos(const FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues);


    bool HarvestEvaluation(const FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues,
                           HarvestEvaluationInfoType &harvestEvalInfos);

    void UpdateGapsInformation(time_t startTime, time_t endTime, time_t ttPrevDate, time_t ttCurDate,
                               int &sumVal, bool countAll = false);
    void UpdateGapsInformation(const std::vector<MergedAllValInfosType> &values,
                               FieldInfoType &fieldInfos, const HarvestEvaluationInfoType &harvestEvalInfos);

    double ComputeOpticalThresholdBuffer(int nVegWeeksCnt, double maxMeanNdviDropVal, double OpticalThresholdValue);
    double ComputeAmplitudeThresholdValue(double meanVal, double stdDevVal);
    double ComputeAmplitudeThresholdBreak(double stdDevVal);

private:
    double m_OpticalThrVegCycle;

    // for MARKER 2 - NDVI loss
    // expected value of harvest/clearance
    double m_NdviDown;
    // buffer value (helps in case of sparse ndvi time-series)
    double m_NdviUp;
    // opt.thr.value is round up to ndvi.step
    double m_ndviStep;
    double m_OpticalThresholdMinimum;

    // for MARKER 5 - COHERENCE increase
    double m_CohThrBase;
    double m_CohThrHigh;
    double m_CohThrAbs;

    // for MARKER 3 - BACKSCATTER loss
    double m_AmpThrMinimum;

    bool m_UseStdDevInAmpThrValComp;
    int m_OpticalThrBufDenominator;
    int m_AmpThrBreakDenominator;
    int m_AmpThrValDenominator;

    time_t m_ttLimitAcqDate;

    bool m_bShortenVegWeeks;

    TsaPrevPrdReader m_prevPrdReader;

    TsaDebugPrinter m_debugPrinter;
    itk::Logger* m_pLogger;

};

#endif
