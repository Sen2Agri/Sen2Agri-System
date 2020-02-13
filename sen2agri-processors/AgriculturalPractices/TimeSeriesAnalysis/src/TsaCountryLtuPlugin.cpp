#include "TsaCountryLtuPlugin.h"

TsaCountryLtuPlugin::TsaCountryLtuPlugin()
{

}

std::string TsaCountryLtuPlugin::GetName()
{
    return "LTU";
}

void TsaCountryLtuPlugin::UpdateFallowLandEfaMarkers(const FieldInfoType &fieldInfos,
                                                         short harvestConfirmWeek,
                                                         time_t ttHarvestConfirmWeekStart,
                                                         const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                                         HarvestEvaluationInfoType &flHarvestEvalInfos)
{
    time_t weekA = fieldInfos.ttPracticeStartWeekFloorTime;
    time_t weekB = fieldInfos.ttPracticeEndWeekFloorTime;
    time_t weekC = weekB;

}

void TsaCountryLtuPlugin::UpdateNFCEfaMarkers(const FieldInfoType &fieldInfos,
                                                  const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                                  HarvestEvaluationInfoType &ncHarvestEvalInfos)
{
}
