#include "StatisticsInfosSingleCsvReader.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>

#include <cinttypes>

#include "TimeSeriesAnalysisUtils.h"

StatisticsInfosSingleCsvReader::StatisticsInfosSingleCsvReader()
{
    m_InputFileHeader = {"KOD_PB", "suffix", "date", "mean", "stdev"};
    m_CoheInputFileHeader = {"KOD_PB", "suffix", "date1", "date2", "mean", "stdev"};
}

void StatisticsInfosSingleCsvReader::Initialize(const std::string &source, const std::vector<std::string> &filters)
{
    (void)filters;
    m_strSource = source;
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
                idxInfos.suffix = tokens[1];
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

bool StatisticsInfosSingleCsvReader::GetEntriesForField(const std::string &fid, const std::vector<std::string> &filters,
                                                          std::map<std::string, std::vector<InputFileLineInfoType>> &retMap)
{
    std::ifstream ifs( m_strSource.c_str() );
    if( ifs.fail() ) {
        std::cout << "Error opening input file, exiting..." << std::endl;
        return false;
    }

    std::string fieldId = fid;
    NormalizeFieldId(fieldId);

    std::map<std::string, std::vector<InputFileLineInfoType>> mapInfos;
    const std::vector<std::string> &emptyVect = {""};
    const std::vector<std::string> &findFilters = filters.size() > 0 ? filters : emptyVect;
    if (m_IdxMap.size() == 0) {
        // read from the entire file
        if (!ExtractLinesFromStream(ifs, fieldId, findFilters, mapInfos)) {
            return false;
        }
    } else {
        IdxMapType::const_iterator it = m_IdxMap.find(fieldId);
        if (it != m_IdxMap.end()) {
            // search the indexed infos filtered
            for (const FieldIndexInfos &info: it->second) {
                for (const std::string & filter: findFilters) {
                    if (info.suffix.find(filter) != std::string::npos) {
                        ifs.seekg(info.startIdx, std::ios::beg);
                        char buff[info.len+1];
                        ifs.read(buff, info.len);
                        buff[info.len] = 0;
                        std::istringstream siStream(buff);
                        // read only from this section of the file
                        if (!ExtractLinesFromStream(siStream, fieldId, findFilters, mapInfos)) {
                            return false;
                        }
                    }
                }
            }
        }
    }
    // Now iterate the map infos and extract return the infos according to filters
    std::map<std::string, std::vector<InputFileLineInfoType>>::iterator mapIt;
    for (mapIt = mapInfos.begin(); mapIt != mapInfos.end(); ++mapIt) {
        std::vector<std::string>::const_iterator itF;
        for (itF = filters.begin(); itF != filters.end(); ++itF) {
            if(itF->size() == 0 || mapIt->first.find(*itF) != std::string::npos) {
                // get the existing map for the filter
                std::map<std::string, std::vector<InputFileLineInfoType>>::iterator i = retMap.find(*itF);
                if (i == retMap.end()) {
                    retMap[*itF] = mapIt->second;
                } else {
                    i->second.insert(i->second.end(), mapIt->second.begin(), mapIt->second.end());
                }
            }
        }
    }

    return true;
}

bool StatisticsInfosSingleCsvReader::ExtractLinesFromStream(std::istream &inStream, const std::string &fieldId,
                                                            const std::vector<std::string> &findFilters,
                                                            std::map<std::string, std::vector<InputFileLineInfoType>> &retMap) {
    std::string line;
    // ensure that we don't have a partial matching and getting other ids (ex. xxx_27 to be got for xxx_2)
    std::string fieldToFind = fieldId + ";";

    while (std::getline(inStream, line)) {
        // check if line starts with the field id
        if (line.compare(0, fieldToFind.length(), fieldToFind) == 0) {
            InputFileLineInfoType lineInfo;
            std::string uid;
            lineInfo.Reset();
            if (ExtractInfosFromLine(line, findFilters, lineInfo, uid)) {
                std::map<std::string, std::vector<InputFileLineInfoType>>::iterator i = retMap.find(uid);
                if (i == retMap.end()) {
                    std::vector<InputFileLineInfoType> mapCurInfos;
                    mapCurInfos.push_back(lineInfo);
                    retMap[uid] = mapCurInfos;
                }
                else {
                    // TODO: Check for duplicates!!!
                    i->second.push_back(lineInfo);
                }
            }
        }
    }
    // sort the entries and check for the needed input size
    std::map<std::string, std::vector<InputFileLineInfoType>>::iterator itMap;
    for (itMap = retMap.begin(); itMap != retMap.end(); ++itMap) {

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

    return true;
}

std::vector<std::string> StatisticsInfosSingleCsvReader::GetInputFileLineElements(const std::string &line) {
    std::vector<std::string> results;
    boost::algorithm::split(results, line, [](char c){return c == ';';});
    return results;
}

bool StatisticsInfosSingleCsvReader::ExtractInfosFromLine(const std::string &fileLine, const std::vector<std::string> &findFilters,
                                                          InputFileLineInfoType &lineInfo, std::string &uid)
{
    const std::vector<std::string> &lineElems = GetInputFileLineElements(fileLine);
    if (lineElems.size() != m_InputFileHeader.size() &&
        lineElems.size() != m_CoheInputFileHeader.size()) {
        //otbAppLogWARNING("Invalid line fields length for line " << fileLine);
        std::cout << "Invalid line fields length for line " << fileLine << std::endl;
        return false;
    }

    // check also if the suffix pass the filter
    const std::string &fid = lineElems[0];
    const std::string &suffix = lineElems[1];
    const std::string &strDate = lineElems[2];
    if(findFilters.size() > 0) {
        bool doFilter = false;
        for (const std::string & filter: findFilters) {
            if (suffix.find(filter) != std::string::npos) {
                doFilter = true;
            }
        }
        if (!doFilter) {
            return false;
        }
    }

    // update the returned uid
    uid = fid + suffix;

    bool isCohe = (lineElems.size() == m_CoheInputFileHeader.size());
    int meanIdx = isCohe ? 4 : 3;
    int stdDevIdx = isCohe ? 5 : 4;
    std::string::size_type sz;     // alias of size_t
    int weekNo, yearNo;
    if (GetWeekFromDate(strDate, yearNo, weekNo, INPUT_FILE_DATE_PATTERN))
    {
        lineInfo.fieldId = fid; // Normally, here the name should be normalized
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
        const std::string &strDate2 = lineElems[3];
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
