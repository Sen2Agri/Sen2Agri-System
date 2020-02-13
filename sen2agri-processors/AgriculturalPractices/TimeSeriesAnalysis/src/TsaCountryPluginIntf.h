#ifndef TsaCountryPluginIntf_H
#define TsaCountryPluginIntf_H

#include <fstream>
#include <boost/algorithm/string.hpp>

#include "TimeSeriesAnalysisTypes.h"
#include "TimeSeriesAnalysisUtils.h"

class TsaCountryPluginIntf {
public:
    TsaCountryPluginIntf();
    virtual std::string GetName() = 0;

    void SetFallowMarkersStartDate(const std::string &flMarkersStartDateStr) { m_flMarkersStartDateStr = flMarkersStartDateStr;}

    virtual void UpdateCatchCropLandEfaMarkers() = 0;
    virtual void UpdateFallowLandEfaMarkers(const FieldInfoType &fieldInfos, short harvestConfirmWeek,
                                            time_t ttHarvestConfirmWeekStart,
                                            const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                            HarvestEvaluationInfoType &flHarvestEvalInfos) = 0;
    virtual void UpdateNFCEfaMarkers(const FieldInfoType &fieldInfos,
                                     const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                     HarvestEvaluationInfoType &ncHarvestEvalInfos) = 0;

    bool CheckCountryCode(const std::string &str, const std::string &countryCode) {
        return boost::starts_with(str, countryCode);
    }

protected:
    bool AllNdviMeanAreNA(const std::vector<EfaMarkersInfoType> &efaMarkers) {
        bool allNdviMeanAreNA = true;
        for(size_t i = 0; i<efaMarkers.size(); i++) {
            if (!IsNA(efaMarkers[i].ndviMean)) {
                allNdviMeanAreNA = false;
                break;
            }
        }
        return allNdviMeanAreNA;
    }


    std::string m_flMarkersStartDateStr;
};

#endif
