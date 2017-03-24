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

namespace otb
{
    typedef struct {
        std::vector<int> bandsIdxs;
        bool bUseNdvi;
        bool bUseRvi;
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
      LAIBandsConfigInfos infos = {{}, false, false};
      try {
          bandCfgFile.open(fileName.c_str());
          std::string cfgLine;
          while ( std::getline(bandCfgFile, cfgLine))
          {
              // skip empty lines:
              if (cfgLine.empty())
                  continue;
              std::vector<std::string> keyValVect = split(cfgLine, '=');
              if (keyValVect.size() == 2) {
                  std::string key = keyValVect[0];
                  boost::trim(key);
                  std::string value = keyValVect[1];
                  boost::trim(value);
                  if (key == "LAI_BANDS") {
                      bHasBandsKey = true;
                      std::vector<std::string> bandIdxsVect = split(value, ',');
                      for (size_t i = 0; i < bandIdxsVect.size(); i++) {
                          try {
                              int nValIdx = std::stoi(bandIdxsVect[i]);
                              if(std::find(infos.bandsIdxs.begin(), infos.bandsIdxs.end(), nValIdx) != infos.bandsIdxs.end()) {
                                  std::cout << "WARNING: LAI_BANDS contains twice the value " << nValIdx << std::endl;
                              } else {
                                  infos.bandsIdxs.push_back(nValIdx);
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
          //The blue band (B2) will not be used due to residual atmospheric eects. B8 will not be used because of its
          //overlap with B7 anb B8a. Therefore, B3, B4, B5, B6, B7, B8a, B11 and B12 will be used.
          // B8A has actually the index 9, B11->index 12 and B12 -> Index 13
          static const int arr[] = {3, 4, 5, 6, 7, 9, 12, 13};
          infos.bandsIdxs = std::vector<int> (arr, arr + sizeof(arr) / sizeof(arr[0]) );
      }
      // also sort the array of bands
      std::sort (infos.bandsIdxs.begin(), infos.bandsIdxs.end());
      return infos;
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
                std::cout << "Checking line: " << line << std::endl;
                size_t lastindex = line.find_last_of("=");
                if((lastindex != std::string::npos) && (lastindex != (line.length()-1))) {
                    std::string keyStr = line.substr(0, lastindex);
                    std::string cfgFileName = line.substr(lastindex+1);
                    std::string missionName = keyStr;   // by default, mission name is the key
                    std::string sensorInstrument;

                    size_t diezIndex = keyStr.find_last_of("#");
                    // Check if actually we have MISSION#INSTRUMENT
                    if((diezIndex != std::string::npos) && (diezIndex != (keyStr.length()-1))) {
                        missionName = keyStr.substr(0, diezIndex);
                        sensorInstrument = keyStr.substr(diezIndex+1);
                    }

                    if((productMissionName == missionName) &&
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
