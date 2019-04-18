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

#ifndef __COMMONFUNCTIONS_H
#define __COMMONFUNCTIONS_H

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

namespace otb
{
    typedef struct {
        std::vector<std::string> bandsNames;
        bool bUseNdvi;
        bool bUseRvi;
        std::vector<std::string> rsrColumnFilterBandNames;
        std::string laiModelFilePath;
        std::string faparModelFilePath;
        std::string fcoverModelFilePath;
    } LAIBandsConfigInfos;

    std::vector<std::string> split(std::string str, char delimiter) {
      std::vector<std::string> internal;
      std::stringstream ss(str); // Turn the string into a stream.
      std::string tok;

      while(std::getline(ss, tok, delimiter)) {
        internal.push_back(tok);
      }

      return internal;
    }

    LAIBandsConfigInfos loadLaiBandsConfigFile(const std::string &fileName)
    {
      std::ifstream bandCfgFile;
      bool bHasBandsKey = false;
      LAIBandsConfigInfos infos = {{}, false, false, {}};
      try {
          bandCfgFile.open(fileName.c_str());
          std::string cfgLine;
          while ( std::getline(bandCfgFile, cfgLine))
          {
              boost::trim(cfgLine);
              // skip empty or comment lines
              if (cfgLine.empty() || cfgLine[0] == '#') {
                  continue;
              }
              std::vector<std::string> keyValVect = split(cfgLine, '=');
              if (keyValVect.size() == 2) {
                  std::string key = keyValVect[0];
                  boost::trim(key);
                  std::string value = keyValVect[1];
                  boost::trim(value);
                  if (key == "LAI_BANDS") {
                      bHasBandsKey = true;
                      const std::vector<std::string> &bandNamesVect = split(value, ',');
                      for (size_t i = 0; i < bandNamesVect.size(); i++) {
                          try {
                              std::string bandName = bandNamesVect[i];
                              boost::trim(bandName);
                              if(std::find(infos.bandsNames.begin(), infos.bandsNames.end(), bandName) != infos.bandsNames.end()) {
                                  std::cout << "WARNING: LAI_BANDS contains twice the value " << bandName << std::endl;
                              } else {
                                  infos.bandsNames.push_back(bandName);
                              }
                          } catch (...) {
                              std::cout << "WARNING: Invalid band index found in LAI_BANDS key! Ignored!" << std::endl;
                          }
                      }
                  } else if (key == "USE_NDVI") {
                      if (boost::iequals(value, "true") || boost::iequals(value, "yes") || (value == "1")) {
                          infos.bUseNdvi = true;
                      }
                  } else if (key == "USE_RVI") {
                      if (boost::iequals(value, "true") || boost::iequals(value, "yes") || (value == "1")) {
                          infos.bUseRvi = true;
                      }
                  } else if (key == "RSR_COLS_FILTER") {
                      const std::vector<std::string> &bandNamesVect = split(value, ',');
                      for (size_t i = 0; i < bandNamesVect.size(); i++) {
                          try {
                              std::string bandName = bandNamesVect[i];
                              boost::trim(bandName);
                              if(std::find(infos.rsrColumnFilterBandNames.begin(), infos.rsrColumnFilterBandNames.end(), bandName) != infos.rsrColumnFilterBandNames.end()) {
                                  std::cout << "WARNING: RSR_COLS_FILTER contains twice the value " << bandName << std::endl;
                              } else {
                                  infos.rsrColumnFilterBandNames.push_back(bandName);
                              }
                          } catch (...) {
                              std::cout << "WARNING: Invalid band index found in RSR_COLS_FILTER key! Ignored!" << std::endl;
                          }
                      }
                  } else if (key == "LAI_MODEL_PATH") {
                      infos.laiModelFilePath = value;
                  } else if (key == "FAPAR_MODEL_PATH") {
                      infos.faparModelFilePath = value;
                  } else if (key == "FCOVER_MODEL_PATH") {
                      infos.fcoverModelFilePath = value;
                  }
              }
          }
      } catch(...) {
          // If not possible to open it, then use all bands
          std::cout << "WARNING: Cannot find or parse the file name " << fileName << ". All bands (except blue band,NDVI and RVI) will be used!" << std::endl;
      }
      // if LAI_BANDS is missing or file completely missing/not loaded, then initialize the bands with the default values
      // The bands will not be initialized if explicitly the key is empty
      if (!bHasBandsKey) {
          //The blue band (B2) will not be used due to residual atmospheric efects. B8 will not be used because of its
          //overlap with B7 anb B8a. Therefore, B3, B4, B5, B6, B7, B8a, B11 and B12 will be used.
          std::cout << "No RSR columns specified. Using default ones ..." << std::endl;
          infos.bandsNames = {"B3", "B4", "B5", "B6", "B7", "B8A", "B11", "B12"};
      }
      // also sort the array of bands
      //std::sort (infos.bandsIdxs.begin(), infos.bandsIdxs.end());
      //std::sort (infos.rsrColumnFilterIdxs.begin(), infos.rsrColumnFilterIdxs.end());

      // print the indexes
      std::cout << "======================" << std::endl;
      std::cout << "Add NDVI:" << infos.bUseNdvi << std::endl;
      std::cout << "Add RVI:" << infos.bUseRvi << std::endl;
      std::cout << "Loaded bands names:" << std::endl;
      for(size_t i = 0; i < infos.bandsNames.size(); i++) {
          std::cout << " " << infos.bandsNames[i] << std::endl;
      }
      std::cout << " " << std::endl;
      std::cout << "Using RSR bands names:" << std::endl;
      for(size_t i = 0; i < infos.rsrColumnFilterBandNames.size(); i++) {
          std::cout << " " << infos.rsrColumnFilterBandNames[i] << std::endl;
      }
      std::cout << " " << std::endl;
      std::cout << "======================" << std::endl;

      return infos;
    }

    bool IsMatchingMission(const std::string &missionName, const std::string &missionRegex) {
        if (missionName == missionRegex) {
            return true;
        }
        try {
            boost::regex re(missionRegex);
            return boost::regex_match(missionName, re);
        } catch (boost::regex_error& e) {
            std::cerr << "Invalid regular expression found in configuration file for " << missionRegex <<
                              ". The exception was \"" << e.what() << "\"";
            throw e;
        }
    }

    std::string getValueFromMissionsCfgFile(const std::string &fileName, const std::string &productMissionName,
                                     const std::string &productInstrumentName)
    {
        std::ifstream  data(fileName);
        if(data.is_open()) {
            std::cout << "Loading file: " << fileName << std::endl;
            std::cout << "Product mission is " << productMissionName << std::endl;
            std::cout << "Product instrument is " << productInstrumentName << std::endl;
            std::string line;
            while(std::getline(data,line)) {
                boost::trim(line);
                // skip empty or comment lines
                if (line.empty() || line[0] == '#') {
                    continue;
                }
                std::cout << "Checking line: " << line << std::endl;
                size_t lastindex = line.find_last_of("=");
                if((lastindex != std::string::npos) && (lastindex != (line.length()-1))) {
                    const std::string &keyStr = line.substr(0, lastindex);
                    std::string cfgFileName = line.substr(lastindex+1);
                    std::string missionName = keyStr;   // by default, mission name is the key
                    std::string sensorInstrument;

                    size_t diezIndex = keyStr.find_last_of("#");
                    // Check if actually we have MISSION#INSTRUMENT
                    if((diezIndex != std::string::npos) && (diezIndex != (keyStr.length()-1))) {
                        missionName = keyStr.substr(0, diezIndex);
                        sensorInstrument = keyStr.substr(diezIndex+1);
                    }

                    if(IsMatchingMission(productMissionName, missionName) &&
                            ((sensorInstrument == "") || (productInstrumentName == sensorInstrument))) {
                        std::cout << "Found configured file!"<< std::endl;

                        boost::filesystem::path cfgFilePath(cfgFileName);
                        if (cfgFilePath.is_relative()) {
                            boost::filesystem::path containerFilePath(fileName);
                            containerFilePath.remove_filename();
                            cfgFilePath = containerFilePath / cfgFilePath;
                            cfgFileName = cfgFilePath.string();
                        }

                        return cfgFileName;
                    }
                }
            }
        } else {
            itkGenericExceptionMacro(<< "Could not open RSR configuration file " << fileName);
        }

        return "";
    }

}

#endif
