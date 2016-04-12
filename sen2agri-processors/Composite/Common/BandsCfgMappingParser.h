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
 
#ifndef BANDS_CFG_MAPPING_PARSER_H
#define BANDS_CFG_MAPPING_PARSER_H

#include <string>
#include <vector>
#include "itkMacro.h"

class BandConfig {

public:
    int identifier;
    int res;
    bool bIsMaster;
};

class BandMappingConfig {
public:
    void AddBandConfig(bool bMaster, int bandIdx, int res);
    BandConfig GetMasterBand();
    BandConfig GetBand(int idx);
    unsigned int GetBandsNo();
    int GetMasterBandResolution();
    bool operator<( const BandMappingConfig & other ) const;

private:
    std::vector<BandConfig> m_bandsCfg;
};

class BandsMappingConfig {
public:
    const char * GetNameOfClass() { return "BandsMappingConfig"; }
    std::vector<BandMappingConfig> GetBandMappingConfigs(int nRes);
    std::vector<BandConfig> GetBands(int nRes, const std::string &missionName);
    void AddBandsCfgMapping (const BandMappingConfig& bandsMappingCfg);
    void AddMission(std::string &mission);
    unsigned int GetMissionsNo();
    std::string GetMasterMissionName();
    bool IsConfiguredMission(const std::string &missionName);

    std::vector<int> GetAbsoluteBandIndexes(int res, const std::string &missionName, bool bIgnoreMissing = true);
    /* This function does not returns the indexes from the file but the valid indexes in
     * sequencial ascending order and -1 if missing band */
    std::vector<int> GetBandsPresence(int nRes, const std::string &missionName, int &outNbValidBands);
    int GetMasterBandIndex(const std::string &missionName, int nRes, int nSensorBandIdx);
    int GetIndexInPresenceArray(int nRes, const std::string &missionName, int absIdx);

private:
    std::vector<std::string> m_missionNames;
    // we know that the first band is always the master band
    std::vector<BandMappingConfig> m_bandsCfgMapping;
};

class BandsCfgMappingParser
{
public:
    BandsCfgMappingParser();
    void ParseFile(const std::string &fileName);
    const BandsMappingConfig& GetBandsMappingCfg() { return m_bandMappingCfg; }
    const char * GetNameOfClass() { return "BandsCfgMappingParser"; }

private:
    BandsMappingConfig m_bandMappingCfg;
    std::string trim(std::string const& str, const std::string strChars=" \r");
};

#endif // BANDS_CFG_MAPPING_PARSER_H
