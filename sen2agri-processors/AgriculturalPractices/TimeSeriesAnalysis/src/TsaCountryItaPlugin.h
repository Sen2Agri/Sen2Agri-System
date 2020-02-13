#ifndef TsaCountryItaPlugin_H
#define TsaCountryItaPlugin_H

#include "TsaCountryPluginIntf.h"

class TsaCountryItaPlugin : public TsaCountryDefaultPlugin {
public:
    TsaCountryItaPlugin();
    virtual std::string GetName();

    virtual void UpdateFallowLandEfaMarkers(const FieldInfoType &fieldInfos, short harvestConfirmWeek,
                                            time_t ttHarvestConfirmWeekStart,
                                            const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                            HarvestEvaluationInfoType &flHarvestEvalInfos);
    virtual void UpdateNFCEfaMarkers(const FieldInfoType &fieldInfos,
                                     const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                     HarvestEvaluationInfoType &ncHarvestEvalInfos);
};

#endif
