#ifndef TsaEfaAnalysisBase_H
#define TsaEfaAnalysisBase_H

#include "TimeSeriesAnalysisTypes.h"
#include "TimeSeriesAnalysisUtils.h"
#include "TsaDebugPrinter.h"
#include "TsaDataPreProcessor.h"

class TsaEfaAnalysisBase
{
public:
    TsaEfaAnalysisBase(itk::Logger* logger);
    void SetEfaNdviThr(int val) { m_EfaNdviThr = val; }
    void SetEfaNdviUp(int val) { m_EfaNdviUp = val; }
    void SetEfaNdviDown(int val) { m_EfaNdviDown = val; }

    void SetEfaCohChange(double val) { m_EfaCohChange = val; }
    void SetEfaCohValue(double val) { m_EfaCohValue = val; }

    void SetEfaNdviMin(double val) { m_EfaNdviMin = val; }
    void SetEfaAmpThr(double val) { m_EfaAmpThr = val; }

    void SetCohThrBase(double val) { m_CohThrBase = val; }
    void SetCohThrHigh(double val) { m_CohThrHigh = val; }
    void SetCohThrAbs(double val) { m_CohThrAbs = val; }

    void SetNdviDown(double val) { m_NdviDown = val; }
    void SetNdviUp(double val) { m_NdviUp = val; }
    void SetNdviStep(double val) { m_ndviStep = val; }

    bool ExtractEfaMarkers(time_t ttStartTime, time_t ttEndTime, const FieldInfoType &fieldInfos,
                           std::vector<EfaMarkersInfoType> &efaMarkers);

protected :
    int m_EfaNdviThr;
    int m_EfaNdviUp;
    int m_EfaNdviDown;

    double m_EfaCohChange;
    double m_EfaCohValue;

    double m_EfaNdviMin;
    double m_EfaAmpThr;

    double m_CohThrBase;
    double m_CohThrHigh;
    double m_CohThrAbs;

    // expected value of harvest/clearance
    double m_NdviDown;
    // buffer value (helps in case of sparse ndvi time-series)
    double m_NdviUp;
    // opt.thr.value is round up to ndvi.step
    double m_ndviStep;

    TsaDebugPrinter m_debugPrinter;

    TsaDataExtrPreProcessor m_tsaPreProcessor;
};

#endif
