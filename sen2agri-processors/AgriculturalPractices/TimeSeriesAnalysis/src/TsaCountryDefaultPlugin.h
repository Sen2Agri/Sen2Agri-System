#ifndef TsaCountryDefaultPlugin_H
#define TsaCountryDefaultPlugin_H

#include "TsaCountryPluginIntf.h"

class TsaCountryDefaultPlugin : public TsaCountryPluginIntf {
public:
    TsaCountryDefaultPlugin();
    virtual std::string GetName();

    virtual void UpdateCatchCropLandEfaMarkers();
    virtual void UpdateFallowLandEfaMarkers(const FieldInfoType &fieldInfos, short harvestConfirmWeek,
                                            time_t ttHarvestConfirmWeekStart,
                                            const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                            HarvestEvaluationInfoType &flHarvestEvalInfos);
    virtual void UpdateNFCEfaMarkers(const FieldInfoType &fieldInfos,
                                     const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                     HarvestEvaluationInfoType &ncHarvestEvalInfos);
};

#endif
