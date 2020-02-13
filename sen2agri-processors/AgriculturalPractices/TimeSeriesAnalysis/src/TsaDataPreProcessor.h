#ifndef TsaDataExtrPreProcessor_H
#define TsaDataExtrPreProcessor_H

#include "itkLogger.h"
#include "itkMacro.h"
#include "otbWrapperMacros.h"

#include "TimeSeriesAnalysisUtils.h"
#include "TimeSeriesAnalysisTypes.h"
#include "TsaDebugPrinter.h"

class TsaDataExtrPreProcessor
{
public:
    TsaDataExtrPreProcessor(itk::Logger* logger);
    void SetVerbose(bool verbose) { m_bVerbose = verbose; }

    bool GroupAndMergeAllData(const FieldInfoType &fieldInfos, const std::vector<InputFileLineInfoType> &ampVHLines,
                              const std::vector<InputFileLineInfoType> &ampVVLines,
                              const std::vector<InputFileLineInfoType> &ndviLines, const std::vector<InputFileLineInfoType> &coheVVLines,
                              std::vector<MergedDateAmplitudeType> &mergedAmpInfos, std::vector<GroupedMeanValInfosType> &ampRatioGroups,
                              std::vector<GroupedMeanValInfosType> &ndviGroups, std::vector<GroupedMeanValInfosType> &coherenceGroups,
                              std::vector<MergedAllValInfosType> &allMergedValues);
    bool GroupAndMergeFilteredData(const FieldInfoType &fieldInfos, time_t ttStartTime, time_t ttEndTime,
                                   std::vector<MergedDateAmplitudeType> &mergedAmpInfos,
                                   std::vector<MergedAllValInfosType> &allMergedValues);

    itk::Logger* GetLogger() { return m_pLogger; }

private:
    bool MergeAmplitudes(const std::string &fieldId, const std::vector<InputFileLineInfoType> &ampVVLines,
                         const std::vector<InputFileLineInfoType> &ampVHLines,
                         std::vector<MergedDateAmplitudeType> &retInfos);

    template <typename TimedValue>
    bool GroupTimedValuesByWeeks(const std::vector<TimedValue> &infos,
                                std::vector<GroupedMeanValInfosType> &retGroups);
    bool Compute3WeeksAmplRatioDiffs(std::vector<GroupedMeanValInfosType> &ampRatioGroups);

    bool MergeAmpCoheFieldInfos(const FieldInfoType &fieldInfos, const std::vector<GroupedMeanValInfosType> &ampRatioGroups,
                            const std::vector<GroupedMeanValInfosType> &coherenceGroups,
                            std::vector<MergedAllValInfosType> &retAllMergedValues);
    bool MergeAllFieldInfos(const FieldInfoType &fieldInfos, const std::vector<GroupedMeanValInfosType> &ampRatioGroups,
                            const std::vector<GroupedMeanValInfosType> &coherenceGroups,
                            const std::vector<GroupedMeanValInfosType> &ndviGroups,
                            std::vector<MergedAllValInfosType> &retAllMergedValues);
    TsaDebugPrinter m_debugPrinter;
    bool m_bVerbose;
    itk::Logger* m_pLogger;
};
#endif
