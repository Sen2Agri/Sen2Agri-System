#ifndef TsaCountryLtuPlugin_H
#define TsaCountryLtuPlugin_H

#include "TsaCountryDefaultPlugin.h"

class TsaCountryLtuPlugin : public TsaCountryDefaultPlugin {
public:
    TsaCountryLtuPlugin();
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
