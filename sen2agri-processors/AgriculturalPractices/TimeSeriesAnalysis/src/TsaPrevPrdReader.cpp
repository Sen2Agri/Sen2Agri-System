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

TsaPrevPrdReader::TsaPrevPrdReader() : m_fieldIdPos(-1), m_hEndPos(-1) {

}
bool TsaPrevPrdReader::Initialize(const std::string &prevPrd) {
    std::ifstream fStream(prevPrd);
    if (!fStream.is_open()) {
        std::cout << "Error opennig file " << prevPrd << std::endl;
        return false;
    }
    std::string line;
    if (std::getline(fStream, line)) {

        ExtractHeaderInfos(line);
        if (m_fieldIdPos != -1 && m_hEndPos != -1) {
            while (std::getline(fStream, line)) {
                const std::vector<std::string> &vect = LineToVector(line);
                if ((int)vect.size() > m_fieldIdPos && (int)vect.size() > m_hEndPos) {
                    // extract the ID and the date into the map
                    m_MapIdHendWeek[std::atoi(vect[m_fieldIdPos].c_str())] = vect[m_hEndPos];
                }
            }
        }
    }
    return true;
}

std::string TsaPrevPrdReader::GetHWeekForFieldId(int fieldId) {
    if (m_MapIdHendWeek.size() > 0) {
        std::map<int,std::string>::iterator it = m_MapIdHendWeek.find(fieldId);
        if (it != m_MapIdHendWeek.end()) {
            return it->second;
        }
    }
    return "";
}

void TsaPrevPrdReader::ExtractHeaderInfos(const std::string &line)
{
    m_InputFileHeader = LineToVector(line);
    m_fieldIdPos = GetPosInVector(m_InputFileHeader, "FIELD_ID");
    m_hEndPos = GetPosInVector(m_InputFileHeader, "H_END");
}

