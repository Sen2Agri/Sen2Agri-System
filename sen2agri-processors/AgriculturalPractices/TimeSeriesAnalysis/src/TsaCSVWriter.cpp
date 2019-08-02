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
               "S1PIX;S1GAPS;H_S1GAPS;P_S1GAPS\n";

    return true;
}

void TsaCSVWriter::WriteHarvestInfoToCsv(const FieldInfoType &fieldInfo, const HarvestInfoType &harvestInfo,
                                         const HarvestInfoType &efaHarvestInfo) {
    //"FIELD_ID;COUNTRY;YEAR;MAIN_CROP
    m_OutFileStream << fieldInfo.fieldSeqId << ";" << fieldInfo.fieldId << ";" << fieldInfo.countryCode << ";" << ValueToString(fieldInfo.year) << ";" << fieldInfo.mainCrop << ";" <<
               // VEG_START;H_START;H_END;"
               TimeToString(fieldInfo.ttVegStartTime) << ";" << TimeToString(fieldInfo.ttHarvestStartTime) << ";" << TimeToString(fieldInfo.ttHarvestEndTime) << ";" <<
               // "PRACTICE;P_TYPE;P_START;P_END;
               fieldInfo.practiceName << ";" << fieldInfo.practiceType << ";" << TimeToString(efaHarvestInfo.evaluation.ttPracticeStartTime) << ";" <<
               TimeToString(efaHarvestInfo.evaluation.ttPracticeEndTime) << ";" <<
               TimeToString(harvestInfo.evaluation.ttLWeekStartTime) << ";" <<
               // M1;M2;
               ValueToString(harvestInfo.evaluation.ndviPresence, true) << ";" << ValueToString(harvestInfo.evaluation.candidateOptical, true) << ";" <<
               // M3;M4;
               ValueToString(harvestInfo.evaluation.candidateAmplitude, true) << ";" << ValueToString(harvestInfo.evaluation.amplitudePresence, true) << ";" <<
               // M5;
               ValueToString(harvestInfo.evaluation.candidateCoherence, true) << ";" <<
               //H_WEEK;
               TranslateHWeekNrDate(ValueToString(harvestInfo.evaluation.harvestConfirmWeek)) << ";" <<
               // H_W_START;
               TranslateHWeekNrDate(TimeToString(harvestInfo.evaluation.ttHarvestConfirmWeekStart)) << ";" <<
               // H_W_END;
               TranslateHWeekNrDate(TimeToString((IsNA(harvestInfo.evaluation.ttHarvestConfirmWeekStart) || harvestInfo.evaluation.ttHarvestConfirmWeekStart == 0) ?
                               harvestInfo.evaluation.ttHarvestConfirmWeekStart :
                               harvestInfo.evaluation.ttHarvestConfirmWeekStart + (6 * SEC_IN_DAY))) << ";" <<
               // H_W_S1;
               TranslateHWeekNrDate(TimeToString(harvestInfo.evaluation.ttS1HarvestWeekStart)) << ";" <<
               // M6;M7;
               ValueToString(efaHarvestInfo.evaluation.ndviPresence, true) << ";" << ValueToString(efaHarvestInfo.evaluation.ndviGrowth, true) << ";" <<
               // M8;M9;
               ValueToString(efaHarvestInfo.evaluation.ndviNoLoss, true) << ";" << ValueToString(efaHarvestInfo.evaluation.ampNoLoss, true) << ";" <<
               //M10;C_INDEX;
               ValueToString(efaHarvestInfo.evaluation.cohNoLoss, true) << ";" << efaHarvestInfo.evaluation.efaIndex << ";" <<
               // S1PIX;S1GAPS
               fieldInfo.s1PixValue << ";" << ValueToString(fieldInfo.gapsInfos) <<  ";" <<
               // H_S1GAPS;P_S1GAPS
               ValueToString(fieldInfo.hS1GapsInfos) <<  ";"  << ValueToString(fieldInfo.pS1GapsInfos) << "\n";
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



