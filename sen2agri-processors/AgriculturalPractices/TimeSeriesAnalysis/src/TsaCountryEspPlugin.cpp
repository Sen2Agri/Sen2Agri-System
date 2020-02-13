#include "TsaCountryEspPlugin.h"

TsaCountryEspPlugin::TsaCountryEspPlugin()
{

}

std::string TsaCountryEspPlugin::GetName()
{
    return "ESP";
}

void TsaCountryEspPlugin::UpdateCatchCropLandEfaMarkers()
{

}

void TsaCountryEspPlugin::UpdateFallowLandEfaMarkers(const FieldInfoType &fieldInfos,
                                                         short harvestConfirmWeek,
                                                         time_t ttHarvestConfirmWeekStart,
                                                         const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                                         HarvestEvaluationInfoType &flHarvestEvalInfos)
{
    time_t weekA = fieldInfos.ttPracticeStartWeekFloorTime;
    time_t weekB = fieldInfos.ttPracticeEndWeekFloorTime;
    time_t weekC = weekB;

}

void TsaCountryEspPlugin::UpdateNFCEfaMarkers(const FieldInfoType &fieldInfos,
                                                  const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                                  HarvestEvaluationInfoType &ncHarvestEvalInfos)
{
}
