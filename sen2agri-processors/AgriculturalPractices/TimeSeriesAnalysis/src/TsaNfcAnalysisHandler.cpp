#include "TsaNfcAnalysisHandler.h"

#include "TsaHelperFunctions.h"

TsaNfcAnalysisHandler::TsaNfcAnalysisHandler(itk::Logger* logger)
    : TsaEfaAnalysisBase(logger)
{
}

bool TsaNfcAnalysisHandler::PerformAnalysis(const FieldInfoType &fieldInfos,
                                            std::vector<MergedAllValInfosType> &retAllMergedValues,
                                            HarvestEvaluationInfoType &harvestInfos,
                                            HarvestEvaluationInfoType &ncHarvestEvalInfos) {
    ncHarvestEvalInfos = harvestInfos;

    ncHarvestEvalInfos.ndviPresence = NR;                        // M6
    ncHarvestEvalInfos.ndviGrowth = NR;                          // M7
    ncHarvestEvalInfos.ndviNoLoss = NR;                          // M8
    ncHarvestEvalInfos.ampNoLoss = NR;                           // M9
    ncHarvestEvalInfos.cohNoLoss = NR;                           // M10

    time_t ttDateA = fieldInfos.ttPracticeStartTime;     // last possible start of catch-crop period
    time_t ttDateB = fieldInfos.ttPracticeEndTime;     // last possible end of catch-crop period
    time_t weekA = fieldInfos.ttPracticeStartWeekFloorTime;
    time_t weekB = fieldInfos.ttPracticeEndWeekFloorTime;

    time_t lastDate = retAllMergedValues[retAllMergedValues.size()-1].ttDate;
    bool efaPeriodStarted = (lastDate > weekA);
    //bool efaPeriodEnded = (lastDate >= weekB);

    // ### BEFORE end of EFA practice period ###
    if (!efaPeriodStarted) {
        // # EFA practice is not evaluated before the end of the EFA practice period - return "NR" evaluation
        ncHarvestEvalInfos.efaIndex = "NR";
        return true;
    }

    // ### EFA PRACTICE EVALUATION ###
    // get efa markers for the defined catch-crop period
    time_t vegSeasonStart = (fieldInfos.countryCode == "CZE" ?
                FloorDateToWeekStart(harvestInfos.ttVegStartTime):
                ttDateA);
    std::vector<EfaMarkersInfoType> efaMarkers;
    ExtractEfaMarkers(vegSeasonStart, ttDateB, fieldInfos, efaMarkers);

    // ### MARKER 6 - efa.presence.ndvi ###
    // # if there is evidence of vegetation (NDVI>efa.ndvi.thr) in the vegetation season - M6 is TRUE
    ncHarvestEvalInfos.ndviPresence = IsNdviPresence(efaMarkers);

    if (fieldInfos.countryCode != "CZE") {
        if (!ncHarvestEvalInfos.ndviPresence) {
            // # no evidence of vegetation in the whole vegetation season - no evidence of NFC - return WEAK evaluation
            if (fieldInfos.countryCode == "ESP" || CheckCountryCode(fieldInfos.countryCode, "ITA")) {
                ncHarvestEvalInfos.efaIndex = "POOR";
            } else {
                ncHarvestEvalInfos.efaIndex = "WEAK";
            }
        } else if(IsNA(ncHarvestEvalInfos.harvestConfirmWeek)) {
            // vegetation is present, harvest was not detected - NFC is considered ok - return MODERATE evaluation
            ncHarvestEvalInfos.efaIndex = "MODERATE";
        } else {
            // # set the first day of the harvest week
            //# vegetation is present, harvest was detected - NFC is considered ok - return STRONG evaluation
            if ((fieldInfos.countryCode == "ROU") &&
                    (harvestInfos.ttHarvestConfirmWeekStart > weekA &&
                    harvestInfos.ttHarvestConfirmWeekStart < weekB)) {
                ncHarvestEvalInfos.efaIndex = "WEAK";
            } else {
                ncHarvestEvalInfos.efaIndex = "STRONG";
            }
        }
        // # if there is no NDVI value in the EFA practice period - M6 is NA
        if (AllNdviMeanAreNA(efaMarkers)) {
            ncHarvestEvalInfos.ndviPresence = NOT_AVAILABLE;
        }
        if (fieldInfos.countryCode == "ROU") {
            if (IsNA(ncHarvestEvalInfos.ndviPresence)) {
                ncHarvestEvalInfos.efaIndex = "NR";
            }
        }
    } else {
        if (!ncHarvestEvalInfos.ndviPresence) {
            // # no evidence of the NFC vegetation in the vegetation season - return POOR evaluation
            ncHarvestEvalInfos.efaIndex = "POOR";
            return true;

        } else if (!IsNA(harvestInfos.harvestConfirmWeek)) {
            // # set the first day of the harvest week - evaluate
            time_t harvestConfirmed = harvestInfos.ttHarvestConfirmWeekStart;
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
    return true;
}
