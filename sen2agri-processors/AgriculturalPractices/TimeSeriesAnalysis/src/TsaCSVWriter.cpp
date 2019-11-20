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
#include "TsaCSVWriter.h"
#include "TimeSeriesAnalysisUtils.h"

#include <boost/filesystem.hpp>

TsaCSVWriter::TsaCSVWriter() {
    m_IndexPossibleVals = {{"STRONG", 1}, {"MODERATE", 1}, {"WEAK", 1}, {"POOR", 1}};
    m_HWeekInvalidVals = {{"NA", 1}, {"NA1", 1}, {"NO-HARVEST", 1}};
}

std::string TsaCSVWriter::BuildResultsCsvFileName(const std::string &practiceName, const std::string &countryCode,
                                  int year) {
    const std::string &fileName = "Sen4CAP_L4C_" + practiceName + "_" +
            countryCode + "_" + std::to_string(year) + "_CSV.csv";
    return fileName;
}

bool TsaCSVWriter::WriteCSVHeader(const std::string &outDir, const std::string &practiceName, const std::string &countryCode,
                    int year) {
    if (m_OutFileStream.is_open()) {
        return true;
    }
    const std::string &fullPath = GetResultsCsvFilePath(outDir, practiceName, countryCode, year);
    m_OutFileStream.open(fullPath, std::ios_base::trunc | std::ios_base::out);
    if( m_OutFileStream.fail() ) {
        //otbAppLogFATAL("Error opening output file " << fullPath << ". Exiting...");
        return false;
    }

    // # create result csv file for harvest and EFA practice evaluation
    m_OutFileStream << "FIELD_ID;ORIG_ID;COUNTRY;YEAR;MAIN_CROP;VEG_START;H_START;H_END;"
               "PRACTICE;P_TYPE;P_START;P_END;L_WEEK;M1;M2;M3;M4;M5;H_WEEK;H_W_START;H_W_END;H_W_S1;M6;M7;M8;M9;M10;C_INDEX;"
               "S1PIX;S1GAPS;H_S1GAPS;P_S1GAPS;H_W_S1GAPS;H_QUALITY;C_QUALITY\n";

    return true;
}

void TsaCSVWriter::WriteHarvestInfoToCsv(const FieldInfoType &fieldInfo, const HarvestEvaluationInfoType &harvestEvalInfo,
                                         const HarvestEvaluationInfoType &efaHarvestEvalInfo) {
    //"FIELD_ID;COUNTRY;YEAR;MAIN_CROP
    m_OutFileStream << fieldInfo.fieldSeqId << ";" << fieldInfo.fieldId << ";" << fieldInfo.countryCode << ";" << ValueToString(fieldInfo.year) << ";" << fieldInfo.mainCrop << ";" <<
               // VEG_START;H_START;H_END;"
               TimeToString(fieldInfo.ttVegStartTime) << ";" << TimeToString(fieldInfo.ttHarvestStartTime) << ";" << TimeToString(fieldInfo.ttHarvestEndTime) << ";" <<
               // "PRACTICE;P_TYPE;P_START;P_END;
               fieldInfo.practiceName << ";" << fieldInfo.practiceType << ";" << TimeToString(efaHarvestEvalInfo.ttPracticeStartTime) << ";" <<
               TimeToString(efaHarvestEvalInfo.ttPracticeEndTime) << ";" <<
               // L_WEEK
               TimeToString(harvestEvalInfo.ttLWeekStartTime) << ";" <<
               // M1;M2;
               ValueToString(harvestEvalInfo.ndviPresence, true) << ";" << ValueToString(harvestEvalInfo.candidateOptical, true) << ";" <<
               // M3;M4;
               ValueToString(harvestEvalInfo.candidateAmplitude, true) << ";" << ValueToString(harvestEvalInfo.amplitudePresence, true) << ";" <<
               // M5;
               ValueToString(harvestEvalInfo.candidateCoherence, true) << ";" <<
               //H_WEEK;
               TranslateHWeekNrDate(ValueToString(harvestEvalInfo.harvestConfirmWeek)) << ";" <<
               // H_W_START;
               TranslateHWeekNrDate(TimeToString(harvestEvalInfo.ttHarvestConfirmWeekStart)) << ";" <<
               // H_W_END;
               TranslateHWeekNrDate(TimeToString((IsNA(harvestEvalInfo.ttHarvestConfirmWeekStart) || harvestEvalInfo.ttHarvestConfirmWeekStart == 0) ?
                               harvestEvalInfo.ttHarvestConfirmWeekStart :
                               harvestEvalInfo.ttHarvestConfirmWeekStart + (6 * SEC_IN_DAY))) << ";" <<
               // H_W_S1;
               TranslateHWeekNrDate(TimeToString(harvestEvalInfo.ttS1HarvestWeekStart)) << ";" <<
               // M6;M7;
               ValueToString(efaHarvestEvalInfo.ndviPresence, true) << ";" << ValueToString(efaHarvestEvalInfo.ndviGrowth, true) << ";" <<
               // M8;M9;
               ValueToString(efaHarvestEvalInfo.ndviNoLoss, true) << ";" << ValueToString(efaHarvestEvalInfo.ampNoLoss, true) << ";" <<
               //M10;C_INDEX;
               ValueToString(efaHarvestEvalInfo.cohNoLoss, true) << ";" << efaHarvestEvalInfo.efaIndex << ";" <<
               // S1PIX;S1GAPS
               fieldInfo.s1PixValue << ";" << ValueToString(fieldInfo.gapsInfos) <<  ";" <<
               // H_S1GAPS;P_S1GAPS
               ValueToString(fieldInfo.hS1GapsInfos) <<  ";"  << ValueToString(fieldInfo.pS1GapsInfos) << ";" <<
               GetHWS1Gaps(fieldInfo, harvestEvalInfo, efaHarvestEvalInfo) << ";" <<
               GetHQuality(fieldInfo, harvestEvalInfo, efaHarvestEvalInfo) << ";" <<
               GetCQuality(fieldInfo, harvestEvalInfo, efaHarvestEvalInfo) <<
              "\n";
    m_OutFileStream.flush();
}

std::string TsaCSVWriter::GetResultsCsvFilePath(const std::string &outDir, const std::string &practiceName, const std::string &countryCode,
                                  int year) {
    const std::string &fileName = BuildResultsCsvFileName(practiceName, countryCode, year);
    boost::filesystem::path rootFolder(outDir);
    return (rootFolder / fileName).string();
}

std::string TsaCSVWriter::TranslateHWeekNrDate(const std::string &strHDate) {
    if (strHDate == "NR") {
        return "NO-HARVEST";
    }
    return strHDate;
}

std::string TsaCSVWriter::GetHWS1Gaps(const FieldInfoType &fieldInfo, const HarvestEvaluationInfoType &harvestEvalInfo,
                                         const HarvestEvaluationInfoType &efaHarvestEvalInfo) {
    if (!IsNA(harvestEvalInfo.harvestConfirmWeek) && harvestEvalInfo.harvestConfirmWeek > 0) {
        return ValueToString(fieldInfo.h_W_S1GapsInfos);
    }
    return ValueToString(harvestEvalInfo.harvestConfirmWeek);
}

std::string TsaCSVWriter::GetHQuality(const FieldInfoType &fieldInfo, const HarvestEvaluationInfoType &harvestEvalInfo,
                                         const HarvestEvaluationInfoType &efaHarvestEvalInfo) {
    const std::string &s1HwStart = TimeToString(harvestEvalInfo.ttS1HarvestWeekStart);
    if (m_HWeekInvalidVals.find(s1HwStart) == m_HWeekInvalidVals.end()) {
        if (!IsNA(harvestEvalInfo.harvestConfirmWeek) && harvestEvalInfo.harvestConfirmWeek > 0) {
            if (fieldInfo.h_W_S1GapsInfos >= 2) {
                return "1";
            }
        }
    }
    return "";
}

std::string TsaCSVWriter::GetCQuality(const FieldInfoType &fieldInfo, const HarvestEvaluationInfoType &harvestEvalInfo,
                                         const HarvestEvaluationInfoType &efaHarvestEvalInfo) {

//    if (m_IndexPossibleVals.find(efaHarvestInfo.efaIndex) != m_IndexPossibleVals.end()) {
    if (fieldInfo.pS1GapsInfos >= 2) {
        return "1";
    }
//    }
    return "";
}

