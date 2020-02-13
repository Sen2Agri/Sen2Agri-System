#include "TsaHarvestOnlyAnalysisHandler.h"

#include "TimeSeriesAnalysisTypes.h"
#include "TimeSeriesAnalysisUtils.h"
#include "TsaHelperFunctions.h"

#include "boost/filesystem.hpp"

#define SEC_IN_5_WEEKS                  3024000 // 5 weeks * 7 days * seconds in day

TsaHarvestOnlyAnalysisHandler::TsaHarvestOnlyAnalysisHandler(itk::Logger* logger)
    : m_pLogger(logger)
{
    m_bShortenVegWeeks = false;
}

void TsaHarvestOnlyAnalysisHandler::SetPrevPracticeFileName(const std::string &prevPrdDir, const std::string &prevFileName) {
    boost::filesystem::path rootFolder(prevPrdDir);
    m_prevPrdReader.Initialize((rootFolder / prevFileName).string());
}

void TsaHarvestOnlyAnalysisHandler::TsaHarvestOnlyAnalysisHandler::PerformHarvestEvaluation(
        FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &allMergedValues,
        HarvestEvaluationInfoType &harvestInfos)
{
    CheckVegetationStart(fieldInfos, allMergedValues);

    UpdateMarker1Infos(fieldInfos, allMergedValues);
    UpdateMarker2Infos(fieldInfos, allMergedValues);
    UpdateMarker5Infos(fieldInfos, allMergedValues);
    double ampThrValue;
    UpdateMarker3Infos(fieldInfos, allMergedValues, ampThrValue);
    UpdateMarker4Infos(fieldInfos, allMergedValues, ampThrValue);

    // DEBUG
    m_debugPrinter.PrintMergedValues(allMergedValues, ampThrValue);
    // DEBUG

    HarvestEvaluation(fieldInfos, allMergedValues, harvestInfos);

    // update the gaps information
    UpdateGapsInformation(allMergedValues, fieldInfos, harvestInfos);

//      DEBUG
    m_debugPrinter.PrintHarvestEvaluation(fieldInfos, harvestInfos);
//      DEBUG
}


void TsaHarvestOnlyAnalysisHandler::CheckVegetationStart(FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues) {
    // # to avoid gap in vegseason.start week
    // if (length(which(group.merge$Group.1==vegseason.start)==vegseason.start)==0) {
    //    vegseason.start <- min(group.merge[which(group.merge$Group.1>=vegseason.start),]$Group.1);
    //    base.info$VEG_START <- vegseason.start;
    // }
    for(size_t i = 0; i<retAllMergedValues.size(); i++) {
        if (retAllMergedValues[i].ttDate == fieldInfos.ttVegStartWeekFloorTime) {
            break;
        } else if (retAllMergedValues[i].ttDate > fieldInfos.ttVegStartWeekFloorTime) {
            // Get the first date that is greater than ttVegStartWeekFloorTime if we did not had equality
            // we assume that the dates are already sorted in cronological order
            fieldInfos.ttVegStartWeekFloorTime = retAllMergedValues[i].ttDate;
            fieldInfos.ttVegStartTime = retAllMergedValues[i].ttDate;
            break;
        }
    }
}
void TsaHarvestOnlyAnalysisHandler::UpdateMarker1Infos(FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues) {
    // # to avoid gap in vegseason.start week
    bool bVegStartFound = false;
    int minVegStartDateIdx = -1;
    for(size_t i = 0; i<retAllMergedValues.size(); i++) {
        if (retAllMergedValues[i].ttDate == fieldInfos.ttVegStartWeekFloorTime) {
            bVegStartFound = true;
            break;
        } else if (retAllMergedValues[i].ttDate > fieldInfos.ttVegStartWeekFloorTime) {
            // get the first date greater than veg start
            minVegStartDateIdx = i;
            break;
        }
    }
    if (!bVegStartFound && minVegStartDateIdx > 0) {
        fieldInfos.ttVegStartTime = retAllMergedValues[minVegStartDateIdx].ttDate;
        fieldInfos.ttVegStartWeekFloorTime = retAllMergedValues[minVegStartDateIdx].ttDate;
    }

    // MARKER 1  Presence of vegetation cycle (NDVI): M1 == $ndvi.presence
    // Define weeks in vegetation season
    for(size_t i = 0; i<retAllMergedValues.size(); i++) {
        if (retAllMergedValues[i].ttDate >= fieldInfos.ttVegStartWeekFloorTime &&
            retAllMergedValues[i].ttDate <= fieldInfos.ttHarvestEndWeekFloorTime) {
            retAllMergedValues[i].vegWeeks = true;
        }
    }
    if (m_bShortenVegWeeks && (fieldInfos.ttVegStartWeekFloorTime < fieldInfos.ttPracticeStartWeekFloorTime)) {
        time_t ttPractStartNextWeek = fieldInfos.ttPracticeStartWeekFloorTime + SEC_IN_WEEK;
        for(size_t i = 0; i<retAllMergedValues.size(); i++) {
            if (retAllMergedValues[i].ttDate >= fieldInfos.ttVegStartWeekFloorTime &&
                retAllMergedValues[i].ttDate <= ttPractStartNextWeek) {
                retAllMergedValues[i].vegWeeks = true;
            }
        }
    }

    // if( length(which(group.merge$Group.1==harvest.to ))==0  ){harvest.to <- group.merge[nrow(group.merge),]$Group.1}
    bool bHarvestEndFound = false;
    for(size_t i = 0; i<retAllMergedValues.size(); i++) {
        if (retAllMergedValues[i].ttDate == fieldInfos.ttHarvestEndWeekFloorTime) {
            bHarvestEndFound = true;
            break;
        }
    }
    if (!bHarvestEndFound) {
        fieldInfos.ttHarvestEndWeekFloorTime = retAllMergedValues[retAllMergedValues.size()-1].ttDate;
    }

    // Define presence of NDVI (M1) for vegetation season
    for(size_t i = 0; i<retAllMergedValues.size(); i++) {
        if (retAllMergedValues[i].ttDate >= fieldInfos.ttVegStartWeekFloorTime &&
            retAllMergedValues[i].ttDate <= fieldInfos.ttHarvestEndWeekFloorTime) {
            if (IsNA(retAllMergedValues[i].ndviMeanVal)) {
                retAllMergedValues[i].ndviPresence = false;
            } else if (IsGreater(retAllMergedValues[i].ndviMeanVal, m_OpticalThrVegCycle)) {
                // here we must set all the values from i to the date of Harvest End to true
                for (size_t j = i; j < retAllMergedValues.size() && retAllMergedValues[j].ttDate <= fieldInfos.ttHarvestEndWeekFloorTime; j++) {
                    retAllMergedValues[j].ndviPresence = true;
                }
                break;  // exit now, all needed entries were processed
            } else {
                retAllMergedValues[i].ndviPresence = false;
            }
        }
    }
}

void TsaHarvestOnlyAnalysisHandler::UpdateMarker2Infos(const FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues) {
    double prevValidNdviMeanVal = NOT_AVAILABLE;

    bool allVwstNdviDw = true; // all vegetation weeks smaller than NDVI down
    bool ndviDropMeanLessDw = false; // at least one NDVI mean value corresponding to a drop is less than NDVI down
    bool ndviDropMeanLessUp = false; // at least one NDVI mean value corresponding to a drop is less than NDVI up
    double minMeanNdviDropVal = NOT_AVAILABLE;
    double maxMeanNdviDropVal = NOT_AVAILABLE;
    int nVegWeeksCnt = 0;
    bool bHasNdviDrop = false;

    for(size_t i = 0; i<retAllMergedValues.size(); i++) {
        retAllMergedValues[i].ndviDrop = NOT_AVAILABLE;
        if (IsNA(retAllMergedValues[i].ndviMeanVal)) {
            continue;
        }
        if (retAllMergedValues[i].vegWeeks == true) {
            if (retAllMergedValues[i].ndviPresence == true) {
                nVegWeeksCnt++;
            }

            if ((IsNA(retAllMergedValues[i].ndviPresence) || retAllMergedValues[i].ndviPresence == true) && !IsNA(prevValidNdviMeanVal)) {
                // update the NDVI drop only to the ones that have NDVI presence true
                retAllMergedValues[i].ndviDrop = (IsLess(retAllMergedValues[i].ndviMeanVal, prevValidNdviMeanVal) &&
                        IsLess(retAllMergedValues[i].ndviMeanVal, m_NdviUp));

                // compute additional infos for computing opt.thr.value
                if (retAllMergedValues[i].ndviDrop) {
                    bHasNdviDrop = true;
                    if (IsLess(retAllMergedValues[i].ndviMeanVal, m_NdviDown)) {
                        ndviDropMeanLessDw = true;
                    }
                    if (IsLess(retAllMergedValues[i].ndviMeanVal, m_NdviUp)) {
                        ndviDropMeanLessUp = true;
                    }
                    if (IsNA(minMeanNdviDropVal)) {
                        minMeanNdviDropVal = retAllMergedValues[i].ndviMeanVal;
                    } else {
                        if (IsGreater(minMeanNdviDropVal, retAllMergedValues[i].ndviMeanVal)) {
                            minMeanNdviDropVal = retAllMergedValues[i].ndviMeanVal;
                        }
                    }
                }
            }
            // save the previous valid ndvi mean value that has veg.weeks = true needed for ndvi.drop calculation
            prevValidNdviMeanVal = retAllMergedValues[i].ndviMeanVal;

            // compute the maximum ndvi drop needed for opt.thr.value computation
            if (IsGreaterOrEqual(retAllMergedValues[i].ndviMeanVal, m_NdviDown)) {
                allVwstNdviDw = false;
            }
            if (IsNA(maxMeanNdviDropVal)) {
                maxMeanNdviDropVal = retAllMergedValues[i].ndviMeanVal;
            } else {
                if (IsLess(maxMeanNdviDropVal, retAllMergedValues[i].ndviMeanVal)) {
                    maxMeanNdviDropVal = retAllMergedValues[i].ndviMeanVal;
                }
            }
        }
//          DEBUG
//            std::cout<< i+1 << " " << ValueToString(retAllMergedValues[i].ndviDrop, true) << std::endl;
//          DEBUG
    }

    // Compute the opt.thr.value
    double OpticalThresholdValue;
    if (allVwstNdviDw || ndviDropMeanLessDw) {
        OpticalThresholdValue = m_NdviDown;
    } else if (ndviDropMeanLessUp) {
        OpticalThresholdValue = m_ndviStep * std::ceil(minMeanNdviDropVal / m_ndviStep);
    } else {
        OpticalThresholdValue = m_NdviUp;
    }

    double OpticalThresholdBuffer = ComputeOpticalThresholdBuffer(nVegWeeksCnt, maxMeanNdviDropVal, OpticalThresholdValue);

    // Define start of harvest period based on optical data (harvest.opt.start)
    time_t harvestOpticalStart = 0;
    double optThrValBuf = OpticalThresholdValue + OpticalThresholdBuffer;
    for(size_t i = 0; i<retAllMergedValues.size(); i++) {
        if ((!IsNA(retAllMergedValues[i].ndviNext)) &&
                IsLess(retAllMergedValues[i].ndviNext, optThrValBuf) &&
                IsGreater(retAllMergedValues[i].ndviPrev, OpticalThresholdValue) &&
                IsGreater(retAllMergedValues[i].ndviPrev, retAllMergedValues[i].ndviNext) &&
                (retAllMergedValues[i].ttDate >= fieldInfos.ttHarvestStartWeekFloorTime)) {
            harvestOpticalStart = retAllMergedValues[i].ttDate;
            break;
        }
    }
    if (harvestOpticalStart == 0) {
        harvestOpticalStart = fieldInfos.ttHarvestStartWeekFloorTime;
    }
//          DEBUG
//        std::cout << TimeToString(fieldInfos.ttHarvestStartWeekFloorTime) << std::endl;
//        std::cout << TimeToString(harvestOpticalStart) << std::endl;
//          DEBUG

    // Define candidate-weeks for harvest based on optical data (M2)
    if (bHasNdviDrop) {
        for(size_t i = 0; i<retAllMergedValues.size(); i++) {
            if (!IsNA(retAllMergedValues[i].ndviNext) && !IsNA(retAllMergedValues[i].ndviPrev)) {
                if ((IsLess(retAllMergedValues[i].ndviNext, optThrValBuf) ||
                     IsLess(retAllMergedValues[i].ndviPrev, optThrValBuf)) &&
                        IsGreater(retAllMergedValues[i].ndviPrev, m_OpticalThresholdMinimum) &&
                        (retAllMergedValues[i].ttDate >= harvestOpticalStart) &&
                        IsGreater((retAllMergedValues[i].ndviPrev + m_NdviDown), retAllMergedValues[i].ndviNext)) {
                    retAllMergedValues[i].candidateOptical = true;
                }
            }
        }
    }
}


void TsaHarvestOnlyAnalysisHandler::UpdateMarker3Infos(const FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues,
                    double &ampThrValue) {
    std::vector<int> ampOutlier;
    std::map<time_t, int> mapCnts;
    std::map<time_t, double> mapSums;
    ampOutlier.resize(fieldInfos.ampVVLines.size());
    for(size_t i = 0; i < fieldInfos.ampVVLines.size(); i++) {
        // for backscatter outliars VV < -3 dB  (draft soulution)
        ampOutlier[i] = (fieldInfos.ampVVLines[i].meanVal > -3) ? 1 : 0;
    }
    // first extract the sum and counts for each date
    for(size_t i = 0; i < fieldInfos.ampVVLines.size(); i++) {
        if (ampOutlier[i] == 0) {
            mapCnts[fieldInfos.ampVVLines[i].ttDateFloor]++;
            mapSums[fieldInfos.ampVVLines[i].ttDateFloor] += fieldInfos.mergedAmpInfos[i].ampRatio;
        }
    }
    // Compute the mean for the filtered amplitudes
    std::vector<double> meanAmpValsFiltered;
    for( std::map<time_t, double>::iterator it = mapSums.begin(); it != mapSums.end(); ++it ) {
        int cntVal = mapCnts[it->first];
        if (cntVal > 0) {
            meanAmpValsFiltered.push_back(it->second / cntVal);
        }
    }
//          DEBUG
//        for (int i = 0; i<meanAmpValsFiltered.size(); i++) {
//            std::cout << meanAmpValsFiltered[i] << std::endl;
//        }
//          DEBUG
    // Compute the standard deviation
    double meanVal = 0;
    double stdDevVal = 0;
    // Compute Sample Standard Deviation instead of Population Standard Deviation
    // TODO: To be confirmed by Gisat
    ComputeMeanAndStandardDeviation(meanAmpValsFiltered, meanVal, stdDevVal, true);

//        grd.thr.break <- paste("grd.thr.break <- sd(grd.filtered)/6")
//        grd.thr.value <- paste("grd.thr.value <- mean(grd.filtered)-sd(grd.filtered)/2")
    double ampThrBreak = ComputeAmplitudeThresholdBreak(stdDevVal);
    ampThrValue = ComputeAmplitudeThresholdValue(meanVal, stdDevVal);
    for(size_t i = 0; i<retAllMergedValues.size(); i++) {
        retAllMergedValues[i].candidateAmplitude = IsGreater(retAllMergedValues[i].ampChange, ampThrBreak);
//          DEBUG
//            std::cout << ValueToString(retAllMergedValues[i].candidateAmplitude, true) << std::endl;
//          DEBUG
    }

    // we iterate from index 1 to index size - 2 but we access also these indexes
    std::vector<int> indexesToUpdate;
    for(size_t i = 1; i<retAllMergedValues.size()-1; i++) {
        if (IsGreater(retAllMergedValues[i].cohChange, m_CohThrHigh) &&
                ((retAllMergedValues[i-1].candidateAmplitude) ||
                 (retAllMergedValues[i].candidateAmplitude) ||
                 (retAllMergedValues[i+1].candidateAmplitude))) {
            indexesToUpdate.push_back(i);
            // Note: we do not set here as we change the value for the next item that might
            // be incorrectly set to true. This is why we save the indexes and make the update later
            //retAllMergedValues[i].candidateAmplitude = true;
        }
    }
    for (auto idx: indexesToUpdate) {
        retAllMergedValues[idx].candidateAmplitude = true;
    }
}


void TsaHarvestOnlyAnalysisHandler::UpdateMarker4Infos(const FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues,
                        double &ampThrValue) {
    (void)fieldInfos;    // supress unused warning
    // MARKER 4  Presence of vegetation cycle (BACKSCATTER): M4 == $grd.presence
    for(size_t i = 0; i<retAllMergedValues.size(); i++) {
        retAllMergedValues[i].amplitudePresence = IsGreater(retAllMergedValues[i].ampMax, ampThrValue);
        if (IsGreater(retAllMergedValues[i].cohChange, m_CohThrHigh) &&
                (retAllMergedValues[i].ndviPresence == true) &&
                (retAllMergedValues[i].candidateOptical == true) &&
                (retAllMergedValues[i].candidateAmplitude == true) &&
                !retAllMergedValues[i].amplitudePresence ) {
            retAllMergedValues[i].amplitudePresence = true;
        }
    }
}


void TsaHarvestOnlyAnalysisHandler::UpdateMarker5Infos(const FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues) {
     // Define candidate-weeks for harvest based on coherence data (M5)
    double CohThrAbs = m_CohThrAbs;
    if (IsLess(fieldInfos.coheVVMaxValue, m_CohThrAbs)) {
        CohThrAbs = fieldInfos.coheVVMaxValue - m_CohThrBase;
    }

    for(size_t i = 0; i<retAllMergedValues.size(); i++) {
        retAllMergedValues[i].coherenceBase = IsGreaterOrEqual(retAllMergedValues[i].cohChange, m_CohThrBase);
        retAllMergedValues[i].coherenceHigh = IsGreaterOrEqual(retAllMergedValues[i].cohChange, m_CohThrHigh);
        retAllMergedValues[i].coherencePresence = IsGreaterOrEqual(retAllMergedValues[i].cohMax, CohThrAbs);
        retAllMergedValues[i].candidateCoherence =
                (retAllMergedValues[i].coherenceBase || retAllMergedValues[i].coherencePresence);
    }
}

bool TsaHarvestOnlyAnalysisHandler::HarvestEvaluation(const FieldInfoType &fieldInfos, std::vector<MergedAllValInfosType> &retAllMergedValues,
                       HarvestEvaluationInfoType &harvestEvalInfos) {
    // TODO: This can be done using a flag in a structure encapsulating also retAllMergedValues
    //      in order to avoid this iteration (can be done in an iteration previously made)
    harvestEvalInfos.Initialize(fieldInfos);
    // Update the L_Week time
    harvestEvalInfos.ttLWeekStartTime = retAllMergedValues[retAllMergedValues.size() - 1].ttDate;

    unsigned int nSeqIdNo = std::atoi(fieldInfos.fieldSeqId.c_str());
    const std::string &prevHWeekStartStr = m_prevPrdReader.GetHWeekStartForFieldId(nSeqIdNo);
    time_t ttprevHWeekStart = 0;
    if (prevHWeekStartStr.size() > 0) {
        ttprevHWeekStart = GetTimeFromString(prevHWeekStartStr);
    }

    if (!HasValidNdviValues(retAllMergedValues, m_OpticalThrVegCycle, true)) {
        harvestEvalInfos.ndviPresence = false;               // M1
        harvestEvalInfos.candidateOptical = NR;           // M2
        harvestEvalInfos.candidateAmplitude = NR;         // M3
        harvestEvalInfos.amplitudePresence = NR;          // M4
        harvestEvalInfos.candidateCoherence = NR;         // M5
        harvestEvalInfos.harvestConfirmWeek = (ttprevHWeekStart != 0 ? GetWeekFromDate(ttprevHWeekStart) : NR);
        harvestEvalInfos.ttHarvestConfirmWeekStart = (ttprevHWeekStart != 0 ? ttprevHWeekStart : NR);
        harvestEvalInfos.ttS1HarvestWeekStart = NR;
        return false;
    }

    int idxFirstHarvest = -1;
    int idxFirstS1Harvest = -1;
    bool harvestS1;
    time_t harvestConfirmWeekStart = NOT_AVAILABLE;
    for(size_t i = 0; i<retAllMergedValues.size(); i++) {
        harvestS1 = ((retAllMergedValues[i].ndviPresence == true) && (retAllMergedValues[i].candidateAmplitude == true) &&
                     (retAllMergedValues[i].amplitudePresence == true) && (retAllMergedValues[i].candidateCoherence == true) &&
                     retAllMergedValues[i].ttDate >= fieldInfos.ttHarvestStartWeekFloorTime);

        if (idxFirstS1Harvest == -1 && harvestS1) {
            idxFirstS1Harvest = i;
        }

        retAllMergedValues[i].harvest = (harvestS1 && (retAllMergedValues[i].candidateOptical == true));

        // save the first == week of harvest
        if (retAllMergedValues[i].harvest && idxFirstHarvest == -1) {
            idxFirstHarvest = i;
            harvestConfirmWeekStart = retAllMergedValues[i].ttDate;
        }
    }

    // update the harvestConfirmed with the value from the previous file, if present and different
    if (ttprevHWeekStart != 0) {
        harvestConfirmWeekStart = ttprevHWeekStart;
    }

    int lastAvailableIdx = retAllMergedValues.size() - 1;
    harvestEvalInfos.ndviPresence = retAllMergedValues[lastAvailableIdx].ndviPresence;               // M1
    harvestEvalInfos.candidateOptical = retAllMergedValues[lastAvailableIdx].candidateOptical;       // M2
    harvestEvalInfos.candidateAmplitude = retAllMergedValues[lastAvailableIdx].candidateAmplitude;   // M3
    harvestEvalInfos.amplitudePresence = retAllMergedValues[lastAvailableIdx].amplitudePresence;     // M4
    harvestEvalInfos.candidateCoherence = retAllMergedValues[lastAvailableIdx].candidateCoherence;   // M5

    // "HARVESTED CONDITIONS NOT DETECTED"
    if (IsNA(harvestConfirmWeekStart)) {
        // report results from last available week
        harvestEvalInfos.harvestConfirmWeek = NR;
        harvestEvalInfos.ttHarvestConfirmWeekStart = NR;
    } else {
        harvestEvalInfos.harvestConfirmWeek = GetWeekFromDate(harvestConfirmWeekStart);
        harvestEvalInfos.ttHarvestConfirmWeekStart = harvestConfirmWeekStart;
    }

    if (idxFirstS1Harvest != -1) {
        harvestEvalInfos.ttS1HarvestWeekStart = retAllMergedValues[idxFirstS1Harvest].ttDate;
    } else {
        harvestEvalInfos.ttS1HarvestWeekStart = NR;
    }

    return true;
}

void TsaHarvestOnlyAnalysisHandler::UpdateGapsInformation(time_t startTime, time_t endTime,
                           time_t ttPrevDate, time_t ttCurDate, int &sumVal, bool countAll) {
    // Check for gaps between harvest start and harvest end interval
    if (ttCurDate <= startTime || ttPrevDate >= endTime) {
        return;
    }
    if (startTime != 0 && endTime != 0) {
        if (ttPrevDate < startTime) {
            ttPrevDate = startTime;
        }
        if (ttPrevDate < endTime && ttCurDate >= endTime) {
            ttCurDate = endTime;
        }
        int diffInDays = (ttCurDate - ttPrevDate) / SEC_IN_DAY;
        if (diffInDays > 7) {
            sumVal += ((diffInDays / 7) - (countAll ? 0 : 1));
        }
    }
}


void TsaHarvestOnlyAnalysisHandler::UpdateGapsInformation(const std::vector<MergedAllValInfosType> &values,
                                                          FieldInfoType &fieldInfos,
                                                          const HarvestEvaluationInfoType &harvestEvalInfos) {
    int sum = 0;
    int sumHGaps = 0;
    int sumPGaps = 0;
    int diffInDays;
    bool bComputeHWS1GapsVal = false;
    int sumHWS1Gaps = 0;
    time_t ttPractConfWeekOffset = 0;

    const std::string & prevHWS1GapsVal = m_prevPrdReader.GetHWS1GapsInfosForFieldId(std::atoi(fieldInfos.fieldSeqId.c_str()));
    int nPrevHWS1GapsVal = std::atoi(prevHWS1GapsVal.c_str());
    if (prevHWS1GapsVal.length() > 0 && nPrevHWS1GapsVal > 0) {
        fieldInfos.h_W_S1GapsInfos = nPrevHWS1GapsVal;
    } else {
        if (!IsNA(harvestEvalInfos.harvestConfirmWeek)) {
            bComputeHWS1GapsVal = true;
            ttPractConfWeekOffset = harvestEvalInfos.ttHarvestConfirmWeekStart - SEC_IN_5_WEEKS;
        }
    }


    // according to ISO calendar, the first week of the year is the one that contains 4th of January
    time_t ttFirstWeekStart = GetTimeFromString(std::to_string(fieldInfos.year).append("-01-01"));
    time_t ttPrevDate = ttFirstWeekStart;
    time_t ttCurDate;
    if (values.size() > 0) {
        for(size_t i = 0; i < (values.size() + 1); i++) {
            if (i > 0) {
                ttPrevDate = values[i-1].ttDate;
            }
            if (i<values.size()) {
                ttCurDate = values[i].ttDate;
            } else {
                ttCurDate = m_ttLimitAcqDate;
            }
            diffInDays = (ttCurDate - ttPrevDate) / SEC_IN_DAY;
            if (diffInDays > 7) {
                sum += (diffInDays / 7) - 1;
            }

            // Check for gaps between harvest start and harvest end interval
            UpdateGapsInformation(fieldInfos.ttHarvestStartWeekFloorTime, fieldInfos.ttHarvestEndWeekFloorTime,
                                  ttPrevDate, ttCurDate, sumHGaps);
            // Check for gaps between practice start and harvest end interval
            UpdateGapsInformation(fieldInfos.ttPracticeStartWeekFloorTime, fieldInfos.ttPracticeEndWeekFloorTime,
                                  ttPrevDate, ttCurDate, sumPGaps);

            // Check for gaps in the preceding 5 weeks before the harvestConfirmWeek
            if (bComputeHWS1GapsVal) {
                UpdateGapsInformation(ttPractConfWeekOffset, harvestEvalInfos.ttHarvestConfirmWeekStart,
                                      ttPrevDate, ttCurDate, sumHWS1Gaps, true);
            }
        }
    }
    fieldInfos.gapsInfos = sum;
    fieldInfos.hS1GapsInfos = sumHGaps;
    fieldInfos.pS1GapsInfos = sumPGaps;
    if (bComputeHWS1GapsVal) {
        fieldInfos.h_W_S1GapsInfos = sumHWS1Gaps;
    }
}


double TsaHarvestOnlyAnalysisHandler::ComputeOpticalThresholdBuffer(int nVegWeeksCnt, double maxMeanNdviDropVal, double OpticalThresholdValue) {
    double OpticalThresholdBuffer = -1;
    if (nVegWeeksCnt > 1) {
        OpticalThresholdBuffer = std::round((maxMeanNdviDropVal - OpticalThresholdValue) / m_OpticalThrBufDenominator);
    }
    if (OpticalThresholdBuffer < 0) {
        OpticalThresholdBuffer = 0;
    }
    return OpticalThresholdBuffer;
}

double TsaHarvestOnlyAnalysisHandler::ComputeAmplitudeThresholdValue(double meanVal, double stdDevVal) {
    //        grd.thr.value <- paste("grd.thr.value <- mean(grd.filtered)-sd(grd.filtered)/2")
    // or
    //        grd.thr.value <- paste("grd.thr.value <- mean(grd.filtered)
    return meanVal - (m_UseStdDevInAmpThrValComp ? (stdDevVal / m_AmpThrValDenominator) : 0);
}

double TsaHarvestOnlyAnalysisHandler::ComputeAmplitudeThresholdBreak(double stdDevVal) {
    double ampThrBreak = stdDevVal / m_AmpThrBreakDenominator;
    // minimum change
    if (IsLess(ampThrBreak, m_AmpThrMinimum)) {
        ampThrBreak = m_AmpThrMinimum;
    }
    return ampThrBreak;
}
