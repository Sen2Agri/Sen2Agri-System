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
#include "TsaContinuousFileWriter.h"
#include "TimeSeriesAnalysisUtils.h"

#include <boost/filesystem.hpp>

TsaContinuousFileWriter::TsaContinuousFileWriter() : m_bResultContinuousProduct(false) {

}
void TsaContinuousFileWriter::CreateContinousProductFile(const std::string &outDir, const std::string &practiceName, const std::string &countryCode, int year) {
    if (!m_bResultContinuousProduct) {
        return;
    }
    if (m_OutContinousPrdFileStream.is_open()) {
        return;
    }
    const std::string &fullPath = GetContinousProductCsvFilePath(outDir, practiceName, countryCode, year);
    m_OutContinousPrdFileStream.open(fullPath, std::ios_base::trunc | std::ios_base::out);

    // # create continous product result csv file header
    m_OutContinousPrdFileStream << "FIELD_ID;ORIG_ID;WEEK;M1;M2;M3;M4;M5\n";
}

void TsaContinuousFileWriter::WriteContinousToCsv(const FieldInfoType &fieldInfo, const std::vector<MergedAllValInfosType> &allMergedVals) {
    if (!m_bResultContinuousProduct) {
        return;
    }

    std::vector<MergedAllValInfosType>::const_iterator it;
    for (it = allMergedVals.begin(); it != allMergedVals.end(); ++it) {
       //"FIELD_ID;Week
        m_OutContinousPrdFileStream << fieldInfo.fieldSeqId << ";" << fieldInfo.fieldId << ";" << ValueToString(GetWeekFromDate(it->ttDate)) << ";" <<
               // M1;M2;
               ValueToString(it->ndviPresence, true) << ";" << ValueToString(it->candidateOptical, true) << ";" <<
               // M3;M4;
               ValueToString(it->candidateAmplitude, true) << ";" << ValueToString(it->amplitudePresence, true) << ";" <<
               // M5
               ValueToString(it->candidateCoherence, true) << "\n";
    }
}

std::string TsaContinuousFileWriter::GetContinousProductCsvFilePath(const std::string &outDir, const std::string &practiceName, const std::string &countryCode,
                                  int year) {
    const std::string &fileName = "Sen4CAP_L4C_" + practiceName + "_" +
            countryCode + "_" + std::to_string(year) + "_CSV_ContinousProduct.csv";
    boost::filesystem::path rootFolder(outDir);
    return (rootFolder / fileName).string();
}


