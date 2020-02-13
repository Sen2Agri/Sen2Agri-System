#ifndef TsaCountryEspPlugin_H
#define TsaCountryEspPlugin_H

#include "TsaCountryDefaultPlugin.h"

class TsaCountryEspPlugin : public TsaCountryDefaultPlugin {
public:
    TsaCountryEspPlugin();
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
