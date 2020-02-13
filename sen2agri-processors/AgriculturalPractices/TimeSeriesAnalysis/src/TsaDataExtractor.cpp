#include "TsaDataExtractor.h"
#include "TimeSeriesAnalysisUtils.h"
#include "StatisticsInfosReaderFactory.h"

#define MIN_REQUIRED_COHE_VALUES        15      // previous was 26

TsaDataExtractor::TsaDataExtractor(itk::Logger *logger) : m_pLogger(logger)
{
    m_bAllowGaps = true;
    m_bVerbose = false;
}

void TsaDataExtractor::Initialize(const std::string &ampSrc, const std::string &coheSrc, const std::string &ndviSrc,
                                  int minCoheAcqs, int curYear, const std::string &inputType) {
    auto factory = StatisticsInfosReaderFactory::New();
    m_pAmpReader = factory->GetInfosReader(inputType);
    m_pAmpReader->Initialize(ampSrc, {"VV", "VH"}, curYear);
    m_pAmpReader->SetUseDate2(false);
    m_pAmpReader->SetSwitchDates(false);

    m_pCoheReader = factory->GetInfosReader(inputType);
    m_pCoheReader->Initialize(coheSrc, {"VV", "VH"}, curYear);
    m_pCoheReader->SetMinRequiredEntries(minCoheAcqs);

    m_pCoheReader->SetUseDate2(true);
    m_pCoheReader->SetSwitchDates(true);

    m_pNdviReader = factory->GetInfosReader(inputType);
    m_pNdviReader->Initialize(ndviSrc, {}, curYear);
    m_pNdviReader->SetUseDate2(false);
    m_pNdviReader->SetSwitchDates(false);
}

void TsaDataExtractor::KeepCommonDates(const std::vector<InputFileLineInfoType> &lineInfos1, const std::vector<InputFileLineInfoType> &lineInfos2,
                     std::vector<InputFileLineInfoType> &retLineInfos1, std::vector<InputFileLineInfoType> &retLineInfos2) {
    if (lineInfos1.size() == 0 || lineInfos2.size() == 0) {
        return;
    }
    for (size_t i = 0; i<lineInfos1.size(); i++) {
        for (size_t j = 0; j<lineInfos2.size(); j++) {
            if (lineInfos1[i].ttDate == lineInfos2[j].ttDate) {
                retLineInfos1.push_back(lineInfos1[i]);
                retLineInfos2.push_back(lineInfos2[j]);
            }
        }
    }
    if (m_bVerbose) {
        otbAppLogINFO("Kept a number of " << retLineInfos1.size() << " common lines!");
    }
}

std::vector<InputFileLineInfoType> TsaDataExtractor::FilterDuplicates(const std::vector<InputFileLineInfoType> &lineInfos) {
    std::vector<InputFileLineInfoType> retInfos;
    for (std::vector<InputFileLineInfoType>::const_iterator it1 = lineInfos.begin(); it1 != lineInfos.end(); ++it1) {
        bool found = false;
        for (std::vector<InputFileLineInfoType>::const_iterator it2 = retInfos.begin(); it2 != retInfos.end(); ++it2) {
            if (it1->ttDate == it2->ttDate && IsEqual(it1->meanVal, it2->meanVal) && IsEqual(it1->stdDev, it2->stdDev)) {
                found = true;
                break;
            }
        }
        if (!found) {
            retInfos.push_back(*it1);
        }
    }
    return retInfos;
}


bool TsaDataExtractor::ExtractAmplitudeFilesInfos(FieldInfoType &fieldInfo)
{
    std::map<std::string, std::vector<InputFileLineInfoType>> mapInfos;
    if (!m_pAmpReader->GetEntriesForField(fieldInfo.fieldSeqId, {"VV", "VH"}, mapInfos)) {
        otbAppLogWARNING("Cannot extract amplitude infos for the field " << fieldInfo.fieldId);
        return false;
    }
    std::map<std::string, std::vector<InputFileLineInfoType>>::const_iterator itVV;
    std::map<std::string, std::vector<InputFileLineInfoType>>::const_iterator itVH;
    itVV = mapInfos.find("VV");
    itVH = mapInfos.find("VH");
    if (itVV == mapInfos.end() || itVH == mapInfos.end() ||
            itVV->second.size() == 0 || itVH->second.size() == 0) {
        if (m_bVerbose) {
            otbAppLogWARNING("Didn't find entries for both VV and VH amplitude files for the field " << fieldInfo.fieldId);
        }
        return false;
    }

    if (m_bVerbose) {
        otbAppLogINFO("Amplitude : Extracted a number of " << itVV->second.size() <<
                      " VV entries and a number of " << itVH->second.size() << " VH entries!");
    }
    const std::vector<InputFileLineInfoType> &uniqueVVEntries = FilterDuplicates(itVV->second);
    const std::vector<InputFileLineInfoType> &uniqueVHEntries = FilterDuplicates(itVH->second);

    std::vector<InputFileLineInfoType> commonVVInfos;
    std::vector<InputFileLineInfoType> commonVHInfos;
    KeepCommonDates(uniqueVVEntries, uniqueVHEntries, commonVVInfos, commonVHInfos);

    fieldInfo.ampVVLines.insert(fieldInfo.ampVVLines.end(), commonVVInfos.begin(), commonVVInfos.end());
    fieldInfo.ampVHLines.insert(fieldInfo.ampVHLines.end(), commonVHInfos.begin(), commonVHInfos.end());

    if (m_bVerbose) {
        otbAppLogINFO("Amplitude : Available " << fieldInfo.ampVVLines.size() <<
                      " VV entries and a number of " << fieldInfo.ampVHLines.size() << " VH entries!");
    }

    if (!m_bAllowGaps) {
        if (!CheckWeekGaps(fieldInfo.vegStartWeekNo, fieldInfo.ampVVLines) ||
            !CheckWeekGaps(fieldInfo.vegStartWeekNo, fieldInfo.ampVHLines)) {
            if (m_bVerbose) {
                otbAppLogWARNING("No valid amplitude lines were found for the field id (gaps)" << fieldInfo.fieldId);
            }
            return false;
        }
    }

    return true;
}

bool TsaDataExtractor::ExtractCoherenceFilesInfos(FieldInfoType &fieldInfo)
{
    std::map<std::string, std::vector<InputFileLineInfoType>> mapInfos;
    if (!m_pCoheReader->GetEntriesForField(fieldInfo.fieldSeqId, /*{"VV", "VH"}*/{"VV"}, mapInfos)) {
        otbAppLogWARNING("Cannot extract coherence infos for the field " << fieldInfo.fieldId);
        return false;
    }
    std::map<std::string, std::vector<InputFileLineInfoType>>::const_iterator itVV;
    itVV = mapInfos.find("VV");
    if (itVV == mapInfos.end() || itVV->second.size() == 0) {
        if (m_bVerbose) {
            otbAppLogWARNING("Didn't find any entry in coherence VV files for the field " << fieldInfo.fieldId);
        }
        return false;
    }

    if (m_bVerbose) {
        otbAppLogINFO("Coherence : Extracted a number of VV entries of " << itVV->second.size());
    }

    const std::vector<InputFileLineInfoType> &uniqueEntries = FilterDuplicates(itVV->second);
    fieldInfo.coheVVLines.insert(fieldInfo.coheVVLines.end(), uniqueEntries.begin(), uniqueEntries.end());

    if (fieldInfo.coheVVLines.size() <= MIN_REQUIRED_COHE_VALUES) {
        if (m_bVerbose) {
            otbAppLogWARNING("No/empty/short coherence input text files the field id " << fieldInfo.fieldId);
        }
        return false;
    }

    std::sort (fieldInfo.coheVVLines.begin(), fieldInfo.coheVVLines.end(), TimedValInfosComparator<InputFileLineInfoType>());

    for (std::vector<InputFileLineInfoType>::const_iterator it = fieldInfo.coheVVLines.begin() ;
         it != fieldInfo.coheVVLines.end(); ++it) {
        if (IsNA(fieldInfo.coheVVMaxValue)) {
            fieldInfo.coheVVMaxValue = it->meanVal;
        } else {
            if (IsLess(fieldInfo.coheVVMaxValue, it->meanVal)) {
                fieldInfo.coheVVMaxValue = it->meanVal;
            }
        }
    }

    if (!m_bAllowGaps) {
        if (!CheckWeekGaps(fieldInfo.vegStartWeekNo, fieldInfo.coheVVLines)) {
            if (m_bVerbose) {
                otbAppLogWARNING("No valid coherence lines were found for the field id (gaps) " << fieldInfo.fieldId);
            }
            return false;
        }
    }

    return true;
}

bool TsaDataExtractor::ExtractNdviFilesInfos(FieldInfoType &fieldInfo)
{
    //if (!m_pNdviReader->GetEntriesForField(fieldInfo.fieldId, fieldInfo.ndviLines)) {
    if (!m_pNdviReader->GetEntriesForField(fieldInfo.fieldSeqId, fieldInfo.ndviLines)) {
        otbAppLogWARNING("Cannot extract NDVI infos for the field " << fieldInfo.fieldId);
        return false;
    }
    if (fieldInfo.ndviLines.size() == 0) {
        otbAppLogWARNING("No valid lines were extracted for the field id " << fieldInfo.fieldId);
    }
    return true;
}

bool TsaDataExtractor::CheckWeekGaps(int &vegetationStartWeek, const std::vector<InputFileLineInfoType> &inLineInfos)
{
    if (inLineInfos.size() == 0) {
        return false;
    }

    std::vector<InputFileLineInfoType> lineInfos;
    lineInfos.insert(lineInfos.end(), inLineInfos.begin(), inLineInfos.end());

    // sort the read lines information
    std::sort (lineInfos.begin(), lineInfos.end(), TimedValInfosComparator<InputFileLineInfoType>());

    bool gaps = false;
    int validLines = 0;
    int prevWeekNo = -1;

    typename std::vector<InputFileLineInfoType>::const_iterator infosIter;
    for (infosIter = lineInfos.begin(); infosIter != lineInfos.end(); ++infosIter)
    {
        if (vegetationStartWeek <= infosIter->weekNo) {
            // we found a valid line
            if (prevWeekNo == -1) {
                prevWeekNo = infosIter->weekNo;
                continue;
            }
            if (infosIter->weekNo - prevWeekNo > 1) {
                gaps = true;
                break;
            }
            prevWeekNo = infosIter->weekNo;
            // we ignore the first valid line
            validLines++;
        }
    }
    if (gaps || validLines == 0) {
        return false;
    }

    return true;
}

