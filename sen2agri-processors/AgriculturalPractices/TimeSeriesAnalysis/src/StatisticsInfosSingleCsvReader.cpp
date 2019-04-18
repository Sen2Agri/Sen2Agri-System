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
    m_bIsCoheFile = false;
}

void StatisticsInfosSingleCsvReader::Initialize(const std::string &source, const std::vector<std::string> &filters, int year)
{
    (void)filters;

    std::ifstream ifs( source.c_str() );
    if( ifs.fail() ) {
        std::cout << "Error opening input file " << source << " exiting..." << std::endl;
        return;
    }

    std::string line;
    if (!std::getline(ifs, line)) {
        std::cout << "Input file " << source << " is empty. Exiting..." << std::endl;
        return;
    }
    std::vector<std::string> hdrItems;
    boost::algorithm::split(hdrItems, line, [](char c){return c == ';';});
    if (hdrItems.size() != m_InputFileHeader.size() &&
        hdrItems.size() != m_CoheInputFileHeader.size()) {
        //otbAppLogWARNING("Invalid line fields length for line " << fileLine);
        std::cout << "Invalid header length for file " << source << std::endl;
        return;
    }
    m_bIsCoheFile = (hdrItems.size() == m_CoheInputFileHeader.size());

    m_strSource = source;
    m_year = year;

    // check if index file exists near the source xml
    const std::string &idxFilePath(source + ".idx");
    if ( boost::filesystem::exists(idxFilePath)) {
        std::cout << "Loading indexes file " << idxFilePath << std::endl;
        // load the indexes
        std::ifstream idxFileStream(idxFilePath);
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
        std::cout << "Error opening input file " << m_strSource.c_str() <<". Exiting..." << std::endl;
        return false;
    }

    std::string fieldId = fid;
    NormalizeFieldId(fieldId);

    std::map<std::string, std::vector<InputFileLineInfoType>> mapInfos;
    const std::vector<std::string> &emptyVect = {""};
    const std::vector<std::string> &findFilters = filters.size() > 0 ? filters : emptyVect;
    if (m_IdxMap.size() == 0) {
        // read from the entire file
        if (!ExtractLinesFromStream(ifs, fid, findFilters, mapInfos)) {
            return false;
        }
    } else {
        IdxMapType::const_iterator it = m_IdxMap.find(fieldId);
        if (it != m_IdxMap.end()) {
            bool dataOk = false;
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
                        if (ExtractLinesFromStream(siStream, fid, findFilters, mapInfos)) {
                            dataOk = true;
                        }
                    }
                }
            }
            // none of the orbits was OK
            if (!dataOk) {
                return false;
            }
        }
    }
    // Now iterate the map infos and extract return the infos according to filters
    std::map<std::string, std::vector<InputFileLineInfoType>>::iterator mapIt;
    for (mapIt = mapInfos.begin(); mapIt != mapInfos.end(); ++mapIt) {
        std::vector<std::string>::const_iterator itF;
        for (itF = findFilters.begin(); itF != findFilters.end(); ++itF) {
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
            std::vector<InputFileLineInfoType> lineInfos;
            std::string uid;
            lineInfos.clear();
            if (ExtractInfosFromLine(line, findFilters, lineInfos, uid)) {
                std::map<std::string, std::vector<InputFileLineInfoType>>::iterator i = retMap.find(uid);
                if (i == retMap.end()) {
                    retMap[uid] = lineInfos;
                }
                else {
                    // TODO: Check for duplicates!!!
                    i->second.insert(i->second.end(), lineInfos.begin(), lineInfos.end());

                }
            }
        }
    }
    // sort the entries and check for the needed input size
    std::map<std::string, std::vector<InputFileLineInfoType>>::iterator itMap;
    bool hasValidValues = false;
    std::string shortOrbits;
    for (itMap = retMap.begin(); itMap != retMap.end(); ++itMap) {

        if (m_minReqEntries == 0 || (int)itMap->second.size() >= m_minReqEntries) {
            hasValidValues = true;
        } else {
            shortOrbits.append(", ").append(itMap->first).
                    append(" : ").append(std::to_string(itMap->second.size()));
        }

        // sort the read lines information
        std::sort (itMap->second.begin(), itMap->second.end(), InputFileLineInfoComparator());

        // compute the change from the previous value
        // TODO: needed only by the coherence
        for (size_t i = 0; i<itMap->second.size(); i++) {
            if (i >=1) {
                if ((itMap->second[i].ttDate - itMap->second[i-1].ttDate) <= 6 * SEC_IN_DAY) {
                    double diff = itMap->second[i].meanVal - itMap->second[i-1].meanVal;
                    itMap->second[i].meanValChange = (diff < 0 ? 0 : diff);
                } else {
                    //if we have more than 6 days difference between products, set the change to 0
                    itMap->second[i].meanValChange = 0;
                }
            }
        }
    }
    if (!hasValidValues) {
        std::cout << "Error extracting data. Input files are short" << shortOrbits << std::endl;
    }
    return hasValidValues;
}

std::vector<std::string> StatisticsInfosSingleCsvReader::GetInputFileLineElements(const std::string &line) {
    std::vector<std::string> results;
    boost::algorithm::split(results, line, [](char c){return c == ';';});
    return results;
}

bool StatisticsInfosSingleCsvReader::ExtractInfosFromLine(const std::string &fileLine, const std::vector<std::string> &findFilters,
                                                          std::vector<InputFileLineInfoType> &lineInfos, std::string &uid)
{
    std::vector<std::string> entries;
    // check if we have the compacted version of the CVS file
    boost::algorithm::split(entries, fileLine, [](char c){return c == '|';});

    int curEntryIdx = 0;
    std::string fid;
    std::string suffix;
    for (const auto &entry: entries) {
        int offset = 0;
        int hdrDiff;
        const std::vector<std::string> &lineElems = GetInputFileLineElements(entry);
        if (curEntryIdx == 0) {
            if(findFilters.size() > 0) {
                bool doFilter = false;
                for (const std::string & filter: findFilters) {
                    if (lineElems[1].find(filter) != std::string::npos) {
                        doFilter = true;
                        break;
                    }
                }
                if (!doFilter) {
                    return false;
                }
            }
            fid = lineElems[0];
            suffix = lineElems[1];
            offset = 2;
            hdrDiff = 0;
            // update the returned uid
            uid = fid + suffix;
            curEntryIdx++;
        } else {
            hdrDiff = 2;
        }

        if (m_bIsCoheFile) {
            if (lineElems.size() != (size_t)(m_CoheInputFileHeader.size() - hdrDiff)) {
                std::cout << "Invalid file field length for " << fid + "_" + suffix << std::endl;
                return false;
            }
        } else {
            if (lineElems.size() != (size_t)(m_InputFileHeader.size() - hdrDiff)) {
                std::cout << "Invalid file field length for " << fid + "_" + suffix << std::endl;
                return false;
            }
        }

        InputFileLineInfoType lineInfo;
        lineInfo.Reset();
        int curIdx = offset;
        const std::string &strDate = lineElems[curIdx++];

        const std::string &strDate2 = m_bIsCoheFile ? lineElems[curIdx++] : "";
        std::string::size_type sz;     // alias of size_t
        lineInfo.meanVal = std::stod(lineElems[curIdx++], &sz);
        lineInfo.stdDev = std::stod(lineElems[curIdx++], &sz);

        int weekNo, yearNo;
        int itemYear = 0;
        // ignore products that are not from the current year
        if (GetWeekFromDate(strDate, yearNo, weekNo, INPUT_FILE_DATE_PATTERN))
        {
            lineInfo.fieldId = fid; // Normally, here the name should be normalized
            if (m_bIsCoheFile && m_bSwitchDates && strDate2 != "") {
                lineInfo.strDate2 = strDate;
                lineInfo.ttDate2 = GetTimeFromString(strDate);
                lineInfo.weekNo2 = weekNo;
                lineInfo.ttDate2Floor = FloorDateToWeekStart(lineInfo.ttDate2);
            } else {
                lineInfo.strDate = strDate;
                lineInfo.ttDate = GetTimeFromString(strDate);
                lineInfo.weekNo = weekNo;
                lineInfo.ttDateFloor = FloorDateToWeekStart(lineInfo.ttDate);
                itemYear = yearNo;
            }
        } else {
            std::cout << "Ignoring date: Invalid date format " << strDate <<std::endl;
            return false;
        }

        if (m_bIsCoheFile)
        {
            int weekNo2, yearNo2;
            if (GetWeekFromDate(strDate2, yearNo2, weekNo2, INPUT_FILE_DATE_PATTERN)) {
                if (m_bIsCoheFile && m_bSwitchDates && strDate2 != "") {
                    lineInfo.strDate = strDate2;
                    lineInfo.ttDate = GetTimeFromString(strDate2);
                    lineInfo.weekNo = weekNo2;
                    lineInfo.ttDateFloor = FloorDateToWeekStart(lineInfo.ttDate);
                    itemYear = yearNo2;
                } else {
                    lineInfo.strDate2 = strDate2;
                    lineInfo.ttDate2 = GetTimeFromString(strDate2);
                    lineInfo.weekNo2 = weekNo2;
                    lineInfo.ttDate2Floor = FloorDateToWeekStart(lineInfo.ttDate2);
                }
            } else {
                //otbAppLogWARNING("Invalid date 2 format found for line " << fileLine);
                std::cout << "Ignoring date: Invalid date 2 format found " << strDate << std::endl;
                return false;
            }
        }
        if (m_year == itemYear) {
            lineInfos.push_back (lineInfo);
        } //else {
            // silently ignore the error
            //std::cout << "Ignoring date: Invalid year found in date " << strDate << " for UID " << uid << std::endl;
//        }
    }

    return true;
}
