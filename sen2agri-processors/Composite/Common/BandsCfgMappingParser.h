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
    void AddBandConfig(bool bMaster, int bandIdx, int res) {
        BandConfig bandCfg;
        bandCfg.bIsMaster = bMaster;
        bandCfg.identifier = bandIdx;
        bandCfg.res = res;
        m_bandsCfg.push_back(bandCfg);
    }
    BandConfig GetMasterBand() { return m_bandsCfg[0]; }
    BandConfig GetBand(int idx) { return m_bandsCfg[idx]; }
    unsigned int GetBandsNo() { return m_bandsCfg.size(); }
    int GetMasterBandResolution() { return m_bandsCfg[0].res; }

private:
    std::vector<BandConfig> m_bandsCfg;
};

class BandsMappingConfig {
public:
    const char * GetNameOfClass() { return "BandsMappingConfig"; }

    std::vector<BandMappingConfig> GetBandMappingConfigs(int nRes) {
        std::vector<BandMappingConfig> retMappings;
        for(unsigned int i = 0; i<m_bandsCfgMapping.size(); i++) {
            if(nRes == m_bandsCfgMapping[i].GetMasterBandResolution()) {
                retMappings.push_back(m_bandsCfgMapping[i]);
            }
        }
        return retMappings;
    }

    std::vector<BandConfig> GetBands(int nRes, const std::string &missionName) {
        std::vector<BandConfig> retBands;
        int nMissionIdx = -1;
        for(unsigned int i = 0; i<m_missionNames.size(); i++) {
            if(m_missionNames[i] == missionName) {
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

    void AddBandsCfgMapping (const BandMappingConfig& bandsMappingCfg) {
        m_bandsCfgMapping.push_back(bandsMappingCfg);
    }

    void AddMission(std::string &mission) {
        // TODO: Check that the mission does not already exists
        m_missionNames.push_back(mission);
    }

    unsigned int GetMissionsNo() { return m_missionNames.size();}
    std::string GetMasterMissionName() { return m_missionNames[0];}
    std::string GetMissionsName(int pos) { return m_missionNames[pos];}
    bool IsMasterMission(const std::string &missionName) {return (m_missionNames[0] == missionName);}
    bool IsConfiguredMission(const std::string &missionName) {
        for(unsigned int i = 0; i<m_missionNames.size(); i++) {
            if(m_missionNames[i] == missionName) {
                return true;
            }
        }
        return false;
    }

    std::vector<int> GetAbsoluteBandIndexes(int res, const std::string &missionName) {
        std::vector<int> retIndexes;
        if(!IsConfiguredMission(missionName)) {
            itkExceptionMacro("Mission " + missionName + " is not configured for composition!");
        }

        std::vector<BandConfig> bandsCfg = GetBands(res, missionName);
        if(bandsCfg.size() <=0) {
            itkExceptionMacro("No bands configured for this resolution " << res << " and mission " << missionName);
        }
        for(unsigned int i = 0; i<bandsCfg.size(); i++) {
            int nBandIdx = bandsCfg[i].identifier;
            if(nBandIdx <= 0) {
                // we ignore the bands that are in master but not in the secondary product
                continue;
            }
            retIndexes.push_back(nBandIdx);
        }
        return retIndexes;
    }

    /* This function does not returns the indexes from the file but the valid indexes in
     * sequencial ascending order and -1 if missing band */
    std::vector<int> GetBandsPresence(int nRes, const std::string &missionName, int &outNbValidBands) {
        if(!IsConfiguredMission(missionName)) {
            itkExceptionMacro("Mission " + missionName + "is not configured for composition!");
        }

        std::string masterMissionName = GetMasterMissionName();
        std::vector<BandConfig> masterBandsCfg = GetBands(nRes, masterMissionName);
        std::vector<BandConfig> bandsCfg = GetBands(nRes, missionName);
        if((bandsCfg.size() <= 0) || (masterBandsCfg.size() != bandsCfg.size())) {
            itkExceptionMacro("Invalid bands size configuration for resolution " << nRes << " and mission " << missionName);
        }
        outNbValidBands = 0;
        // create an array of bands presences with the same size as the master band size
        std::vector<int> bandsPresenceVect(bandsCfg.size());
        for(unsigned int i = 0; i<bandsCfg.size(); i++) {
            if(bandsCfg[i].identifier <= 0) {
                // we mark the bands that are in the master product but not in the secondary product
                bandsPresenceVect[i] = -1;
                continue;
            }
            bandsPresenceVect[i] = outNbValidBands;
            outNbValidBands++;
        }
        return bandsPresenceVect;
    }

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
