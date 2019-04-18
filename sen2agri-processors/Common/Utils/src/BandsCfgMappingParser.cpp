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
 
#include "BandsCfgMappingParser.h"
#include <fstream>
#include <sstream>
#include <itkMacro.h>
#include <algorithm>    // std::sort
#include <boost/regex.hpp>

/********************************************************************
 *
 * BandsMappingConfig functions
 *
 ********************************************************************/
void BandMappingConfig::AddBandConfig(bool bMaster, const std::string &bandName, int res) {
    BandConfig bandCfg;
    bandCfg.bIsMaster = bMaster;
    bandCfg.bandName = bandName;
    bandCfg.res = res;
    m_bandsCfg.push_back(bandCfg);
}

BandConfig BandMappingConfig::GetMasterBand() {
    return m_bandsCfg[0];
}

BandConfig BandMappingConfig::GetBand(int idx) {
    return m_bandsCfg[idx];
}

unsigned int BandMappingConfig::GetBandsNo() {
    return m_bandsCfg.size();
}

int BandMappingConfig::GetMasterBandResolution() {
    return m_bandsCfg[0].res;
}

bool BandMappingConfig::operator<( const BandMappingConfig & other ) const
{
  return (this->m_bandsCfg[0].bandName < other.m_bandsCfg[0].bandName);
}


/********************************************************************
 *
 * BandsMappingConfig functions
 *
 ********************************************************************/
std::vector<BandMappingConfig> BandsMappingConfig::GetBandMappingConfigs(int nRes) {
    std::vector<BandMappingConfig> retMappings;
    for(unsigned int i = 0; i<m_bandsCfgMapping.size(); i++) {
        if(nRes == m_bandsCfgMapping[i].GetMasterBandResolution()) {
            retMappings.push_back(m_bandsCfgMapping[i]);
        }
    }
    return retMappings;
}

std::vector<BandConfig> BandsMappingConfig::GetBands(int nRes, const std::string &missionName) {
    std::vector<BandConfig> retBands;
    int nMissionIdx = -1;
    for(unsigned int i = 0; i<m_missionNames.size(); i++) {
        if(IsMatchingMission(missionName, m_missionNames[i]) ) {
            nMissionIdx = i;
            break;
        }
    }
    if(nMissionIdx >= 0) {
        for(unsigned int i = 0; i<m_bandsCfgMapping.size(); i++) {
            if(nRes == m_bandsCfgMapping[i].GetMasterBandResolution()) {
                retBands.push_back(m_bandsCfgMapping[i].GetBand(nMissionIdx));
            }
        }
    }
    return retBands;
}

void BandsMappingConfig::AddBandsCfgMapping (const BandMappingConfig& bandsMappingCfg) {
    m_bandsCfgMapping.push_back(bandsMappingCfg);
    // No need to sort as in this case B11 for example can be added before B1
    // So, we'll assume that the bands are already in the right order in file
    //std::sort (m_bandsCfgMapping.begin(), m_bandsCfgMapping.end());
}

void BandsMappingConfig::AddMission(const std::string &mission) {
    // TODO: Check that the mission does not already exists
    m_missionNames.push_back(mission);
}

unsigned int BandsMappingConfig::GetMissionsNo() {
    return m_missionNames.size();
}

bool BandsMappingConfig::IsMasterMission(const std::string &missionName) const {
    return IsMatchingMission(missionName, m_missionNames[0]);
}

std::string BandsMappingConfig::GetMasterMissionName() {
    return m_missionNames[0];
}

bool BandsMappingConfig::IsConfiguredMission(const std::string &missionName) {
    for(const std::string &mission: m_missionNames) {
        if(IsMatchingMission(missionName, mission)) {
            return true;
        }
    }
    return false;
}

// Returns the bands indexes of a mission. If a secondary mission has no index for the one in master,
// a -1 is filled only if bIgnoreMissing is not set otherwise it is not added in the returned vector
std::vector<std::string> BandsMappingConfig::GetBandNames(int res, const std::string &missionName, bool bIgnoreMissing) {
    std::vector<std::string> retBandNames;
    if(!IsConfiguredMission(missionName)) {
        itkExceptionMacro("Mission " + missionName + " is not configured for composition!");
    }

    std::vector<BandConfig> bandsCfg = GetBands(res, missionName);
    if(bandsCfg.size() <=0) {
        itkExceptionMacro("No bands configured for this resolution " << res << " and mission " << missionName);
    }
    for(unsigned int i = 0; i<bandsCfg.size(); i++) {
        if(bandsCfg[i].bandName.size() == 0 || bandsCfg[i].bandName == "-1") {
            if(bIgnoreMissing) {
                // we ignore the bands that are in master but not in the secondary product
                continue;
            }
        }
        retBandNames.push_back(bandsCfg[i].bandName);
    }
    return retBandNames;
}

std::vector<int> BandsMappingConfig::GetMasterBandsPresence(int nRes, int &outNbValidBands) {
    return GetBandsPresence(nRes, GetMasterMissionName(), outNbValidBands);
}

/* This function does not returns the indexes from the file but the valid indexes in
 * sequencial ascending order and -1 if missing band */
std::vector<int> BandsMappingConfig::GetBandsPresence(int nRes, const std::string &missionName, int &outNbValidBands) {
    if(!IsConfiguredMission(missionName)) {
        itkExceptionMacro("Mission " + missionName + " is not configured for composition!");
    }

    const std::string &masterMissionName = GetMasterMissionName();
    const std::vector<BandConfig> &masterBandsCfg = GetBands(nRes, masterMissionName);
    const std::vector<BandConfig> &bandsCfg = GetBands(nRes, missionName);
    if((bandsCfg.size() <= 0) || (masterBandsCfg.size() != bandsCfg.size())) {
        itkExceptionMacro("Invalid bands size configuration for resolution " << nRes << " and mission " << missionName);
    }
    outNbValidBands = 0;
    // create an array of bands presences with the same size as the master band size
    std::vector<int> bandsPresenceVect(bandsCfg.size());
    for(unsigned int i = 0; i<bandsCfg.size(); i++) {
        if(bandsCfg[i].bandName.size() == 0 || bandsCfg[i].bandName == "-1") {
            // we mark the bands that are in the master product but not in the secondary product
            bandsPresenceVect[i] = -1;
            continue;
        }
        bandsPresenceVect[i] = outNbValidBands;
        outNbValidBands++;
    }
    return bandsPresenceVect;
}

std::string BandsMappingConfig::GetMasterBandName(const std::string &missionName, int nRes, const std::string &sensorBandName)
{
    const std::string &masterMissionName = GetMasterMissionName();
    const std::vector<BandConfig> &bandsCfg = GetBands(nRes, missionName);
    const std::vector<BandConfig> &masterBandsCfg = GetBands(nRes, masterMissionName);
    if(bandsCfg.size() != masterBandsCfg.size()) {
        itkExceptionMacro("Invalid bands size configuration for resolution " << nRes << ". Number differ from master for mission " << missionName);
    }
    for(unsigned int i = 0; i< bandsCfg.size(); i++) {
        if(bandsCfg[i].bandName == sensorBandName) {
            return masterBandsCfg[i].bandName;
        }
    }
    return "";
}

int BandsMappingConfig::GetIndexInMasterPresenceArray(int nRes, const std::string &bandName)
{
    return GetIndexInPresenceArray(nRes, GetMasterMissionName(), bandName);
}

int BandsMappingConfig::GetIndexInPresenceArray(int nRes, const std::string &missionName, const std::string &bandName)
{
    int nbValidBands;

    std::vector<int> bandsPresenceVect = GetBandsPresence(nRes, missionName, nbValidBands);
    std::vector<std::string> bandNamesVect = GetBandNames(nRes, missionName, false);
    if(bandsPresenceVect.size() != bandNamesVect.size()) {
        itkExceptionMacro("Wrong number of bands configured for master mission !");
    }

    for(unsigned int i = 0; i<bandsPresenceVect.size(); i++) {
        if(bandsPresenceVect[i] != -1) {
            if(bandNamesVect[i] == bandName) {
                return bandsPresenceVect[i];
            }
        }
    }
    return -1;
}

bool BandsMappingConfig::IsMatchingMission(const std::string &missionName, const std::string &missionRegex) const {
    if (missionName == missionRegex) {
        return true;
    }
    try {
        boost::regex re(missionRegex);
        return boost::regex_match(missionName, re);
    } catch (boost::regex_error& e) {
        std::cerr << "Invalid regular expression found in bands mapping file for " << missionRegex <<
                          ". The exception was \"" << e.what() << "\"";
        throw e;
    }
}

/********************************************************************
 *
 * BandsCfgMappingParser functions
 *
 ********************************************************************/
BandsCfgMappingParser::BandsCfgMappingParser()
{
}

void BandsCfgMappingParser::ParseFile(const std::string &fileName) {
    std::ifstream  data(fileName);
    std::string line;

    int lineCnt = 0;
    while(std::getline(data,line)) {
        std::stringstream  lineStream(line);
        std::string        cell;
        if(lineCnt == 0) {
            // here we need to extract the mission names
            int cellIdx = 0;
            while(std::getline(lineStream,cell,',')) {
                cell = trim(cell);
                if(cell == "") {
                    itkExceptionMacro("Invalid mission name found at pos " << cellIdx);
                }
                m_bandMappingCfg.AddMission(cell);
                cellIdx++;
            }
        } else {
            // here we need to extract the band mappings
            BandMappingConfig bandMappingCfg;
            int cellIdx = 0;
            std::string masterBandName;
            int masterRes = -1;
            while(std::getline(lineStream,cell,',')) {
                // TODO: The first 2 columns are resolution and index of the master band
                // the next pairs of columns are the index and the presence of the secondary products
                // TODO: perform validations
                cell = trim(cell);
                if(cellIdx == 0) {
                    masterBandName = cell;
                    if(masterBandName.size() == 0) {
                        itkExceptionMacro("Master band name should be provided at " <<
                                          lineCnt << " position " << cellIdx);
                    }
                } else if (cellIdx == 1) {
                    masterRes = std::stoi(cell);
                    if(masterRes <= 0) {
                        itkExceptionMacro("Master band resolution should be greater than 0. The value " << cell <<
                                          " was found at " << lineCnt << " position " << cellIdx);
                    }
                    bandMappingCfg.AddBandConfig(true, masterBandName, masterRes);
                } else {
                    const std::string &secondaryBandName = cell;
                    bandMappingCfg.AddBandConfig(false, secondaryBandName, masterRes);
                }
                cellIdx++;
            }
            if(bandMappingCfg.GetBandsNo() == 0) {
                itkExceptionMacro("You should have at least 2 columns on each line. Invalid configuration found at line " << lineCnt << " at position " << cellIdx);
            }
            if(bandMappingCfg.GetBandsNo() != m_bandMappingCfg.GetMissionsNo()) {
                itkExceptionMacro("You should have a number of columns equals with the number of missions (first row) + 1");
            }
            m_bandMappingCfg.AddBandsCfgMapping(bandMappingCfg);
        }
        lineCnt++;
    }
}

std::string BandsCfgMappingParser::trim(std::string const& str, const std::string strChars)
{
    if(str.empty())
        return str;

    int nbChars=strChars.length();
    std::string retStr = str;
    for(int i = 0; i<nbChars; i++) {
        int curChar = strChars.at(i);
        std::size_t firstScan = retStr.find_first_not_of(curChar);
        std::size_t first     = firstScan == std::string::npos ? retStr.length() : firstScan;
        std::size_t last      = retStr.find_last_not_of(curChar);
        retStr = retStr.substr(first, last-first+1);
    }
    return retStr;
}

