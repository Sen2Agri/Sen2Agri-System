/*=========================================================================
  *
  * Program:      Sen2agri-Processors
  * Language:     C++
  * Copyright:    2015-2016, CS Romania, office@c-s.ro
  * See COPYRIGHT file for details.
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.

 =========================================================================*/
#include "TsaDebugPrinter.h"
#include "TimeSeriesAnalysisUtils.h"

#include <boost/filesystem.hpp>

TsaDebugPrinter::TsaDebugPrinter() : m_bDebugMode(false) {

}

void TsaDebugPrinter::PrintFieldGeneralInfos(const FieldInfoType &fieldInfos) {
    if (!m_bDebugMode) {
        return;
    }
    std::cout << "Field ID: " << fieldInfos.fieldId << std::endl;
    std::cout << "Vegetation start: " << TimeToString(fieldInfos.ttVegStartTime) << std::endl;
    std::cout << "Vegetation start floor time: " << TimeToString(fieldInfos.ttVegStartWeekFloorTime) << std::endl;
    std::cout << "Vegetation start WEEK No: " <<  fieldInfos.vegStartWeekNo << std::endl;
    std::cout << "Harvest start: " << TimeToString(fieldInfos.ttHarvestStartTime) << std::endl;
    std::cout << "Harvest start floor time: " << TimeToString(fieldInfos.ttHarvestStartWeekFloorTime) << std::endl;
    std::cout << "Harvest start WEEK No: " <<  fieldInfos.harvestStartWeekNo << std::endl;
    std::cout << "Harvest end : " << TimeToString(fieldInfos.ttHarvestEndTime) << std::endl;
    std::cout << "Harvest end floor time: " << TimeToString(fieldInfos.ttHarvestEndWeekFloorTime) << std::endl;

}
void TsaDebugPrinter::PrintMergedValues(const std::vector<MergedAllValInfosType> &mergedVals, double ampThrValue) {
    if (!m_bDebugMode) {
        return;
    }

    std::cout << "Printing merged values" << std::endl;
    std::cout << "Amplitude threshold value is: " << ValueToString(ampThrValue) << std::endl;

    std::cout << "   Date  coh.max   coh.change  grd.mean  grd.max grd.change " << std::endl;

    for(size_t i = 0; i < mergedVals.size(); i++) {
        const MergedAllValInfosType &val = mergedVals[i];
        std::cout << i + 1 << " " <<
                     TimeToString(val.ttDate) << " " <<
                     ValueToString(val.cohMax) << " " <<
                     ValueToString(val.cohChange) << " " <<
                     ValueToString(val.ampMean) << " " <<
                     ValueToString(val.ampMax) << " " <<
                     ValueToString(val.ampChange) << std::endl;
    }
    std::cout << "   ndvi.mean ndvi.prev   ndvi.next  veg.weeks ndvi.presence  ndvi.drop" << std::endl;
    for(size_t i = 0; i < mergedVals.size(); i++) {
        const MergedAllValInfosType &val = mergedVals[i];
        std::cout << i + 1 << " " <<
                     ValueToString(val.ndviMeanVal) << " " <<
                     ValueToString(val.ndviPrev) << " " <<
                     ValueToString(val.ndviNext) << " " <<
                     ValueToString(val.vegWeeks, true) << " " <<
                     ValueToString(val.ndviPresence, true) << " " <<
                     ValueToString(val.ndviDrop, true) << " " <<
                     std::endl;
    }
    std::cout << "   candidate.opt coh.base   coh.high  coh.presence candidate.coh  candidate.grd" << std::endl;
    for(size_t i = 0; i < mergedVals.size(); i++) {
        const MergedAllValInfosType &val = mergedVals[i];
        std::cout << i + 1 << " " <<
                     ValueToString(val.candidateOptical, true) << " " <<
                     ValueToString(val.coherenceBase, true) << " " <<
                     ValueToString(val.coherenceHigh, true) << " " <<
                     ValueToString(val.coherencePresence, true) << " " <<
                     ValueToString(val.candidateCoherence, true) << " " <<
                     ValueToString(val.candidateAmplitude, true) << " " <<
                     std::endl;
    }
    std::cout << "   grd.presence " << std::endl;
    for(size_t i = 0; i < mergedVals.size(); i++) {
        const MergedAllValInfosType &val = mergedVals[i];
        std::cout << i + 1 << " " <<
                     ValueToString(val.amplitudePresence, true) << " " <<
                     std::endl;
    }

}

void TsaDebugPrinter::PrintAmplitudeInfos(const FieldInfoType &fieldInfos) {
    if (!m_bDebugMode) {
        return;
    }

    std::cout << "Printing amplitude values:" << std::endl;
    for (size_t i = 0; i<fieldInfos.mergedAmpInfos.size(); i++) {
        const MergedDateAmplitudeType &val = fieldInfos.mergedAmpInfos[i];
        std::cout << i + 1 << " " <<
                     TimeToString(val.ttDate) << " " <<
                     ValueToString(val.vvInfo.meanVal) << " " <<
                     ValueToString(val.vvInfo.stdDev) << " " <<
                     ValueToString(val.vvInfo.weekNo) << " " <<
                     TimeToString(val.vvInfo.ttDateFloor) << " " <<

                     ValueToString(val.vhInfo.meanVal) << " " <<
                     ValueToString(val.vhInfo.stdDev) << " " <<
                     ValueToString(val.vhInfo.weekNo) << " " <<
                     TimeToString(val.vhInfo.ttDateFloor) << " " <<

                     ValueToString(val.ampRatio) << std::endl;
    }
}

void TsaDebugPrinter::PrintAmpGroupedMeanValues(const std::vector<GroupedMeanValInfosType> &values) {
    if (!m_bDebugMode) {
        return;
    }

    std::cout << "Printing amplitude grouped mean values:" << std::endl;
    for (size_t i = 0; i<values.size(); i++) {
        const GroupedMeanValInfosType &val = values[i];
        std::cout << i + 1 << " " <<
                     TimeToString(val.ttDate) << " " <<
                     ValueToString(val.meanVal) << " " <<
                     ValueToString(val.maxVal) << " " <<
                     ValueToString(val.ampChange) << std::endl;
    }
}

void TsaDebugPrinter::PrintNdviInfos(const FieldInfoType &fieldInfos) {
    if (!m_bDebugMode) {
        return;
    }

    std::cout << "Printing NDVI values:" << std::endl;
    for (size_t i = 0; i<fieldInfos.ndviLines.size(); i++) {
        const InputFileLineInfoType &val = fieldInfos.ndviLines[i];
        std::cout << i + 1 << " " <<
                     TimeToString(val.ttDate) << " " <<
                     ValueToString(val.meanVal) << " " <<
                     ValueToString(val.stdDev) << " " <<
                     ValueToString(val.weekNo) << " " <<
                     TimeToString(val.ttDateFloor) << std::endl;
    }
}

void TsaDebugPrinter::PrintNdviGroupedMeanValues(const std::vector<GroupedMeanValInfosType> &values) {
    if (!m_bDebugMode) {
        return;
    }

    std::cout << "Printing NDVI Grouped mean values:" << std::endl;
    for (size_t i = 0; i<values.size(); i++) {
        const GroupedMeanValInfosType &val = values[i];
        std::cout << i + 1 << " " <<
                     TimeToString(val.ttDate) << " " <<
                     ValueToString(val.meanVal) << std::endl;
    }
}

void TsaDebugPrinter::PrintCoherenceInfos(const FieldInfoType &fieldInfos) {
    if (!m_bDebugMode) {
        return;
    }

    std::cout << "Printing Coherence values:" << std::endl;
    for (size_t i = 0; i<fieldInfos.coheVVLines.size(); i++) {
        const InputFileLineInfoType &val = fieldInfos.coheVVLines[i];
        std::cout << i + 1 << " " <<
                     TimeToString(val.ttDate) << " " <<
                     TimeToString(val.ttDate2) << " " <<
                     ValueToString(val.meanVal) << " " <<
                     ValueToString(val.stdDev) << " " <<
                     ValueToString(val.weekNo) << " " <<
                     TimeToString(val.ttDateFloor) << " " <<
                     ValueToString(val.meanValChange) << " " <<
                     std::endl;
    }
}

void TsaDebugPrinter::PrintCoherenceGroupedMeanValues(const std::vector<GroupedMeanValInfosType> &values) {
    if (!m_bDebugMode) {
        return;
    }

    std::cout << "Printing Coherence Grouped Mean values:" << std::endl;
    for (size_t i = 0; i<values.size(); i++) {
        const GroupedMeanValInfosType &val = values[i];
        std::cout << i + 1 << " " <<
                     TimeToString(val.ttDate) << " " <<
                     ValueToString(val.maxVal) << " " <<
                     ValueToString(val.maxChangeVal) << std::endl;
    }
}

void TsaDebugPrinter::PrintHarvestEvaluation(const FieldInfoType &fieldInfo, HarvestInfoType &harvestInfo) {
    if (!m_bDebugMode) {
        return;
    }

    std::cout << "Printing Harvest Evaluation:" << std::endl;
    std::cout << "FIELD_ID COUNTRY YEAR MAIN_CROP VEG_START H_START H_END PRACTICE" << std::endl;
    std::cout << 0 << fieldInfo.fieldId << " " << fieldInfo.countryCode << " " << fieldInfo.mainCrop << " " <<
                 // VEG_START H_START H_END;"
                 TimeToString(fieldInfo.ttVegStartTime) << " " << TimeToString(fieldInfo.ttHarvestStartTime) << " " <<
                 // H_END PRACTICE
                 TimeToString(fieldInfo.ttHarvestEndTime) << " " << fieldInfo.practiceName << std::endl;

    std::cout << "P_TYPE P_START P_END M1 M2 M3 M4 M5 H_WEEK M6 M7 M8 M9" << std::endl;
    // "P_TYPE P_START P_END;
    std::cout << fieldInfo.practiceName << " " << fieldInfo.practiceType << " " << TimeToString(fieldInfo.ttPracticeStartTime) << " " <<
                 TimeToString(fieldInfo.ttPracticeEndTime) << " " <<
                // M1;M2;
                ValueToString(harvestInfo.evaluation.ndviPresence, true) << " " << ValueToString(harvestInfo.evaluation.candidateOptical, true) << " " <<
                // M3;M4;
                ValueToString(harvestInfo.evaluation.candidateAmplitude, true) << " " << ValueToString(harvestInfo.evaluation.amplitudePresence, true) << " " <<
                // M5;H_WEEK;
                ValueToString(harvestInfo.evaluation.candidateCoherence, true) << " " << ValueToString(harvestInfo.evaluation.harvestConfirmWeek) << " "
              << std::endl;

    std::cout << "M10 C_INDEX" << std::endl;

    //                     // M6;M7;
    //                     efaHarvestInfo.evaluation.ndviPresence << ";" << efaHarvestInfo.evaluation.ndviGrowth << ";" <<
    //                     // M8;M9;
    //                     efaHarvestInfo.evaluation.ndviNoLoss << ";" << efaHarvestInfo.evaluation.ampNoLoss << ";" <<
    //                      //M10;C_INDEX
    //                     efaHarvestInfo.evaluation.cohNoLoss << ";" << efaHarvestInfo.evaluation.efaIndex

}


void TsaDebugPrinter::PrintEfaMarkers(const std::vector<MergedAllValInfosType> &allMergedValues,
                     const std::vector<EfaMarkersInfoType> &efaMarkers) {
    if (!m_bDebugMode) {
        return;
    }
    std::cout << "Printing EFA Markers:" << std::endl;
    std::cout << "Group.1   coh.max coh.change grd.mean  grd.max ndvi.mean ndvi.presence" << std::endl;
    for (size_t i = 0; i<efaMarkers.size(); i++) {
        std::cout << i+1 << " " << TimeToString(allMergedValues[i].ttDate) <<  " " << ValueToString(allMergedValues[i].cohMax) <<  " "
                  << ValueToString(allMergedValues[i].cohChange) << " " << ValueToString(allMergedValues[i].ampMean) << " " <<
                     ValueToString(allMergedValues[i].ampMax) <<  " " << ValueToString(allMergedValues[i].ndviMeanVal) << " " <<
                     ValueToString(efaMarkers[i].ndviPresence, true)
                  << std::endl;
    }
    std::cout << "ndvi.drop ndvi.growth ndvi.noloss grd.noloss coh.noloss" << std::endl;
    for (size_t i = 0; i<efaMarkers.size(); i++) {
        std::cout << i + 1 << " " << ValueToString(efaMarkers[i].ndviDrop, true) << " " << ValueToString(efaMarkers[i].ndviGrowth, true) << " "  <<
                     ValueToString(efaMarkers[i].ndviNoLoss, true) << " "  << ValueToString(efaMarkers[i].ampNoLoss, true) << " "  <<
                     ValueToString(efaMarkers[i].cohNoLoss, true) << " "
                     << std::endl;
    }
}

