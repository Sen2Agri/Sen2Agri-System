#include "TsaFallowAnalysisHandler.h"

#include "TsaHelperFunctions.h"

TsaFallowAnalysisHandler::TsaFallowAnalysisHandler(itk::Logger* logger)
    : TsaEfaAnalysisBase(logger)
{
}

bool TsaFallowAnalysisHandler::PerformAnalysis(const FieldInfoType &fieldInfos,
                                            std::vector<MergedAllValInfosType> &retAllMergedValues,
                                            HarvestEvaluationInfoType &harvestInfos,
                                            HarvestEvaluationInfoType &flHarvestEvalInfos) {
    flHarvestEvalInfos = harvestInfos;

    flHarvestEvalInfos.ndviPresence = NR;                        // M6
    flHarvestEvalInfos.ndviGrowth = NR;                          // M7
    flHarvestEvalInfos.ndviNoLoss = NR;                          // M8
    flHarvestEvalInfos.ampNoLoss = NR;                           // M9
    flHarvestEvalInfos.cohNoLoss = NR;                           // M10

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
        flHarvestEvalInfos.efaIndex = "NR";
        return true;
    }

    // ### EFA PRACTICE EVALUATION ###
    // get efa markers for the defined catch-crop period
    std::vector<EfaMarkersInfoType> efaMarkers;
    ExtractEfaMarkers(ttDateA, ttDateB, fieldInfos, efaMarkers);

    // ### MARKER 6 - efa.presence.ndvi ###
    // # if there is evidence of vegetation (NDVI>efa.ndvi.thr) in the EFA practice period - M6 is TRUE
    flHarvestEvalInfos.ndviPresence = IsNdviPresence(efaMarkers);

    if (fieldInfos.countryCode == "LTU") {
        // for Lithuania there is a special case
        bool efaCoh = false;
        //time_t ttCohStableWeek = 0;
        if (efaMarkers.size() >= 9) {
            // is there loss in coherence (FALSE value) in 9 subsequent weeks
            int cohStableCnt = 0;
            for (int i = (efaMarkers.size() - 1); i>= 0; i--) {
                if (efaMarkers[i].cohNoLoss == true) {
                    cohStableCnt++;
                    if (cohStableCnt >= 9) {
                        efaCoh = true;
                        //ttCohStableWeek = efaMarkers[i].ttDate;
                        break;
                    }
                } else {
                    cohStableCnt = 0;
                }
            }
        }
        if (fieldInfos.practiceType == "PDJ") {
            if (!efaCoh) {
                flHarvestEvalInfos.efaIndex = "STRONG";
                // # harvest is not evaluated for the confirmed black fallow as it is not expected
                harvestInfos.harvestConfirmWeek = NR;
                harvestInfos.ttHarvestConfirmWeekStart = NR;
                flHarvestEvalInfos.harvestConfirmWeek = NR;
                flHarvestEvalInfos.ttHarvestConfirmWeekStart = NR;
            } else {
                // # if there is no-loss of coherence in all the 9 subsequent
                // weeks (if M10 is TRUE) - return WEAK evaluation for the black fallow practice
                flHarvestEvalInfos.efaIndex = "WEAK";

                // NOTE: Code Just for testing in the R code.
                //flHarvestInfos.ttPracticeStartTime = ttCohStableWeek;
                //flHarvestInfos.ttPracticeEndTime = ttCohStableWeek + 62 * SEC_IN_DAY;
            }
            // M6 is not used - return "NR"
            flHarvestEvalInfos.ndviPresence = NR;
        } else if (fieldInfos.practiceType == "PDZ") {
            // ### GREEN-FALLOW EVALUATION ###
            if (!efaCoh) {
                // # for the green fallow practice no-loss of coherence in at least 9 subsequent weeks is expected
                // # if there is a loss in all the 9 subsequent weeks (if M10 is FALSE) - return WEAK evaluation
                // # in such case harvest is not evaluated
                flHarvestEvalInfos.efaIndex = "WEAK";
                //flHarvestInfos.harvestConfirmWeek = NR;
                //flHarvestInfos.ttHarvestConfirmWeekStart = NR;
            } else if (flHarvestEvalInfos.ndviPresence == false) {
                // # if there is no evidence of vegetation in NDVI - return WEAK evaluation for the green fallow practice
                // # green fallow shall be inserted to soil up to the end of the efa period - harvest shall be detected
                // # if harvest is not detected - return WEAK evaluation
                flHarvestEvalInfos.efaIndex = "WEAK";
            } else if (IsNA(flHarvestEvalInfos.harvestConfirmWeek)) {
                flHarvestEvalInfos.efaIndex = "MODERATE";
            } else {
                // # if harvest is detected
                // # set the first day of the harvest week
                if (flHarvestEvalInfos.ttHarvestConfirmWeekStart <= (weekB - 62 * SEC_IN_DAY)) {
                    // # too early - return MODERATE evaluation
                    flHarvestEvalInfos.efaIndex = "MODERATE";
                } else if (flHarvestEvalInfos.ttHarvestConfirmWeekStart <= (weekB + 7 * SEC_IN_DAY)) {
                    // # before the end of the efa period - return STRONG evaluation
                    flHarvestEvalInfos.efaIndex = "STRONG";
                } else {
                    // # too late - return WEAK evaluation
                    flHarvestEvalInfos.efaIndex = "MODERATE";
                }
            }

            // # if there is no NDVI value in the whole EFA practice period - return "NA" for the M6
            if (AllNdviMeanAreNA(efaMarkers)) {
                flHarvestEvalInfos.ndviPresence = NOT_AVAILABLE;
            }
        }
        flHarvestEvalInfos.cohNoLoss = efaCoh;                       // M10
    } else {
        if (fieldInfos.countryCode == "CZE") {
            time_t ttDateC = GetTimeFromString(m_flMarkersStartDateStr);
            time_t ttDateD = GetTimeFromString(m_flMarkersEndDateStr);
            time_t weekC = FloorDateToWeekStart(ttDateC);
            time_t weekD = FloorDateToWeekStart(ttDateD);

            if (!flHarvestEvalInfos.ndviPresence) {
                // # no evidence of GREEN FALLOW vegetation in the EFA practice period - return POOR evaluation
                flHarvestEvalInfos.efaIndex = "POOR";
                return true;

            } else if (!IsNA(harvestInfos.harvestConfirmWeek)) {
                // # set the first day of the harvest week - evaluate
                time_t harvestConfirmed = harvestInfos.ttHarvestConfirmWeekStart;
                if (harvestConfirmed > weekA && harvestConfirmed <= weekB) {
                    // # harvest was detected within the fallow period - FALLOW is considered not ok - return WEAK evaluation
                    flHarvestEvalInfos.efaIndex = "WEAK";
                    return true;
                }
            } // else {
                    // # vegetation is present, harvest was not detected - FALLOW is considered ok - evaluate
              // }

            // ### MARKER 8 - efa.noloss.ndvi ###
            // # mowing or mulching is required between "2017-06-01" and "2017-08-31" - efa.noloss.ndvi shall be FALSE in this period
            std::vector<EfaMarkersInfoType> efaMarkers2;
            // # get efa markers for the specified period & buffer
            ExtractEfaMarkers(ttDateC - 30 * SEC_IN_DAY, ttDateD + 30 * SEC_IN_DAY, fieldInfos, efaMarkers2);
            for (int i = (int)efaMarkers2.size() - 2; i>=0; i--) {
                if (IsNA(efaMarkers2[i].ndviNoLoss) && !IsNA(efaMarkers2[i+1].ndviNoLoss)) {
                    efaMarkers2[i].ndviNoLoss = efaMarkers2[i+1].ndviNoLoss;
                }
            }

            // # if there is evidence of loss in NDVI in the period - M8(noloss) is FALSE
            bool ndviNoLossFalseFound = false;
            for (size_t i = 0; i<efaMarkers2.size(); i++) {
                if (IsNA(efaMarkers2[i].ndviNoLoss)) {
                    continue;
                }
                if ((efaMarkers2[i].ttDate >= weekC) && (efaMarkers2[i].ttDate <= (weekD + SEC_IN_WEEK))) {
                    if (efaMarkers2[i].ndviNoLoss == false) {
                        ndviNoLossFalseFound = true;
                        break;
                    }
                }
            }
            flHarvestEvalInfos.ndviNoLoss = !ndviNoLossFalseFound; // M8
            if (!flHarvestEvalInfos.ndviNoLoss) {
                flHarvestEvalInfos.efaIndex = "STRONG";
            } else {
                flHarvestEvalInfos.efaIndex = "MODERATE";
            }
        } else {
            // ITA or ESP
            time_t weekC = weekB;
            if (CheckCountryCode(fieldInfos.countryCode, "ITA")) {
                time_t ttDateC = GetTimeFromString(m_flMarkersStartDateStr);
                weekC = FloorDateToWeekStart(ttDateC);
            }
            if (!flHarvestEvalInfos.ndviPresence) {
                // # no evidence of vegetation in the whole vegetation season (==black fallow) - return STRONG evaluation
                flHarvestEvalInfos.efaIndex = "STRONG";
            } else if (IsNA(harvestInfos.harvestConfirmWeek)) {
                flHarvestEvalInfos.efaIndex = "MODERATE";
            } else {
                // # set the first day of the harvest week - evaluate
                time_t harvestConfirmed = harvestInfos.ttHarvestConfirmWeekStart;
                if (harvestConfirmed > weekA && harvestConfirmed <= weekC) {
                    // # harvest was detected within the fallow period - FALLOW is considered not ok - return WEAK evaluation
                    flHarvestEvalInfos.efaIndex = "WEAK";
                } else {
                    flHarvestEvalInfos.efaIndex = "MODERATE";
                }
            }
            // # if there is no NDVI value in the whole EFA practice period - return "NA" for the M6
            if (AllNdviMeanAreNA(efaMarkers)) {
                flHarvestEvalInfos.ndviPresence = NOT_AVAILABLE;
            }
        }
    }
    return true;
}

