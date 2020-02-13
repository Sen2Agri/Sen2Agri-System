#include "TsaCountryCzePlugin.h"

TsaCountryCzePlugin::TsaCountryCzePlugin()
{
}

std::string TsaCountryCzePlugin::GetName()
{
    return "CZE";
}

void TsaCountryCzePlugin::UpdateFallowLandEfaMarkers(const FieldInfoType &fieldInfos,
                                                         short harvestConfirmWeek,
                                                         time_t ttHarvestConfirmWeekStart,
                                                         const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                                         HarvestEvaluationInfoType &flHarvestEvalInfos)
{
    time_t weekA = fieldInfos.ttPracticeStartWeekFloorTime;
    time_t weekB = fieldInfos.ttPracticeEndWeekFloorTime;
    time_t weekC = weekB;

}

void TsaCountryCzePlugin::UpdateNFCEfaMarkers(const FieldInfoType &fieldInfos,
                                                  const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                                  HarvestEvaluationInfoType &ncHarvestEvalInfos)
{
    time_t ttDateA = fieldInfos.ttPracticeStartTime;     // last possible start of catch-crop period
    time_t ttDateB = fieldInfos.ttPracticeEndTime;     // last possible end of catch-crop period
    time_t weekB = fieldInfos.ttPracticeEndWeekFloorTime;
    if (!ncHarvestEvalInfos.ndviPresence) {
        // # no evidence of the NFC vegetation in the vegetation season - return POOR evaluation
        ncHarvestEvalInfos.efaIndex = "POOR";
        return true;

    } else if (!IsNA(harvestConfirmWeek)) {
        // # set the first day of the harvest week - evaluate
        time_t harvestConfirmed = ttHarvestConfirmWeekStart;
        if (harvestConfirmed > weekA && harvestConfirmed <= weekB) {
            // # harvest detected within the efa practice period - NFC is considered not ok - return WEAK evaluation
            ncHarvestEvalInfos.efaIndex = "WEAK";
            return true;
        }
    } //else {
        // # vegetation is present, harvest was not detected - NFC is considered ok - evaluate
    // }

    // ### MARKER 6 - efa.presence.ndvi ###
    std::vector<EfaMarkersInfoType> efaMarkers2;
    // # get efa markers for the specified period & buffer
    ExtractEfaMarkers(ttDateA, ttDateB, fieldInfos, efaMarkers2);

    // #  if there is evidence of vegetation (NDVI>efa.ndvi.thr) in the EFA practice period - M6 is TRUE
    ncHarvestEvalInfos.ndviPresence = IsNdviPresence(efaMarkers2);
    if (ncHarvestEvalInfos.ndviPresence) {
        ncHarvestEvalInfos.efaIndex = "STRONG";
    } else {
        ncHarvestEvalInfos.efaIndex = "MODERATE";
    }

}
