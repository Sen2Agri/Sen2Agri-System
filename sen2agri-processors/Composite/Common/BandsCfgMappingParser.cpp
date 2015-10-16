#include "BandsCfgMappingParser.h"
#include <fstream>
#include <sstream>
#include <itkMacro.h>

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
            // here we need to extract the mission names
            BandMappingConfig bandMappingCfg;
            int cellIdx = 0;
            int masterIdx;
            int masterRes;
            while(std::getline(lineStream,cell,',')) {
                // TODO: The first 2 columns are resolution and index of the master band
                // the next pairs of columns are the index and the presence of the secondary products
                // TODO: perform validations
                cell = trim(cell);
                if(cellIdx == 0) {
                    masterIdx = std::stoi(cell);
                    if(masterIdx <= 0) {
                        itkExceptionMacro("Master band index should be greater than 0. The value " << cell <<
                                          " was found at " << lineCnt << " position " << cellIdx);
                    }
                } else if (cellIdx == 1) {
                    masterRes = std::stoi(cell);
                    if(masterRes <= 0) {
                        itkExceptionMacro("Master band resolution should be greater than 0. The value " << cell <<
                                          " was found at " << lineCnt << " position " << cellIdx);
                    }
                    bandMappingCfg.AddBandConfig(true, masterIdx, masterRes);
                } else {
                    int secondaryIdx = std::stoi(cell);
                    bandMappingCfg.AddBandConfig(false, secondaryIdx, masterRes);
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

