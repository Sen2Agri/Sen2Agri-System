#include "TsaCountryDefaultPlugin.h"

TsaCountryDefaultPlugin::TsaCountryDefaultPlugin()
{

}

std::string TsaCountryDefaultPlugin::GetName()
{
    return "NO_NAME";
}

void TsaCountryDefaultPlugin::UpdateCatchCropLandEfaMarkers()
{

}

void TsaCountryDefaultPlugin::UpdateFallowLandEfaMarkers(const FieldInfoType &fieldInfos,
                                                         short harvestConfirmWeek,
                                                         time_t ttHarvestConfirmWeekStart,
                                                         const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                                         HarvestEvaluationInfoType &flHarvestEvalInfos)
{
    time_t weekA = fieldInfos.ttPracticeStartWeekFloorTime;
    time_t weekB = fieldInfos.ttPracticeEndWeekFloorTime;
    time_t weekC = weekB;

    if (m_flMarkersStartDateStr.size() > 0) {
        time_t ttDateC = GetTimeFromString(m_flMarkersStartDateStr);
        weekC = FloorDateToWeekStart(ttDateC);
    }
    if (!flHarvestEvalInfos.ndviPresence) {
        // # no evidence of vegetation in the whole vegetation season (==black fallow) - return STRONG evaluation
        flHarvestEvalInfos.efaIndex = "STRONG";
    } else if (IsNA(harvestConfirmWeek)) {
        flHarvestEvalInfos.efaIndex = "MODERATE";
    } else {
        // # set the first day of the harvest week - evaluate
        time_t harvestConfirmed = ttHarvestConfirmWeekStart;
        if (harvestConfirmed > weekA && harvestConfirmed <= weekC) {
            // # harvest was detected within the fallow period - FALLOW is considered not ok - return WEAK evaluation
            flHarvestEvalInfos.efaIndex = "WEAK";
        } else {
            flHarvestEvalInfos.efaIndex = "MODERATE";
        }
    }
    // # if there is no NDVI value in the whole EFA practice period - return "NA" for the M6
    if (AllNdviMeanAreNA(extractedEfaMarkers)) {
        flHarvestEvalInfos.ndviPresence = NOT_AVAILABLE;
    }
}

void TsaCountryDefaultPlugin::UpdateNFCEfaMarkers(const FieldInfoType &fieldInfos,
                                                  const std::vector<EfaMarkersInfoType> &extractedEfaMarkers,
                                                  HarvestEvaluationInfoType &ncHarvestEvalInfos)
{
    if (!ncHarvestEvalInfos.ndviPresence) {
        // # no evidence of vegetation in the whole vegetation season - no evidence of NFC - return WEAK evaluation
        ncHarvestEvalInfos.efaIndex = "WEAK";
    } else if(IsNA(ncHarvestEvalInfos.harvestConfirmWeek)) {
        // vegetation is present, harvest was not detected - NFC is considered ok - return MODERATE evaluation
        ncHarvestEvalInfos.efaIndex = "MODERATE";
    } else {
        // # set the first day of the harvest week
        //# vegetation is present, harvest was detected - NFC is considered ok - return STRONG evaluation
        ncHarvestEvalInfos.efaIndex = "STRONG";
    }
    // # if there is no NDVI value in the EFA practice period - M6 is NA
    if (AllNdviMeanAreNA(extractedEfaMarkers)) {
        ncHarvestEvalInfos.ndviPresence = NOT_AVAILABLE;
    }
}
