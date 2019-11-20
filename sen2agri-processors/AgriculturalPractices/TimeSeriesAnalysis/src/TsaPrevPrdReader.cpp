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
#include "TsaPrevPrdReader.h"
#include "TimeSeriesAnalysisUtils.h"

TsaPrevPrdReader::TsaPrevPrdReader() : m_fieldIdPos(-1), m_hWeekPos(-1), m_hWeekStartPos(-1), m_hWS1GapsPos(-1) {

}
bool TsaPrevPrdReader::Initialize(const std::string &prevPrd) {
    std::ifstream fStream(prevPrd);
    if (!fStream.is_open()) {
        std::cout << "ERROR: Cannot open PREVIOUS product file " << prevPrd << std::endl;
        return false;
    }
    std::string line;
    if (std::getline(fStream, line)) {

        ExtractHeaderInfos(line);

        if (m_fieldIdPos != -1) {
            while (std::getline(fStream, line)) {
                const std::vector<std::string> &vect = LineToVector(line);
                if ((int)vect.size() > m_fieldIdPos) {
                    if (m_hWeekPos != -1 && m_hWeekStartPos != -1 &&
                            (int)vect.size() > m_hWeekPos && (int)vect.size() > m_hWeekStartPos) {
                        // extract the ID and the date into the map
                        m_MapIdHendWeek[std::atoi(vect[m_fieldIdPos].c_str())] = {vect[m_hWeekPos], vect[m_hWeekStartPos]};
                    }
                    // check the H_W_S1GAPS
                    if (m_hWS1GapsPos != -1 && (int)vect.size() > m_hWS1GapsPos) {
                        // extract the ID and the date into the map
                        m_MapIdHWS1Gaps[std::atoi(vect[m_fieldIdPos].c_str())] = vect[m_hWS1GapsPos];
                    }
                }
            }
        }
    }
    return true;
}

std::string TsaPrevPrdReader::GetHWeekForFieldId(int fieldId) {
    if (m_MapIdHendWeek.size() > 0) {
        std::map<int,FieldInfos>::iterator it = m_MapIdHendWeek.find(fieldId);
        if (it != m_MapIdHendWeek.end()) {
            return it->second.hWeek;
        }
    }
    return "";
}

std::string TsaPrevPrdReader::GetHWeekStartForFieldId(int fieldId) {
    if (m_MapIdHendWeek.size() > 0) {
        std::map<int,FieldInfos>::iterator it = m_MapIdHendWeek.find(fieldId);
        if (it != m_MapIdHendWeek.end()) {
            return it->second.hWeekStart;
        }
    }
    return "";
}

std::string TsaPrevPrdReader::GetHWS1GapsInfosForFieldId(int fieldId) {
    if (m_MapIdHWS1Gaps.size() > 0) {
        std::map<int,std::string>::iterator it = m_MapIdHWS1Gaps.find(fieldId);
        if (it != m_MapIdHWS1Gaps.end()) {
            return it->second;
        }
    }
    return "";
}

void TsaPrevPrdReader::ExtractHeaderInfos(const std::string &line)
{
    m_InputFileHeader = LineToVector(line);
    m_fieldIdPos = GetPosInVector(m_InputFileHeader, "FIELD_ID");
    m_hWeekPos = GetPosInVector(m_InputFileHeader, "H_WEEK");
    m_hWeekStartPos = GetPosInVector(m_InputFileHeader, "H_W_START");
    m_hWS1GapsPos = GetPosInVector(m_InputFileHeader, "H_W_S1GAPS");
}

