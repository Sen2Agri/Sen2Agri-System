#include "TsaCountryItaPlugin.h"

TsaCountryItaPlugin::TsaCountryItaPlugin()
{

}

std::string TsaCountryItaPlugin::GetName()
{
    return "ITA";
}

void TsaCountryItaPlugin::UpdateCatchCropLandEfaMarkers()
{

}

void TsaCountryItaPlugin::UpdateFallowLandEfaMarkers(const FieldInfoType &fieldInfos,
                                                         short harvestConfirmWeek,
                                                         time_t ttHarvestConfirmWeekStart,
                                                         const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                                         HarvestEvaluationInfoType &flHarvestEvalInfos)
{
    time_t weekA = fieldInfos.ttPracticeStartWeekFloorTime;
    time_t weekB = fieldInfos.ttPracticeEndWeekFloorTime;
    time_t weekC = weekB;

}

void TsaCountryDefaultPlugin::UpdateNFCEfaMarkers(const FieldInfoType &fieldInfos,
                                                  const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                                  HarvestEvaluationInfoType &ncHarvestEvalInfos)
{
}
