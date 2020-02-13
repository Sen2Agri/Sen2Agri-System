#include "TsaCatchCropAnalysisHandler.h"

#include "TsaHelperFunctions.h"

TsaCatchCropAnalysisHandler::TsaCatchCropAnalysisHandler(itk::Logger* logger)
    : TsaEfaAnalysisBase(logger)
{
}

bool TsaCatchCropAnalysisHandler::PerformAnalysis(const FieldInfoType &fieldInfos,
                                            std::vector<MergedAllValInfosType> &retAllMergedValues,
                                            HarvestEvaluationInfoType &harvestEvalInfos,
                                            HarvestEvaluationInfoType &ccHarvestEvalInfos) {
    time_t ttDateA;     // last possible start of catch-crop period
    time_t ttDateB;     // last possible end of catch-crop period
    time_t ttDateC;
    time_t ttDateD = 0;

    time_t weekA;
    //time_t weekB;
    time_t catchStart;

    ccHarvestEvalInfos = harvestEvalInfos;

    if (harvestEvalInfos.ttPracticeEndTime == 0) {
        // # last possible start of catch-crop period
        ttDateA = harvestEvalInfos.ttPracticeStartTime;
        // # last possible end of catch-crop period
        ttDateB = ttDateA + (m_CatchPeriod - 1) * SEC_IN_DAY;
        weekA = FloorDateToWeekStart(harvestEvalInfos.ttPracticeStartTime);
        //weekB = FloorDateToWeekStart(ttDateB);
        if (IsNA(harvestEvalInfos.harvestConfirmWeek)) {
             // harvest is "NR", set the start of catch-crop period to the last possible start of
            // catch-crop period (to select the NDVI values)
            catchStart = ttDateA;
        } else {
            int lastValidHarvestIdx = -1;
            for (size_t i = 0; i<retAllMergedValues.size(); i++) {
                // after the last possible start of catch-crop period all the weeks with
                // the potential $harvest conditions are set to FALSE
                if (retAllMergedValues[i].ttDate > weekA) {
                    retAllMergedValues[i].harvest = false;
                } else {
                    // select the weeks with potential $harvest conditions (== bare land)
                    if (retAllMergedValues[i].harvest == true) {
                        lastValidHarvestIdx = i;
                    }
                }
            }
            // set the start of catch-crop period to the last potential harvest week  or the harvest week from the harvest evaluation
            if (lastValidHarvestIdx != -1) {
                catchStart = retAllMergedValues[lastValidHarvestIdx].ttDate;
            } else {
                catchStart = harvestEvalInfos.ttHarvestConfirmWeekStart;
            }
        }
        // set the start of the catch-crop (first possible date)
        ttDateC = catchStart;
        time_t ttCatchPeriodStart =  0;
        if (m_CatchPeriodStart.size() > 0) {
            ttCatchPeriodStart = GetTimeFromString(m_CatchPeriodStart);
            if (ttDateC < ttCatchPeriodStart) {
                // a variable can be used to define the earliest date the catch-crop can be sown (???)
                ttDateC = ttCatchPeriodStart;
            }
        }

        // check if the period is covered with data
        if ((retAllMergedValues[retAllMergedValues.size()-1].ttDate - ttDateC) >= (m_CatchPeriod-1) * SEC_IN_DAY) {
            // In the original script, here it was a calculation of the EFA markers
            // but function f.efa.markers is used to get the $ndvi.mean for selected weeks <date.c; date.b>
            const std::vector<MergedAllValInfosType> &filteredAllMergedValues = FilterValuesByDates(
                        retAllMergedValues, ttDateC, ttDateB);
            // if there is any NDVI value
            // select the maximum NDVI
            double minVal, maxVal, efaMin, efaMax;
            // function f.efa.markers is used to get the $ndvi.mean for selected weeks <date.c; date.b>
            if (GetMinMaxNdviValues(filteredAllMergedValues, minVal, maxVal)) {
                efaMax = maxVal;    // select the maximum NDVI
                efaMin = minVal;    // select the minimum NDVI
                if( IsLess(efaMin, m_EfaNdviDown) ){ efaMin = m_EfaNdviDown; }                 // efa.min is >= efa.ndvi.dw
                if( IsGreater(efaMin, m_NdviUp) ){ efaMin = m_EfaNdviUp; }                 // efa.min is <= efa.ndvi.up if efa.min>ndvi.up !
            } else {
                // else set it to efa.ndvi.dw
                efaMax = m_EfaNdviDown;
                efaMin = m_EfaNdviDown;
            }
            double efaChange = (efaMax - efaMin) * m_CatchProportion;                  // compute ndvi buffer
            if( efaChange <10 ) { efaChange = 10; }
            // Date from which the $candidate.efa (potential bare-land conditions) are relevant to compute
            if (!IsNA(harvestEvalInfos.harvestConfirmWeek)) {
                ttDateD = harvestEvalInfos.ttHarvestConfirmWeekStart;
            } else {
                // get first day of the posible harvest weeks
                for (size_t i = 0; i<retAllMergedValues.size(); i++) {
                    if (retAllMergedValues[i].candidateOptical) {
                        ttDateD = retAllMergedValues[i].ttDate;
                        break;
                    }
                }
                if (ttDateD == 0) {
                    ttDateD = ttDateC;
                }
            }
            bool curCatchStart;
            std::vector<int> catchStartIndices;
            // Compute $candidate.efa and define the start of the catch-crop period
            double efaMinChangeVal = efaMin + efaChange;
            for (size_t i = 0; i<retAllMergedValues.size(); i++) {
                retAllMergedValues[i].candidateEfa = (!IsNA(retAllMergedValues[i].ndviNext)) &&
                        IsLess(retAllMergedValues[i].ndviNext, efaMinChangeVal);
                // potential bare-land conditions (based on NDVI)
                if (retAllMergedValues[i].ttDate > weekA) {
                    // after the last possible start of catch-crop period days all the weeks with
                    // the potential bare-land conditions are set to FALSE
                    retAllMergedValues[i].candidateEfa = false;
                }

                // # update
                if (ttCatchPeriodStart > 0) {
                    time_t ttFlooredCatchPeriodStart = FloorDateToWeekStart(ttCatchPeriodStart);
                    if (retAllMergedValues[i].ttDate < ttFlooredCatchPeriodStart) {
                        // after the last possible start of catch-crop period days all the weeks with
                        // the potential bare-land conditions are set to FALSE
                        retAllMergedValues[i].candidateEfa = false;
                    }
                }

                // before the date.d all the weeks with the potential bare-land conditions are set to FALSE
                if (retAllMergedValues[i].ttDate < ttDateD) {
                    retAllMergedValues[i].candidateEfa = false;
                }
                // define the bare-land conditions from all three time-series == breaks
                curCatchStart = retAllMergedValues[i].candidateEfa &&
                        retAllMergedValues[i].candidateAmplitude &&
                        retAllMergedValues[i].candidateCoherence;
                if (curCatchStart) {
                    // weeks with defined breaks
                    catchStartIndices.push_back(i);
                }
            }
            int lastValidCatchStartIdx = -1;
            if (catchStartIndices.size() > 1) {
               int catchStartTest = 0;
               int catchPeriodInSec = m_CatchPeriod * SEC_IN_DAY;
               for(size_t i = 1; i<catchStartIndices.size(); i++) {
                    catchStartTest = retAllMergedValues[catchStartIndices[i]].ttDate - retAllMergedValues[catchStartIndices[i-1]].ttDate;
                    if (catchStartTest >= catchPeriodInSec) {
                        lastValidCatchStartIdx = i - 1;
                    }
                }
            }
            if (lastValidCatchStartIdx == -1 && catchStartIndices.size() > 0) {
                lastValidCatchStartIdx = catchStartIndices.size()-1;
            }

            if (lastValidCatchStartIdx != -1) {
                // last break == week of the most probalbe start of the catch-crop period
                catchStart = retAllMergedValues[catchStartIndices[lastValidCatchStartIdx]].ttDate;
            } else {
                catchStart = ttDateD;
            }
        } else {
            // if the period is not covered with data set the start of catch-crop period to the last possible start of catch-crop period
            catchStart = ttDateA;
        }
        if (m_CatchPeriodStart.size() > 0) {
            // a variable can be used to define the earliest start of the catch-crop period
            if (catchStart < ttCatchPeriodStart) {
                catchStart = ttCatchPeriodStart;
            }
        }
        // set the first day of the catch-crop period
        ccHarvestEvalInfos.ttPracticeStartTime = catchStart;
        // set the last day of the catch-crop period
        ccHarvestEvalInfos.ttPracticeEndTime = catchStart + ((m_CatchPeriod - 1) * SEC_IN_DAY);
    }

    // ### EFA practice period ###
    ttDateA = ccHarvestEvalInfos.ttPracticeStartTime;
    ttDateB = ccHarvestEvalInfos.ttPracticeEndTime;
    weekA = FloorDateToWeekStart(ttDateA);
    //weekB = FloorDateToWeekStart(ttDateB);

    bool efaPeriodStarted = retAllMergedValues[retAllMergedValues.size() - 1].ttDate > weekA;
    //bool efaPeriodEnded = retAllMergedValues[retAllMergedValues.size() - 1].ttDate >= weekB;

//      DEBUG
//        std::cout << TimeToString(weekA) << std::endl;
//        std::cout << TimeToString(weekB) << std::endl;
//        std::cout << TimeToString(retAllMergedValues[retAllMergedValues.size() - 1].ttDate) << std::endl;
//      DEBUG

    // ### BEFORE end of EFA practice period ###
    if (!efaPeriodStarted) {
        // EFA practice is not evaluated before the end of the EFA practice period - return "NR" evaluation
        ccHarvestEvalInfos.efaIndex = "NR";
        ccHarvestEvalInfos.ndviPresence = NR;                        // M6
        ccHarvestEvalInfos.ndviGrowth = NR;                          // M7
        ccHarvestEvalInfos.ndviNoLoss = NR;                          // M8
        ccHarvestEvalInfos.ampNoLoss = NR;                           // M9
        ccHarvestEvalInfos.cohNoLoss = NR;                           // M10
        return true;
    }

    // ### EFA PRACTICE EVALUATION ###

    // is catch-crop grown in/under the main crop on the parcel
    bool catchInMaincrop = ((fieldInfos.practiceType == m_CatchMain) ||
            (fieldInfos.practiceType == m_CatchCropIsMain));
    time_t ttVegSeasonStart = FloorDateToWeekStart(ccHarvestEvalInfos.ttVegStartTime);

//      DEBUG
//        std::cout << TimeToString(ttVegSeasonStart) << std::endl;
//      DEBUG

    // is there any evidence of main crop (>opt.thr.vegcycle) in the vegetation season
    bool vegSeasonStatus = false;
    for (size_t i = 0; i<retAllMergedValues.size(); i++) {
        if (!IsNA(retAllMergedValues[i].vegWeeks) && retAllMergedValues[i].vegWeeks == true &&
                IsGreater(retAllMergedValues[i].ndviMeanVal, m_OpticalThrVegCycle)) {
            vegSeasonStatus = true;
            break;
        }
    }
    if (!vegSeasonStatus) {
        // # no evidence of the main crop vegetation in the vegetation season - evaluation of catch-crop is not relevant - return "NR"
        ccHarvestEvalInfos.efaIndex = "NR";
        ccHarvestEvalInfos.ndviPresence = NR;                        // M6
        ccHarvestEvalInfos.ndviGrowth = NR;                          // M7
        ccHarvestEvalInfos.ndviNoLoss = NR;                          // M8
        ccHarvestEvalInfos.ampNoLoss = NR;                           // M9
        ccHarvestEvalInfos.cohNoLoss = NR;                           // M10
        return true;

    } else if (IsNA(ccHarvestEvalInfos.harvestConfirmWeek)) {
        if (!catchInMaincrop) {
            bool found = false;
            for (size_t i = 0; i<retAllMergedValues.size(); i++) {
                if (retAllMergedValues[i].ttDate >= ttVegSeasonStart && retAllMergedValues[i].ttDate <= weekA) {
                    if (retAllMergedValues[i].ndviDrop == true) {
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                ccHarvestEvalInfos.efaIndex = "POOR";
                ccHarvestEvalInfos.ndviPresence = NR;                        // M6
                ccHarvestEvalInfos.ndviGrowth = NR;                          // M7
                ccHarvestEvalInfos.ndviNoLoss = NR;                          // M8
                ccHarvestEvalInfos.ampNoLoss = NR;                           // M9
                ccHarvestEvalInfos.cohNoLoss = NR;                           // M10
                return true;
            } //else {
                // harvest was not detected but there is "drop" in NDVI before the catch-crop period - evaluate
            // }
        } //else {
            // harvest was not detected but catch-crop was sown in/under the main crop - evaluate
        // }
    } else {
        if (ccHarvestEvalInfos.ttHarvestConfirmWeekStart > weekA && !catchInMaincrop) {
            ccHarvestEvalInfos.efaIndex = "POOR";
            ccHarvestEvalInfos.ndviPresence = NR;                        // M6
            ccHarvestEvalInfos.ndviGrowth = NR;                          // M7
            ccHarvestEvalInfos.ndviNoLoss = NR;                          // M8
            ccHarvestEvalInfos.ampNoLoss = NR;                           // M9
            ccHarvestEvalInfos.cohNoLoss = NR;                           // M10
            return true;
        }
    }

    return CCEfaMarkersExtraction(ttDateA, ttDateB, weekA, fieldInfos, ccHarvestEvalInfos);
}

bool TsaCatchCropAnalysisHandler::CCEfaMarkersExtraction(time_t ttDateA, time_t ttDateB, time_t weekA,
                         const FieldInfoType &fieldInfos,
                         HarvestEvaluationInfoType &ccHarvestEvalInfos) {
    // get efa markers for the defined catch-crop period
    std::vector<EfaMarkersInfoType> efaMarkers;
    ExtractEfaMarkers(ttDateA, ttDateB, fieldInfos, efaMarkers);

    // ### MARKER 6 - efa.presence.ndvi ###
    // # if there is evidence of NDVI>thr in the catch-crop period - M6 is TRUE
    EfaMarkersInfoType efa;
    efa.ndviPresence = IsNdviPresence(efaMarkers);

    // # if there is no NDVI value from third week of EFA practice period and M6 is FALSE,
    //   set the M6 to NA (low values are allowed at the beginning of the period)
    if (!efa.ndviPresence) {
        bool allNdviNA = true;
        for(size_t i = 0; i<efaMarkers.size(); i++) {
            if (efaMarkers[i].ttDate >= weekA + 21 * SEC_IN_DAY) {
                if (!IsNA(efaMarkers[i].ndviMean)) {
                    allNdviNA = false;
                    break;
                }
            }
        }
        if (allNdviNA) {
            efa.ndviPresence = NOT_AVAILABLE;
        }
    }

    // ### MARKER 7 - efa.growth.ndvi ###
    // # if all the NDVI are >thr from third week of EFA practice period - M7 is TRUE
    bool allNdviGrowthOK = true;
    bool allNdviGrowthNA = true;
    for(size_t i = 0; i<efaMarkers.size(); i++) {
        if (efaMarkers[i].ttDate >= weekA + 21 * SEC_IN_DAY) {
            if (!IsNA(efaMarkers[i].ndviGrowth)) {
                allNdviGrowthNA = false;
                if (efaMarkers[i].ndviGrowth == false) {
                    allNdviGrowthOK = false;
                    break;
                }
            }
        }
    }
    efa.ndviGrowth = allNdviGrowthOK;

    if (IsNA(efa.ndviPresence) || allNdviGrowthNA) {
        efa.ndviGrowth = NOT_AVAILABLE;
    }

    // ### MARKER 8 - efa.noloss.ndvi ###
    if (IsNA(efa.ndviPresence)) {
        efa.ndviNoLoss = NOT_AVAILABLE;
    } else {
        bool allNoLossNA = true;
        bool allNoLossTrue = true;
        for(size_t i = 0; i<efaMarkers.size(); i++) {
            if (!IsNA(efaMarkers[i].ndviNoLoss)) {
                allNoLossNA = false;
                if (efaMarkers[i].ndviNoLoss == false) {
                    allNoLossTrue = false;
                    break;
                }
            }
        }
        if (allNoLossNA) {
            efa.ndviNoLoss = NOT_AVAILABLE;
        } else {
            efa.ndviNoLoss = allNoLossTrue;
        }
    }

    // ### NDVI markers evaluation ###
    // # evaluate the NDVI markers (M6,M7,M8)
    short efaNdvi;
    if (IsNA(efa.ndviPresence) && IsNA(efa.ndviGrowth) && IsNA(efa.ndviNoLoss)) {
        efaNdvi = NOT_AVAILABLE;
    } else {
        // if not all flags are true
        efaNdvi = true;
        if(efa.ndviPresence == false || efa.ndviNoLoss == false || efa.ndviGrowth == false) {
            efaNdvi = false;
        }
    }

    // ### MARKER 9 - backscatter no loss - efa.grd ###
    // # if there is no loss in backscatter - M9 is TRUE
    bool allAmpLossNA = true;
    bool allAmpLossTrue = true;
    for(size_t i = 0; i<efaMarkers.size(); i++) {
        if (!IsNA(efaMarkers[i].ampNoLoss)) {
            allAmpLossNA = false;
            if (efaMarkers[i].ampNoLoss == false) {
                allAmpLossTrue = false;
                break;
            }
        }
    }
    efa.ampNoLoss = allAmpLossNA ? NOT_AVAILABLE : allAmpLossTrue;

    // ### MARKER 10 - coherence no loss - efa.coh ###
    // # if there is no loss in coherence from third week of the EFA practice period - M10 is TRUE
    bool allCohLossTrue = true;
    for(size_t i = 0; i<efaMarkers.size(); i++) {
        if (efaMarkers[i].ttDate >= weekA + 14 * SEC_IN_DAY) {
            if (!IsNA(efaMarkers[i].cohNoLoss)) {
                if (efaMarkers[i].cohNoLoss == false) {
                    allCohLossTrue = false;
                    break;
                }
            }
        }
    }
    efa.cohNoLoss = allCohLossTrue;

    // number the FALSE markers
    int efaNotCompliant = 0;
    efaNotCompliant += (efa.ndviPresence == false ? 1 : 0);
    efaNotCompliant += (efa.ndviGrowth == false ? 1 : 0);
    efaNotCompliant += (efa.ndviNoLoss == false ? 1 : 0);
    efaNotCompliant += (efa.ampNoLoss == false ? 1 : 0);
    efaNotCompliant += (efa.cohNoLoss == false ? 1 : 0);

    // # no NDVI values in the catch-crop perid - evaluation is based only on SAR markers (M9,M10)
    if (IsNA(efaNdvi)) {
        if (efaNotCompliant == 0) {
            ccHarvestEvalInfos.efaIndex = "MODERATE";
        } else {
            ccHarvestEvalInfos.efaIndex = "WEAK";
        }
    } else {
        switch (efaNotCompliant) {
            case 0:
                ccHarvestEvalInfos.efaIndex = "STRONG";
                break;
            case 1:
                ccHarvestEvalInfos.efaIndex = "MODERATE";
                break;
            case 2:
                ccHarvestEvalInfos.efaIndex = "WEAK";
                break;
            default:
                ccHarvestEvalInfos.efaIndex = "POOR";
                break;
        }
    }
    ccHarvestEvalInfos.ndviPresence = efa.ndviPresence;
    ccHarvestEvalInfos.ndviGrowth = efa.ndviGrowth;
    ccHarvestEvalInfos.ndviNoLoss = efa.ndviNoLoss;
    ccHarvestEvalInfos.ampNoLoss = efa.ampNoLoss;
    ccHarvestEvalInfos.cohNoLoss = efa.cohNoLoss;

    return true;
}
