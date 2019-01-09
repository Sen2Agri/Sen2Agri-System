#include "StatisticsInfosXmlReader.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>

#include <cinttypes>

#include "TimeSeriesAnalysisUtils.h"
#include "CommonFunctions.h"

typedef struct XmlFieldInfos {
    XmlFieldInfos() {
        fidStarted = false;
    }
    std::string fieldId;
    std::vector<std::string> filters;

    std::map<std::string, std::vector<InputFileLineInfoType>> retMap;
    std::string curFid;
    std::string curFilter;
    bool fidStarted;
    bool useDate2;
    bool switchDates;

} XmlFieldInfos;

#define BUFFSIZE 2048*2048

static void startFn(void *pUserData, const char* pElement, const char** pAttributes)
{
    XmlFieldInfos* infos = (XmlFieldInfos*)pUserData;
    if (strcmp(pElement, "fid") == 0) {
        // we have a fid starting
        infos->curFid.clear();
        infos->curFilter.clear();
        infos->fidStarted = false;

        std::string Fid;
        std::string pName;
        for( int i = 0; pAttributes[i] != NULL; i+=2) {
            if (strcmp(pAttributes[i], "id") == 0) {
                std::string tmpFid = pAttributes[i+1];
                // Normalize the field id
                NormalizeFieldId(tmpFid);
                if (tmpFid == infos->fieldId) {
                    // we have a valid fid.
                    Fid = tmpFid;
                }
            } else if (strcmp(pAttributes[i], "name") == 0) {
                // name attribute -> check for filters
                if (infos->filters.size() == 0) {
                    pName = pAttributes[i+1];
                } else {
                    for(const std::string &strFilter: infos->filters) {
                        if (strstr(pAttributes[i+1], strFilter.c_str()) != NULL) {
                            pName = pAttributes[i+1];
                            infos->curFilter = strFilter;
                            break;
                        }
                    }
                }
            }
            //std::cout << "\tAttribute : " << pAttributes[i] << std::endl;
        }
        // if they passed the filtering step
        if (Fid.size() > 0 && pName.size()) {
            infos->curFid = Fid;
            infos->fidStarted = true;
        }
    } else if (infos->fidStarted && strcmp(pElement, "info") == 0) {
        InputFileLineInfoType curInfo;
        curInfo.Reset();
        curInfo.fieldId = infos->curFid;

        for( int i = 0; pAttributes[i] != NULL; i+=2) {
            if (strcmp(pAttributes[i], "date") == 0) {
                std::string strDate(pAttributes[i+1]);
                int weekNo, yearNo;
                if (GetWeekFromDate(strDate, yearNo, weekNo, INPUT_FILE_DATE_PATTERN))
                {
                    curInfo.strDate = strDate;
                    curInfo.ttDate = GetTimeFromString(strDate);
                    curInfo.weekNo = weekNo;
                    curInfo.ttDateFloor = FloorDateToWeekStart(curInfo.ttDate);
                } else {
                    std::cout << "Invalid date format found  " << strDate <<std::endl;
                }
            } else if (strcmp(pAttributes[i], "mean") == 0) {
                std::string::size_type sz;
                curInfo.meanVal = std::stod(pAttributes[i+1], &sz);
            } else if (strcmp(pAttributes[i], "stdev") == 0) {
                std::string::size_type sz;
                curInfo.stdDev = std::stod(pAttributes[i+1], &sz);
            } else if (infos->useDate2 && strcmp(pAttributes[i], "date2") == 0) {
                std::string strDate(pAttributes[i+1]);
                int weekNo, yearNo;
                if (GetWeekFromDate(strDate, yearNo, weekNo, INPUT_FILE_DATE_PATTERN))
                {
                    if (infos->switchDates) {
                        // Here we switch the dates
                        curInfo.strDate2 = curInfo.strDate;
                        curInfo.ttDate2 = curInfo.ttDate;
                        curInfo.weekNo2 = curInfo.weekNo;
                        curInfo.ttDate2Floor = curInfo.ttDateFloor;

                        curInfo.strDate = strDate;
                        curInfo.ttDate = GetTimeFromString(strDate);
                        curInfo.weekNo = weekNo;
                        curInfo.ttDateFloor = FloorDateToWeekStart(curInfo.ttDate);
                    } else {
                        curInfo.strDate2 = strDate;
                        curInfo.ttDate2 = GetTimeFromString(strDate);
                        curInfo.weekNo2 = weekNo;
                        curInfo.ttDate2Floor = FloorDateToWeekStart(curInfo.ttDate2);
                    }

                } else {
                    std::cout << "Invalid date 2 format found : " << strDate << std::endl;
                }
            }
        }

        // save the current infos in the ret map, if they exist
        if (infos->fieldId.size() > 0 && curInfo.strDate.size() > 0) {
            if (infos->filters.size() == 0 || (infos->filters.size() > 0 && infos->curFilter.size() > 0)) {
                // get the existing map for the filter
                std::map<std::string, std::vector<InputFileLineInfoType>>::iterator i = infos->retMap.find(infos->curFilter);
                if (i == infos->retMap.end()) {
                    std::vector<InputFileLineInfoType> mapCurInfos;
                    mapCurInfos.push_back(curInfo);
                    infos->retMap[infos->curFilter] = mapCurInfos;
                }
                else {
                    // TODO: Check for duplicates!!!
                    i->second.push_back(curInfo);
                }
            } else {
                std::cout << "Something wrong ... current filter empty when having multiple filters!!!" << std::endl;
            }
        }

    }
}

static void endFn(void *pUserData, const char* pElement)
{
    (void)pUserData;
    (void)pElement;
}

static void characterFn( void *pUserData, const char* pCharacterData, int length )
{
    (void)pUserData;
    (void)pCharacterData;
    (void)length;
}


StatisticsInfosXmlReader::StatisticsInfosXmlReader()
{
}

void StatisticsInfosXmlReader::Initialize(const std::string &source, const std::vector<std::string> &filters, int year)
{
    m_strSource = source;
    m_year = year;
    // check if index file exists near the source xml
    const std::string &idxFilePath(source + ".idx");
    if ( boost::filesystem::exists(idxFilePath)) {
        std::cout << "Loading indexes file " << idxFilePath << std::endl;
        // load the indexes
        std::ifstream idxFileStream(idxFilePath);
        std::string line;
        while (std::getline(idxFileStream, line)) {
            const std::vector<std::string> &tokens = split (line, ';');
            if (tokens.size() == 4) {
                std::string key = tokens[0];
                NormalizeFieldId(key);
                FieldIndexInfos idxInfos;
                idxInfos.name = tokens[1];
                idxInfos.startIdx = std::strtoumax(tokens[2].c_str(), nullptr, 10);
                idxInfos.len = std::atoi(tokens[3].c_str());

                IdxMapType::iterator itMap = m_IdxMap.find(key);
                if (itMap != m_IdxMap.end()) {
                    itMap->second.push_back(idxInfos);
                } else {
                    std::vector<FieldIndexInfos> infos = {idxInfos};
                    m_IdxMap[key] = infos;
                }
            }
        }
        std::cout << "Loading indexes file done!" << std::endl;

    }
}

bool StatisticsInfosXmlReader::GetEntriesForField(const std::string &inFieldId, const std::vector<std::string> &filters,
                                                          std::map<std::string, std::vector<InputFileLineInfoType>> &retMap)
{
    // Normalize the field ID when searching in the file
    std::string fieldId = inFieldId;
    NormalizeFieldId(fieldId);

    std::ifstream ifs( m_strSource.c_str() );
    if( ifs.fail() ) {
        std::cout << "Error opening input file, exiting..." << std::endl;
        return false;
    }

    XML_Parser p = XML_ParserCreate(NULL);
    if (! p) {
        std::cout << "Failed to create parser" << std::endl;
        return false;
    }
    XmlFieldInfos xmlFieldsInfos;
    xmlFieldsInfos.switchDates = m_bSwitchDates;
    xmlFieldsInfos.useDate2 = m_bUseDate2;
    xmlFieldsInfos.fieldId = fieldId;
    xmlFieldsInfos.filters = filters;

    XML_SetUserData( p, &xmlFieldsInfos );
    XML_SetElementHandler(p, startFn, endFn);
    XML_SetCharacterDataHandler(p, characterFn);

    // parser ready and raring to go.
    if (m_IdxMap.size() == 0) {
        bool done = false;
        int len = 0;
        int totalCount = len;
        char buff[BUFFSIZE];
        while( !done ) {
            ifs.read( buff, BUFFSIZE );
            done = ( (len = ifs.gcount()) < BUFFSIZE);
            totalCount += len;
            if( ifs.bad() ) {
                std::cerr << "Error in read operation." << std::endl;
                return false;
            }
            if (! XML_Parse(p, buff, len, done)) {
                std::cerr << "Parse error at line " << XML_GetCurrentLineNumber(p);
                std::cerr << " with " << XML_ErrorString(XML_GetErrorCode(p))
                          << std::endl;
                return false;
            }
       }

    } else {
        // we have indexed field in XML
        // TODO: quick workaround ... replace
        const std::vector<std::string> &emptyVect = {""};
        const std::vector<std::string> &findFilters = filters.size() > 0 ? filters : emptyVect;
        IdxMapType::const_iterator it = m_IdxMap.find(fieldId);
        if (it != m_IdxMap.end()) {
            for (const FieldIndexInfos &info: it->second) {
                for (const std::string & filter: findFilters) {
                    if (info.name.find(filter) != std::string::npos) {
                        XML_ParserReset(p, NULL);
                        XML_SetUserData( p, &xmlFieldsInfos );
                        XML_SetElementHandler(p, startFn, endFn);
                        XML_SetCharacterDataHandler(p, characterFn);

                        ifs.seekg(info.startIdx, std::ios::beg);
                        char buff[info.len+1];
                         ifs.read(buff, info.len);
                         buff[info.len] = 0;
                         bool done = false;
        //                 std::cout << buff << std::endl;
        //                 std::cout << "Done!" << std::endl;
                         if (! XML_Parse(p, buff, info.len, done)) {
                             std::cerr << "Parse error at line " << XML_GetCurrentLineNumber(p);
                             std::cerr << " with " << XML_ErrorString(XML_GetErrorCode(p))
                                       << std::endl;
                             return false;
                         }
                    }
                }
            }
        }
    }

   // free the parser when we’ve finished with it
   XML_ParserFree(p);

    std::map<std::string, std::vector<InputFileLineInfoType>>::iterator itMap;
    for (itMap = xmlFieldsInfos.retMap.begin(); itMap != xmlFieldsInfos.retMap.end(); ++itMap) {
        if (m_minReqEntries > 0 && (int)itMap->second.size() < m_minReqEntries) {
            // if one of the filters has less values than needed
            return false;
        }
        // sort the read lines information
        std::sort (itMap->second.begin(), itMap->second.end(), InputFileLineInfoComparator());

        // compute the change from the previous value
        // TODO: needed only by the coherence
        for (size_t i = 0; i<itMap->second.size(); i++) {
            if (i >=1) {
                double diff = itMap->second[i].meanVal - itMap->second[i-1].meanVal;
                itMap->second[i].meanValChange = (diff < 0 ? 0 : diff);
            }
        }
    }
    retMap.insert(xmlFieldsInfos.retMap.begin(), xmlFieldsInfos.retMap.end());

    return true;
/*

    bool ret = true;

    const std::vector<FileInfoType> &foundFiles = FindFilesForFieldId(fieldId);
    std::vector<FileInfoType>::const_iterator it;
    for (it = foundFiles.begin(); it != foundFiles.end(); ++it) {
        if (filters.size() > 0) {
            std::vector<std::string>::const_iterator itF;
            for (itF = filters.begin(); itF != filters.end(); ++itF) {
                if (!ExtractFileInfosForFilter(*it, *itF, retMap)) {
                    ret = false;
                }
            }
        } else {
            std::string emptyF;
            if (!ExtractFileInfosForFilter(*it, emptyF, retMap)) {
                ret = false;
            }
        }
    }

    return ret;
*/
}

