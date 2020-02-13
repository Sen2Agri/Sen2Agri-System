#include "TsaEfaAnalysisBase.h"

#include "TimeSeriesAnalysisUtils.h"

TsaEfaAnalysisBase::TsaEfaAnalysisBase(itk::Logger *logger) : m_tsaPreProcessor(logger)
{

}

bool TsaEfaAnalysisBase::ExtractEfaMarkers(time_t ttStartTime, time_t ttEndTime, const FieldInfoType &fieldInfos,
                       std::vector<EfaMarkersInfoType> &efaMarkers) {

    std::vector<MergedAllValInfosType> allMergedValues;
    std::vector<MergedDateAmplitudeType> mergedAmpInfos;
    if (!m_tsaPreProcessor.GroupAndMergeFilteredData(fieldInfos, ttStartTime, ttEndTime, mergedAmpInfos, allMergedValues)) {
        return false;
    }

    efaMarkers.resize(allMergedValues.size());

//      DEBUG
    // std::cout << TimeToString(ttStartTime) << std::endl;
    // std::cout << TimeToString(ttEndTime) << std::endl;
    m_debugPrinter.PrintEfaMarkers(allMergedValues, efaMarkers);
//      DEBUG

    // ### COMPUTE EFA MARKERS ###

    // ### MARKER 6 - $ndvi.presence ###

    int prevNotNaNdviIdx = -1;
    bool allNdviLessDw = true;
    bool ndviLessDwFound = false;
    bool ndviLessUpFound = false;
    // TODO: Check if duplicated code with UpdateMarker2Infos
    double minMeanNdviDropVal = NOT_AVAILABLE;

    double efaNdviUp = IsNA(m_EfaNdviUp) ? m_NdviUp : m_EfaNdviUp;
    double efaNdviDown = IsNA(m_EfaNdviDown) ? m_NdviDown : m_EfaNdviDown;
    double efaNdviMin = IsNA(m_EfaNdviMin) ? DEFAULT_EFA_NDVI_MIN : m_EfaNdviMin;

    // TODO: in the code below we might have access violations due to ndviCoheFilteredValues and ampFilteredValues different sizes
    for (size_t i = 0; i<allMergedValues.size(); i++) {
        efaMarkers[i].ttDate = allMergedValues[i].ttDate;
        efaMarkers[i].ndviPresence = NOT_AVAILABLE;
        efaMarkers[i].ndviDrop = NOT_AVAILABLE;
        efaMarkers[i].ndviGrowth = NOT_AVAILABLE;
        efaMarkers[i].ndviNoLoss = NOT_AVAILABLE;
        efaMarkers[i].ndviMean = allMergedValues[i].ndviMeanVal;
        if (IsNA(efaMarkers[i].ndviMean)) {
            continue;
        }
        efaMarkers[i].ndviPresence = IsGreaterOrEqual(allMergedValues[i].ndviMeanVal, m_EfaNdviThr);
        if (!IsNA(efaNdviDown) && !IsNA(efaNdviUp)) {
            if (!IsNA(allMergedValues[i].ndviMeanVal)) {
                // skip the first line
                if (prevNotNaNdviIdx >= 0) {
                    efaMarkers[i].ndviDrop = (IsLess(allMergedValues[i].ndviMeanVal, allMergedValues[prevNotNaNdviIdx].ndviMeanVal) &&
                                              IsLess(allMergedValues[i].ndviMeanVal, efaNdviUp));
                }
                prevNotNaNdviIdx = i;
                // not all ndvi are less than ndvi down
                if (IsGreaterOrEqual(allMergedValues[i].ndviMeanVal, efaNdviDown)) {
                    allNdviLessDw = false;
                }
                if (efaMarkers[i].ndviDrop == true) {
                    if (IsLess(allMergedValues[i].ndviMeanVal, efaNdviDown)) {
                        // at least one ndvi is less than ndvi down
                        ndviLessDwFound = true;
                    }
                    if (IsLess(allMergedValues[i].ndviMeanVal, efaNdviUp)) {
                        ndviLessUpFound = true;
                    }

                    if (IsNA(minMeanNdviDropVal)) {
                        minMeanNdviDropVal = allMergedValues[i].ndviMeanVal;
                    } else {
                        if (IsGreater(minMeanNdviDropVal, allMergedValues[i].ndviMeanVal)) {
                            minMeanNdviDropVal = allMergedValues[i].ndviMeanVal;
                        }
                    }
                }
            }
        }
        // # MARKER 7 - $ndvi.growth ###
        // if both ndviPresence and ndviDrop are NA, we let ndviGrowth to NA
        if (!IsNA(efaMarkers[i].ndviPresence) || !IsNA(efaMarkers[i].ndviDrop)) {
            efaMarkers[i].ndviGrowth = efaMarkers[i].ndviPresence || efaMarkers[i].ndviDrop == false;
        }
        if (IsNA(efaMarkers[i].ndviDrop) && (efaMarkers[i].ndviPresence == false)) {
            efaMarkers[i].ndviGrowth = false;
        }
        if (efaMarkers[i].ndviGrowth == true) {
            efaMarkers[i].ndviGrowth = IsGreaterOrEqual(allMergedValues[i].ndviMeanVal, efaNdviMin);
        }

        double opticalThresholdValue;
        if (allNdviLessDw || ndviLessDwFound) {
            opticalThresholdValue = efaNdviDown;
        } else if (ndviLessUpFound) {
            opticalThresholdValue = m_ndviStep * std::ceil(minMeanNdviDropVal / m_ndviStep);
        } else {
            opticalThresholdValue = efaNdviUp;
        }

        // ### MARKER 8 - $ndvi.noloss ###
        efaMarkers[i].ndviNoLoss = !(efaMarkers[i].ndviDrop &&
                                     IsLessOrEqual(allMergedValues[i].ndviMeanVal, opticalThresholdValue));
        if (IsNA(efaMarkers[i].ndviDrop)) {
            efaMarkers[i].ndviNoLoss = NOT_AVAILABLE;
        }
    }

    // ### MARKER 9 - $grd.noloss ###
//      DEBUG
//        m_debugPrinter.PrintEfaMarkers(allMergedValues, efaMarkers);
//      DEBUG

    std::vector<int> allWeeks;
    allWeeks.reserve(mergedAmpInfos.size());
    std::transform(mergedAmpInfos.begin(), mergedAmpInfos.end(),
                   std::back_inserter(allWeeks), [](MergedDateAmplitude f){return f.vvInfo.weekNo;});
    // sort the values
    std::vector<int> allWeeksUniques = allWeeks;
    std::sort(allWeeksUniques.begin(), allWeeksUniques.end());
    // keep the unique weeks values
    allWeeksUniques.erase(std::unique(allWeeksUniques.begin(), allWeeksUniques.end()), allWeeksUniques.end());

    std::vector<double> ampSlopeVec, ampPValueVec;
    ampSlopeVec.resize(efaMarkers.size());
    std::fill (ampSlopeVec.begin(),ampSlopeVec.end(), NOT_AVAILABLE);
    ampPValueVec.resize(efaMarkers.size());
    std::fill (ampPValueVec.begin(),ampPValueVec.end(), NOT_AVAILABLE);

    if (allWeeksUniques.size() >= 3) {
        // linear fitting for +/- 2 weeks
        int curWeek;
        for (size_t i = 1; i< efaMarkers.size() - 1; i++) {
            curWeek = GetWeekFromDate(efaMarkers[i].ttDate);
            //curWeek = i;
            // extract from the amplitude vectors , the weeks
            std::vector<double> subsetAmpTimes;
            std::vector<int> subsetAmpWeeks;
            std::vector<double> subsetAmpValues;
            for(size_t j = 0; j<mergedAmpInfos.size(); j++) {
                if ((mergedAmpInfos[j].vvInfo.weekNo >= curWeek - 2) &&
                    (mergedAmpInfos[j].vvInfo.weekNo <= curWeek + 2)) {
                    if (std::find(subsetAmpWeeks.begin(), subsetAmpWeeks.end(),
                                  mergedAmpInfos[j].vvInfo.weekNo) == subsetAmpWeeks.end()) {
                        subsetAmpWeeks.push_back(mergedAmpInfos[j].vvInfo.weekNo);
                    }
                    subsetAmpTimes.push_back(mergedAmpInfos[j].ttDate / SEC_IN_DAY);
                    subsetAmpValues.push_back(mergedAmpInfos[j].ampRatio);
                }
            }
            // we check the unique weeks
            if (subsetAmpWeeks.size() < 3) {
                continue;
            }

            double slope;
            bool res = ComputeSlope(subsetAmpTimes, subsetAmpValues, slope);
            // std::cout << "Slope: " << slope << std::endl;
            if (res) {
                // compute the marker to be updated
                 ampSlopeVec[i] = slope;
            }

            double pValue;
            res = ComputePValue(subsetAmpTimes, subsetAmpValues, pValue);
            // std::cout << "p-Value: " << pValue << std::endl;
            if (res) {
                // compute the marker to be updated
                ampPValueVec[i] = pValue;
            }
        }
    }

    double slopeThreshold = (!IsNA(m_EfaAmpThr)) ? m_EfaAmpThr : 0.01;
    for(size_t i = 0; i<efaMarkers.size(); i++) {
        if (IsNA(ampSlopeVec[i]) || IsNA(ampPValueVec[i])) {
            efaMarkers[i].ampNoLoss = NOT_AVAILABLE;
        } else {
            efaMarkers[i].ampNoLoss = !(IsGreaterOrEqual(ampSlopeVec[i], slopeThreshold) &&
                                        IsLess(ampPValueVec[i], 0.05));
        }
    }

    // ### MARKER 10 - $coh.noloss ###
    double cohThrHigh = (!IsNA(m_EfaCohChange)) ? m_EfaCohChange : m_CohThrHigh;
    double cohThrAbs = (!IsNA(m_EfaCohValue)) ? m_EfaCohValue : m_CohThrAbs;
    for(size_t i = 0; i<efaMarkers.size(); i++) {
        bool cohHigh = IsGreaterOrEqual(allMergedValues[i].cohChange, cohThrHigh);
        bool cohPresence = IsGreaterOrEqual(allMergedValues[i].cohMax, cohThrAbs);
        efaMarkers[i].cohNoLoss = !(cohHigh || cohPresence);
    }

//      DEBUG
    m_debugPrinter.PrintEfaMarkers(allMergedValues, efaMarkers);
//      DEBUG

    return true;
}
