#include "TsaDataPreProcessor.h"

#include "TsaHelperFunctions.h"

TsaDataExtrPreProcessor::TsaDataExtrPreProcessor(itk::Logger *logger) : m_pLogger(logger)
{

}

bool TsaDataExtrPreProcessor::MergeAmplitudes(const std::string &fieldId, const std::vector<InputFileLineInfoType> &ampVVLines,
                     const std::vector<InputFileLineInfoType> &ampVHLines,
                     std::vector<MergedDateAmplitudeType> &retInfos) {
    if (ampVVLines.size() == 0) {
        otbAppLogWARNING("The amplitude file info sizes is zero for field " << fieldId);
        return false;
    }
    if (ampVVLines.size() != ampVHLines.size()) {
        otbAppLogWARNING("The extracted amplitude file sizes differ for field " << fieldId);
        return false;
    }
    retInfos.resize(ampVVLines.size());
    // Theoretically we should have at the same position the same date
    for (size_t i = 0; i<ampVVLines.size(); i++) {
        const InputFileLineInfoType &vvLine = ampVVLines[i];
        const InputFileLineInfoType &vhLine = ampVHLines[i];
        if (vvLine.ttDate != vhLine.ttDate) {
            otbAppLogWARNING("Date inconsistency detected for amplitudes at index " << i << " for field " << fieldId);
            return false;
        }
        retInfos[i].vvInfo = vvLine;
        retInfos[i].vhInfo = vhLine;
        retInfos[i].ampRatio = vvLine.meanVal - vhLine.meanVal;
        retInfos[i].ttDate = vvLine.ttDate;
    }
    return true;
}

template <typename TimedValue>
bool TsaDataExtrPreProcessor::GroupTimedValuesByWeeks(const std::vector<TimedValue> &infos,
                            std::vector<GroupedMeanValInfosType> &retGroups) {
    double curMaxVal;
    double curMaxChangeVal;
    double curVal;
    double curChangeVal;
    time_t ttTime;
    std::map<time_t, GroupedMeanValInfosType> mapGroups;
    for (const auto &info: infos) {
        ttTime = info.GetFloorTime();
        curMaxVal = mapGroups[ttTime].maxVal;
        curVal = info.GetValue();
        mapGroups[ttTime].ttDate = ttTime;
        mapGroups[ttTime].maxVal = IsLess(curMaxVal, curVal) ? curVal : curMaxVal ;
        mapGroups[ttTime].sum += curVal;
        mapGroups[ttTime].cnt++;
        mapGroups[ttTime].meanVal = (mapGroups[ttTime].sum / mapGroups[ttTime].cnt);

        // Get the change value, if supported
        if (info.GetChangeValue(curChangeVal)) {
            curMaxChangeVal = mapGroups[ttTime].maxChangeVal;
            mapGroups[ttTime].maxChangeVal = IsLess(curMaxChangeVal, curChangeVal) ? curChangeVal : curMaxChangeVal ;
        }
    }

    for( std::map<time_t, GroupedMeanValInfosType>::iterator it = mapGroups.begin(); it != mapGroups.end(); ++it ) {
            retGroups.push_back( it->second );
    }
    std::sort(retGroups.begin(), retGroups.end(), TimedValInfosComparator<GroupedMeanValInfosType>());

    return true;
}

// Compute 3-weeks-mean before a week and then
// Compute difference between week-mean and previous 3-week-mean
bool TsaDataExtrPreProcessor::Compute3WeeksAmplRatioDiffs(std::vector<GroupedMeanValInfosType> &ampRatioGroups) {
    double cur3WeeksMeanVal;
    for (size_t i = 0; i<ampRatioGroups.size(); i++)
    {
        if (i >= 3) {
            // Compute 3-weeks-mean before a week ($grd.w3m)
            cur3WeeksMeanVal = 0;
            for (int j = 1; j<=3; j++) {
                cur3WeeksMeanVal += ampRatioGroups[i - j].meanVal;
            }
            cur3WeeksMeanVal /= 3;
            // Compute difference between week-mean and previous 3-week-mean
            ampRatioGroups[i].ampChange = ampRatioGroups[i].meanVal - cur3WeeksMeanVal;
            if (ampRatioGroups[i].ampChange < 0) {
                ampRatioGroups[i].ampChange = 0;
            } else {
                // round the value to 2 decimals
                ampRatioGroups[i].ampChange = std::round(ampRatioGroups[i].ampChange * 100)/100;
            }
        }
    }

    return true;
}

bool TsaDataExtrPreProcessor::MergeAmpCoheFieldInfos(const FieldInfoType &fieldInfos, const std::vector<GroupedMeanValInfosType> &ampRatioGroups,
                        const std::vector<GroupedMeanValInfosType> &coherenceGroups,
                        std::vector<MergedAllValInfosType> &retAllMergedValues)
{
    if (ampRatioGroups.size() != coherenceGroups.size() && m_bVerbose) {
        otbAppLogWARNING("Amplitude and coherence groups sizes differ when merging for field " << fieldInfos.fieldId <<
                         " Amp size: " << ampRatioGroups.size() << " Cohe size: " << coherenceGroups.size());
    }

    bool refListIsAmp = ampRatioGroups.size() <= coherenceGroups.size();
    size_t retListSize = refListIsAmp ? ampRatioGroups.size() : coherenceGroups.size();
    const std::vector<GroupedMeanValInfosType> &refList1 = refListIsAmp ? ampRatioGroups : coherenceGroups;
    const std::vector<GroupedMeanValInfosType> &refList2 = refListIsAmp ? coherenceGroups : ampRatioGroups;

    // we keep the dates where we have both backscatter and coherence
    for(size_t i = 0; i < retListSize; i++) {
        const GroupedMeanValInfosType &refItem1 = refList1[i];
        for (size_t j = 0; j<refList2.size(); j++) {
            const GroupedMeanValInfosType &refItem2 = refList2[j];
            if (refItem1.ttDate == refList2[j].ttDate) {
                MergedAllValInfosType mergedVal;
                const GroupedMeanValInfosType &ampItem = refListIsAmp ? refItem1 : refItem2;
                const GroupedMeanValInfosType &coheItem = refListIsAmp ? refItem2 : refItem1;
                mergedVal.ttDate = ampItem.ttDate;
                mergedVal.ampMean = ampItem.meanVal;
                mergedVal.ampMax = ampItem.maxVal;
                mergedVal.ampChange = ampItem.ampChange;

                mergedVal.cohChange = coheItem.maxChangeVal;
                mergedVal.cohMax = coheItem.maxVal;
                retAllMergedValues.push_back(mergedVal);
                break;
            }
        }
    }

    return true;
}

bool TsaDataExtrPreProcessor::MergeAllFieldInfos(const FieldInfoType &fieldInfos, const std::vector<GroupedMeanValInfosType> &ampRatioGroups,
                        const std::vector<GroupedMeanValInfosType> &coherenceGroups,
                        const std::vector<GroupedMeanValInfosType> &ndviGroups,
                        std::vector<MergedAllValInfosType> &retAllMergedValues)
{
    if (!MergeAmpCoheFieldInfos(fieldInfos, ampRatioGroups, coherenceGroups, retAllMergedValues)) {
        return false;
    }

    // update the ret list size with the right value
    size_t retListSize = retAllMergedValues.size();
    if (m_bVerbose) {
        otbAppLogINFO("Kept a common AMP-COHE values of " << retListSize);
    }
    if (retListSize == 0) {
        otbAppLogWARNING("No common AMP-COHE values detected for the parcel with ID " << fieldInfos.fieldId);
        return false;
    }

    // Fill the NDVI fields
    // In this case, we might have gaps
    double prevNdviVal = NOT_AVAILABLE;
    for(size_t i = 0; i<retListSize; i++) {
        bool valueSet = false;
        for(size_t j = 0; j<ndviGroups.size(); j++) {
            if(retAllMergedValues[i].ttDate == ndviGroups[j].ttDate) {
                retAllMergedValues[i].ndviMeanVal = ndviGroups[j].meanVal;
                valueSet = true;
                break;
            }
        }
        // fill the NDVI prev values
        if (retAllMergedValues[i].ttDate >= fieldInfos.ttVegStartWeekFloorTime) {
            if (valueSet) {
                if (IsNA(prevNdviVal)) {
                    // set the current value if NA previously
                    retAllMergedValues[i].ndviPrev = retAllMergedValues[i].ndviMeanVal;
                } else {
                    // in this case, if exists a prevVal, set it
                    retAllMergedValues[i].ndviPrev = prevNdviVal;
                }
                // save current value the previously var
                prevNdviVal = retAllMergedValues[i].ndviMeanVal;
            } else {
                // just keep the previous value, whatever it is
                retAllMergedValues[i].ndviPrev = prevNdviVal;
            }
        }
    }

    // fill the NDVI next values
    double nextNdviVal = NOT_AVAILABLE;
    // iterate backwards for filling the next values
    for (int i = (int)retListSize-1; i>=1; i--) {
        if (retAllMergedValues[i].ttDate < fieldInfos.ttVegStartWeekFloorTime) {
            // Normally, there is no need to continue
            break;
        }
        if (!IsNA(retAllMergedValues[i].ndviMeanVal)) {
            nextNdviVal = retAllMergedValues[i].ndviMeanVal;
            retAllMergedValues[i].ndviNext = nextNdviVal;
        }
        retAllMergedValues[i-1].ndviNext = nextNdviVal;
    }

    for(size_t i = 0; i<retListSize; i++) {
        if (!IsNA(retAllMergedValues[i].ndviMeanVal) && IsNA(retAllMergedValues[i].ndviPrev) && !IsNA(retAllMergedValues[i].ndviNext)) {
            retAllMergedValues[i].ndviPrev = retAllMergedValues[i].ndviNext;
        }
    }

    return true;
}

bool TsaDataExtrPreProcessor::GroupAndMergeAllData(const FieldInfoType &fieldInfos, const std::vector<InputFileLineInfoType> &ampVHLines,
                          const std::vector<InputFileLineInfoType> &ampVVLines,
                          const std::vector<InputFileLineInfoType> &ndviLines, const std::vector<InputFileLineInfoType> &coheVVLines,
                          std::vector<MergedDateAmplitudeType> &mergedAmpInfos, std::vector<GroupedMeanValInfosType> &ampRatioGroups,
                          std::vector<GroupedMeanValInfosType> &ndviGroups, std::vector<GroupedMeanValInfosType> &coherenceGroups,
                          std::vector<MergedAllValInfosType> &allMergedValues) {
    // Read VV and VH backscatter (grd) and compute backscatter ratio (VV-VH)
    if (m_bVerbose) {
        otbAppLogINFO("Merging amplitudes for field  " << fieldInfos.fieldId);
    }
    if (!MergeAmplitudes(fieldInfos.fieldId, ampVVLines, ampVHLines, mergedAmpInfos)) {
        return false;
    }

    // DEBUG
//        m_debugPrinter.PrintAmplitudeInfos(fieldInfos);
    // DEBUG

    if (m_bVerbose) {
        otbAppLogINFO("Grouping amplitudes by weeks for field  " << fieldInfos.fieldId);
    }
    // Group backscatter ratio by weeks (7-day period) - compute week-mean ($grd.mean)
    if (!GroupTimedValuesByWeeks(mergedAmpInfos, ampRatioGroups)) {
        return false;
    }

    // DEBUG
    //m_debugPrinter.PrintAmpGroupedMeanValues(ampRatioGroups);
    // DEBUG

    if (m_bVerbose) {
        otbAppLogINFO("Computing amplitude 3 weeks mean - current value mean for field  " << fieldInfos.fieldId);
    }
    // Compute 3-weeks-mean before a week ($grd.w3m)
    // and compute difference between week-mean and previous 3-week-mean ($grd.change)
    if (!Compute3WeeksAmplRatioDiffs(ampRatioGroups)) {
        return false;
    }

    // DEBUG
//        m_debugPrinter.PrintAmpGroupedMeanValues(ampRatioGroups);
    // DEBUG

    // DEBUG
//        m_debugPrinter.PrintNdviInfos(fieldInfos);
    // DEBUG

    if (m_bVerbose) {
        otbAppLogINFO("Grouping NDVI by weeks for field  " << fieldInfos.fieldId);
    }
    if (!GroupTimedValuesByWeeks(ndviLines, ndviGroups)) {
        return false;
    }

    // round the values from the NDVI groupes
    std::transform(ndviGroups.begin(), ndviGroups.end(), ndviGroups.begin(),
                   [](GroupedMeanValInfosType &x) {
            x.meanVal = std::round(x.meanVal);
            return x;
    });

    // DEBUG
    m_debugPrinter.PrintNdviGroupedMeanValues(ndviGroups);
    // DEBUG

    // DEBUG
    m_debugPrinter.PrintCoherenceInfos(fieldInfos);
    // DEBUG

    if (m_bVerbose) {
        otbAppLogINFO("Grouping Coherence by weeks for field  " << fieldInfos.fieldId);
    }
    if (!GroupTimedValuesByWeeks(coheVVLines, coherenceGroups)) {
        return false;
    }

    // round the coherence change values
    // round the values from the NDVI groupes
    std::transform(coherenceGroups.begin(), coherenceGroups.end(), coherenceGroups.begin(),
                   [](GroupedMeanValInfosType &x) {
            x.maxChangeVal = (std::round(x.maxChangeVal * 1000) / 1000);
            return x;
    });

    // DEBUG
//        m_debugPrinter.PrintCoherenceGroupedMeanValues(coherenceGroups);
    // DEBUG

    if (m_bVerbose) {
        otbAppLogINFO("Merging all information for field  " << fieldInfos.fieldId);
    }
    if (!MergeAllFieldInfos(fieldInfos, ampRatioGroups, coherenceGroups,
                            ndviGroups, allMergedValues)) {
        return false;
    }
    return true;
}

bool TsaDataExtrPreProcessor::GroupAndMergeFilteredData(const FieldInfoType &fieldInfos, time_t ttStartTime, time_t ttEndTime,
                               std::vector<MergedDateAmplitudeType> &mergedAmpInfos,
                               std::vector<MergedAllValInfosType> &allMergedValues) {
    const std::vector<InputFileLineInfoType> &ndviFilteredValues = FilterValuesByDates(
                fieldInfos.ndviLines, ttStartTime, ttEndTime);
    const std::vector<InputFileLineInfoType> &coheVVFilteredValues = FilterValuesByDates(
                fieldInfos.coheVVLines, ttStartTime, ttEndTime);

    const std::vector<InputFileLineInfoType> &ampVVFilteredValues = FilterValuesByDates(
                fieldInfos.ampVVLines, ttStartTime, ttEndTime + SEC_IN_WEEK);
    const std::vector<InputFileLineInfoType> &ampVHFilteredValues = FilterValuesByDates(
                fieldInfos.ampVHLines, ttStartTime, ttEndTime + SEC_IN_WEEK);

    std::vector<GroupedMeanValInfosType> ampRatioGroups;
    std::vector<GroupedMeanValInfosType> ndviGroups;
    std::vector<GroupedMeanValInfosType> coherenceGroups;

    if (!GroupAndMergeAllData(fieldInfos, ampVHFilteredValues, ampVVFilteredValues, ndviFilteredValues, coheVVFilteredValues,
                              mergedAmpInfos, ampRatioGroups, ndviGroups, coherenceGroups,
                              allMergedValues)) {
        return false;
    }
    return true;
}
