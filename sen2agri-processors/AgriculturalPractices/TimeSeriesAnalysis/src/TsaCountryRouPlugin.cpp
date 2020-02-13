#include "TsaCountryRouPlugin.h"

TsaCountryRouPlugin::TsaCountryDefaultPlugin()
{

}

std::string TsaCountryRouPlugin::GetName()
{
    return "ROU";
}

void TsaCountryRouPlugin::UpdateCatchCropLandEfaMarkers()
{

}

void TsaCountryRouPlugin::UpdateFallowLandEfaMarkers(const FieldInfoType &fieldInfos,
                                                         short harvestConfirmWeek,
                                                         time_t ttHarvestConfirmWeekStart,
                                                         const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                                         HarvestEvaluationInfoType &flHarvestEvalInfos)
{
    time_t weekA = fieldInfos.ttPracticeStartWeekFloorTime;
    time_t weekB = fieldInfos.ttPracticeEndWeekFloorTime;
    time_t weekC = weekB;

}

void TsaCountryRouPlugin::UpdateNFCEfaMarkers(const FieldInfoType &fieldInfos,
                                                  const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                                  HarvestEvaluationInfoType &ncHarvestEvalInfos)
{
}
