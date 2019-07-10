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
#include "TsaPlotsWriter.h"
#include "TimeSeriesAnalysisUtils.h"

#include <boost/filesystem.hpp>

TsaPlotsWriter::TsaPlotsWriter() : m_bPlotOutputGraph(false), m_OutPlotsIdxCurIdx(0) {

}
void TsaPlotsWriter::CreatePlotsFile(const std::string &outDir, const std::string &practiceName, const std::string &countryCode, int year) {
    if (!m_bPlotOutputGraph) {
        return;
    }
    const std::string &fullPath = GetPlotsFilePath(outDir, practiceName, countryCode, year);
    //otbAppLogINFO("Creating plot file " << fullPath);
    m_OutPlotsFileStream.open(fullPath, std::ios_base::trunc | std::ios_base::out);
    std::string plotsStart("<plots>\n");
    m_OutPlotsFileStream << plotsStart.c_str();

    std::string outIdxPath;
    boost::filesystem::path path(fullPath);
    outIdxPath = (path.parent_path() / path.filename()).string() + ".idx";
    m_OutPlotsIdxFileStream.open(outIdxPath, std::ios_base::trunc | std::ios_base::out);
    // initialize also the index
    m_OutPlotsIdxCurIdx = plotsStart.size();
}

void TsaPlotsWriter::ClosePlotsFile() {
    if (!m_bPlotOutputGraph) {
        return;
    }
    //otbAppLogINFO("Closing plot file");
    m_OutPlotsFileStream << "</plots>\n";
    m_OutPlotsFileStream.flush();
    m_OutPlotsFileStream.close();

    m_OutPlotsIdxFileStream.flush();
    m_OutPlotsIdxFileStream.close();
}

void TsaPlotsWriter::WritePlotEntry(const FieldInfoType &fieldInfos, const HarvestInfoType &harvestInfo) {
    if (!m_bPlotOutputGraph) {
        return;
    }

    std::stringstream ss;

    ss << " <fid id=\"" << fieldInfos.fieldSeqId.c_str() << "\" orig_id=\"" << fieldInfos.fieldId.c_str() << "\">\n";
    ss << " <practice start=\"" << TimeToString(harvestInfo.evaluation.ttHarvestStartTime).c_str() <<
                           "\" end=\"" << TimeToString(harvestInfo.evaluation.ttHarvestEndTime).c_str() <<
                           "\"/>\n";
    ss << " <harvest start=\"" << TimeToString(harvestInfo.evaluation.ttHarvestConfirmWeekStart).c_str() <<
                          "\" end=\"" << TimeToString((IsNA(harvestInfo.evaluation.ttHarvestConfirmWeekStart) || harvestInfo.evaluation.ttHarvestConfirmWeekStart == 0) ?
                                        harvestInfo.evaluation.ttHarvestConfirmWeekStart :
                                        harvestInfo.evaluation.ttHarvestConfirmWeekStart + (6 * SEC_IN_DAY)).c_str() <<
                            "\"/>\n";

    ss << "  <ndvis>\n";
    for (size_t i = 0; i<fieldInfos.ndviLines.size(); i++) {
        ss << "   <ndvi date=\"" << fieldInfos.ndviLines[i].strDate.c_str() <<
                                "\" val=\"" << ValueToString(fieldInfos.ndviLines[i].meanVal).c_str() << "\"/>\n";
    }
    ss << "  </ndvis>\n";
    ss << "  <amps>\n";
    for (size_t i = 0; i<fieldInfos.mergedAmpInfos.size(); i++) {
        ss << "   <amp date=\"" << TimeToString(fieldInfos.mergedAmpInfos[i].ttDate).c_str() <<
                                "\" val=\"" << ValueToString(fieldInfos.mergedAmpInfos[i].ampRatio).c_str() << "\"/>\n";
    }
    ss << "  </amps>\n";
    ss << "  <cohs>\n";
    for (size_t i = 0; i<fieldInfos.coheVVLines.size(); i++) {
        ss << "   <coh date=\"" << fieldInfos.coheVVLines[i].strDate.c_str() <<
                                "\" val=\"" << ValueToString(fieldInfos.coheVVLines[i].meanVal).c_str() << "\"/>\n";
    }
    ss << "  </cohs>\n</fid>\n";

    const std::string &ssStr = ss.str();
    size_t byteToWrite = ssStr.size();
    if (m_OutPlotsIdxFileStream.is_open()) {
        m_OutPlotsIdxFileStream << fieldInfos.fieldSeqId.c_str() << ";" << m_OutPlotsIdxCurIdx << ";" << byteToWrite <<"\n";
    }
    m_OutPlotsIdxCurIdx += byteToWrite;
    m_OutPlotsFileStream << ssStr.c_str();
}

std::string TsaPlotsWriter::GetPlotsFilePath(const std::string &outDir, const std::string &practiceName, const std::string &countryCode,
                                  int year) {
    const std::string &fileName = "Sen4CAP_L4C_" + practiceName + "_" +
            countryCode + "_" + std::to_string(year) + "_PLOT.xml";
    boost::filesystem::path rootFolder(outDir);
    return (rootFolder / fileName).string();
}


