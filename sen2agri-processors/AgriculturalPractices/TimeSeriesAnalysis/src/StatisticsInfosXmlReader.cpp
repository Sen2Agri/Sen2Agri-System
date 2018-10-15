#include "StatisticsInfosXmlReader.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>

#include "TimeSeriesAnalysisUtils.h"

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

} XmlFieldInfos;

#define BUFFSIZE 2048

static void NormalizeFieldId(std::string &fieldId) {
    std::replace( fieldId.begin(), fieldId.end(), '/', '_');
}

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
            } else if (strcmp(pAttributes[i], "date2") == 0) {
                std::string strDate(pAttributes[i+1]);
                int weekNo, yearNo;
                if (GetWeekFromDate(strDate, yearNo, weekNo, INPUT_FILE_DATE_PATTERN))
                {
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

void StatisticsInfosXmlReader::SetSource(const std::string &source)
{
    m_strSource = source;
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
    xmlFieldsInfos.fieldId = fieldId;
    xmlFieldsInfos.filters = filters;

    XML_SetUserData( p, &xmlFieldsInfos );
    XML_SetElementHandler(p, startFn, endFn);
    XML_SetCharacterDataHandler(p, characterFn);

    // parser ready and raring to go.
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

/*
bool StatisticsInfosXmlReader::ExtractFileInfosForFilter(const FileInfoType &fileInfo, const std::string &filter,
                                                                 std::map<std::string, std::vector<InputFileLineInfoType>> &retMap) {
    bool ret = true;

    // get the existing map for the filter
    if(filter.size() == 0 || fileInfo.fileName.find(filter) != std::string::npos) {
        // get the existing map for the filter
        std::map<std::string, std::vector<InputFileLineInfoType>>::iterator i = retMap.find(filter);
        if (i == retMap.end()) {
            std::vector<InputFileLineInfoType> infos;
            if (!ExtractLineInfos(fileInfo.filePath, infos)) {
                std::cout << "Error extracting infos from file " << fileInfo.fileName << " and filter " << filter << std::endl;
                ret = false;
            } else {
                // ignore files that do not have the minimum number of valid lines
                if (m_minReqEntries <= 0 || (int)infos.size() >= m_minReqEntries) {
                    retMap[filter] = infos;
                }
            }
        }
        else {
            std::vector<InputFileLineInfoType> infos;
            if (!ExtractLineInfos(fileInfo.filePath, infos)) {
                std::cout << "Error extracting infos from file " << fileInfo.fileName << " and filter " << filter << std::endl;
                ret = false;
            }
            // check if a minimum is required and if it is valid
            if (m_minReqEntries <= 0 || (int)infos.size() >= m_minReqEntries) {
                i->second.insert(i->second.end(), infos.begin(), infos.end());
            }
        }
    }

    return ret;
}

std::vector<FileInfoType> StatisticsInfosXmlReader::GetFilesInFolder(const std::string &targetPath) {
    std::vector<FileInfoType> allFolderFiles;
    boost::filesystem::directory_iterator end_itr; // Default ctor yields past-the-end
    for( boost::filesystem::directory_iterator i( targetPath ); i != end_itr; ++i )
    {
        // Skip if not a file
        if (!boost::filesystem::is_regular_file( i->status() ) )  {
            continue;
        }
        allFolderFiles.push_back({i->path().filename().string(), i->path().string()});
    }

    return allFolderFiles;
}

std::vector<FileInfoType> StatisticsInfosXmlReader::FindFilesForFieldId(const std::string &fieldId)
{
    std::vector<FileInfoType> allMatchingFiles;
    typename std::vector<FileInfoType>::const_iterator filesIterator;
    for (filesIterator = m_InfoFiles.begin(); filesIterator != m_InfoFiles.end(); ++filesIterator) {
        if (filesIterator->fileName.compare(0, fieldId.length(), fieldId) == 0) {
            allMatchingFiles.push_back(*filesIterator);
        }
    }
    return allMatchingFiles;
}

std::vector<std::string> StatisticsInfosXmlReader::GetInputFileLineElements(const std::string &line) {
    std::vector<std::string> results;
    boost::algorithm::split(results, line, [](char c){return c == ';';});
    return results;
}

bool StatisticsInfosXmlReader::ExtractInfosFromLine(const std::string &fileLine, InputFileLineInfoType &lineInfo)
{
    const std::vector<std::string> &lineElems = GetInputFileLineElements(fileLine);
    if (lineElems.size() != m_InputFileHeader.size() &&
        lineElems.size() != m_CoheInputFileHeader.size()) {
        //otbAppLogWARNING("Invalid line fields length for line " << fileLine);
        std::cout << "Invalid line fields length for line " << fileLine << std::endl;
        return false;
    }
    bool isCohe = (lineElems.size() == m_CoheInputFileHeader.size());
    int meanIdx = isCohe ? 3 : 2;
    int stdDevIdx = isCohe ? 4 : 3;
    std::string::size_type sz;     // alias of size_t
    const std::string &strDate = lineElems[1];
    int weekNo, yearNo;
    if (GetWeekFromDate(strDate, yearNo, weekNo, INPUT_FILE_DATE_PATTERN))
    {
        lineInfo.fielId = lineElems[0];
        lineInfo.strDate = strDate;
        lineInfo.ttDate = GetTimeFromString(strDate);
        lineInfo.weekNo = weekNo;
        lineInfo.ttDateFloor = FloorDateToWeekStart(lineInfo.ttDate);// FloorWeekDate(yearNo, weekNo);
        lineInfo.stdDev = std::stod(lineElems[stdDevIdx], &sz);
        lineInfo.meanVal = std::stod(lineElems[meanIdx], &sz);
    } else {
        //otbAppLogWARNING("Invalid date format found for line " << fileLine);
        std::cout << "Invalid date format found for line " << fileLine <<std::endl;
        return false;
    }

    if (isCohe)
    {
        const std::string &strDate2 = lineElems[2];
        int weekNo2, yearNo2;
        if (GetWeekFromDate(strDate2, yearNo2, weekNo2, INPUT_FILE_DATE_PATTERN)) {
            lineInfo.strDate2 = strDate2;
            lineInfo.ttDate2 = GetTimeFromString(strDate2);
            lineInfo.weekNo2 = weekNo2;
            lineInfo.ttDate2Floor = FloorDateToWeekStart(lineInfo.ttDate2);// FloorWeekDate(yearNo2, weekNo2);
        } else {
            //otbAppLogWARNING("Invalid date 2 format found for line " << fileLine);
            std::cout << "Invalid date 2 format found for line " << fileLine << std::endl;
            return false;
        }
    }

    return true;
}

bool StatisticsInfosXmlReader::ExtractLineInfos(const std::string &filePath, std::vector<InputFileLineInfoType> &retValidLines)
{
    std::ifstream ampFile(filePath);
    std::string line;
    int curLine = 0;
    std::map<std::string, int> extractedLines;
    while (std::getline(ampFile, line))
    {
        if(extractedLines.find(line) != extractedLines.end()) {
            // duplicate line found in file
            continue;
        }
        // add the line to map
        extractedLines[line] = curLine;

        if (curLine++ == 0) {
            continue;
        }
        InputFileLineInfoType lineInfo;
        if (!ExtractInfosFromLine(line, lineInfo)) {
            continue;
        }
        // add the line info to the output lines vector
        retValidLines.push_back(lineInfo);
    }

    // sort the read lines information
    std::sort (retValidLines.begin(), retValidLines.end(), InputFileLineInfoComparator());

    // compute the change from the previous value
    // TODO: needed only by the coherence
    for (size_t i = 0; i<retValidLines.size(); i++) {
        if (i >=1) {
            double diff = retValidLines[i].meanVal - retValidLines[i-1].meanVal;
            retValidLines[i].meanValChange = (diff < 0 ? 0 : diff);
        }
    }

    return retValidLines.size() > 1;
}
*/
