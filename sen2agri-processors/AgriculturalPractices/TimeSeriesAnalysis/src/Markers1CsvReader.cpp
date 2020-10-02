#include "Markers1CsvReader.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>

#include <cinttypes>

#include "TimeSeriesAnalysisUtils.h"



Markers1CsvReader::Markers1CsvReader()
{
    m_inDateFormat = "%Y%m%d";
}

void Markers1CsvReader::Initialize(const std::string &source, const std::vector<std::string> &filters, int year)
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
    if (!ExtractHeaderInfos(line, year)) {
        std::cout << "Error extracting header information from file " << source << std::endl;
        return;
    }
    m_strSource = source;
    m_year = year;

    LoadIndexFile(source);
}

bool Markers1CsvReader::GetEntriesForField(const std::string &fid, const std::vector<std::string> &filters,
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

bool Markers1CsvReader::ExtractHeaderInfos(const std::string &hdrLine, int year) {
    std::vector<std::string> headerItems;
    boost::algorithm::split(headerItems, hdrLine, [](char c){return c == ';';});
    if (headerItems.size() <= 1) {
        //otbAppLogWARNING("Invalid line fields length for line " << fileLine);
        std::cout << "Invalid header length for header " << hdrLine << std::endl;
        return false;
    }
    int i = 0;
    for (const std::string &headerItem: headerItems) {
        ColumnInfo colInfo;
        colInfo.bIgnoreColumn = false;
        colInfo.fullColName = headerItem;

        if (i == 0) {
            i++;
        } else {
            std::vector<std::string> columnItems;
            boost::algorithm::split(columnItems, headerItem, [](char c){return c == '_';});
            if (columnItems.size() < 3) {
                std::cout << "Invalid column name " << headerItem << " found!" << std::endl;
                return false;
            }
            // first is the date
            colInfo.ttDate = GetTimeFromString(columnItems[0], m_inDateFormat);
            colInfo.strDateSeparators = TimeToString(colInfo.ttDate);

            int weekNo, yearNo;
            // ignore products that are not from the current year
            if (GetWeekFromDate(colInfo.strDateSeparators, yearNo, weekNo))
            {
                colInfo.weekNo = weekNo;
                colInfo.ttDateFloor = FloorDateToWeekStart(colInfo.ttDate);
                colInfo.yearNo = yearNo;
            } else {
                std::cout << "Ignoring date: Invalid date format in column " << headerItem <<std::endl;
                return false;
            }
            colInfo.paramName = columnItems[1];
            colInfo.prdType = columnItems[2];
            if (columnItems.size() >= 4) {
                colInfo.polarisation = columnItems[3];
            }
            if (columnItems.size() >= 5) {
                colInfo.orbit = columnItems[4];
            }

            // TODO: Check if this limitation cannot be actually removed
            if (year != colInfo.yearNo) {
                colInfo.bIgnoreColumn = true;
            }
        }
        m_header.push_back(colInfo);
    }
    return true;
}

bool Markers1CsvReader::LoadIndexFile(const std::string &source) {
    // check if index file exists near the source xml
    const std::string &idxFilePath(source + ".idx");
    if ( boost::filesystem::exists(idxFilePath)) {
        std::cout << "Loading indexes file " << idxFilePath << std::endl;
        // load the indexes
        std::string line;
        std::ifstream idxFileStream(idxFilePath);
        while (std::getline(idxFileStream, line)) {
            const std::vector<std::string> &tokens = split (line, ';');
            if (tokens.size() == 3) {
                std::string key = tokens[0];
                NormalizeFieldId(key);
                FieldIndexInfos idxInfos;
                idxInfos.startIdx = std::strtoumax(tokens[1].c_str(), nullptr, 10);
                idxInfos.len = std::atoi(tokens[2].c_str());

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

    return true;
}

bool Markers1CsvReader::ExtractLinesFromStream(std::istream &inStream, const std::string &fieldId,
                                                            const std::vector<std::string> &findFilters,
                                                            std::map<std::string, std::vector<InputFileLineInfoType>> &retMap) {
    std::string line;
    // ensure that we don't have a partial matching and getting other ids (ex. xxx_27 to be got for xxx_2)
    std::string fieldToFind = fieldId + ";";

    std::map<std::string, std::vector<InputFileLineInfoType>>::iterator i;
    std::map<std::string, std::vector<InputFileLineInfoType>>::iterator itInfos;
    std::map<std::string, std::vector<InputFileLineInfoType>> lineInfos;
    while (std::getline(inStream, line)) {
        // check if line starts with the field id
        if (line.compare(0, fieldToFind.length(), fieldToFind) == 0) {
            lineInfos.clear();
            if (ExtractInfosFromLine(line, findFilters, lineInfos)) {
                for (itInfos = lineInfos.begin() ; itInfos != lineInfos.end() ; ++itInfos) {
                    i = retMap.find(itInfos->first);
                    if (i == retMap.end()) {
                        retMap[itInfos->first] = itInfos->second;
                    }
                    else {
                        // TODO: Check for duplicates!!!
                        i->second.insert(i->second.end(), itInfos->second.begin(), itInfos->second.end());
                    }
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
    if (!hasValidValues && m_bDebug) {
        std::cout << "Error extracting data. Input files are short" << shortOrbits << std::endl;
    }
    return hasValidValues;
}

std::vector<std::string> Markers1CsvReader::GetInputFileLineElements(const std::string &line) {
    std::vector<std::string> results;
    boost::algorithm::split(results, line, [](char c){return c == ';';});
    return results;
}

bool Markers1CsvReader::ExtractInfosFromLine(const std::string &fileLine, const std::vector<std::string> &findFilters,
                                             std::map<std::string, std::vector<InputFileLineInfoType>> &lineInfos)
{
    const std::vector<std::string> &lineElems = GetInputFileLineElements(fileLine);
    if (lineElems.size() != m_header.size()) {
        return false;
    }
    std::string::size_type sz;     // alias of size_t
    const std::string &id = lineElems[0];

    std::string uid;
    // iterate all columns and extract the line infos. Note that a line might contain several polarisations
    typename std::map<std::string, std::vector<InputFileLineInfoType>>::iterator containerIt;
    for (size_t i = 1; i<m_header.size(); i++) {
        if (lineElems[i].size() == 0) {
            i++; // we skip also the next column which should be stddev
            continue;   // ignore no data columns
        }
        const ColumnInfo &colInfo = m_header[i];
        // if we have filters on polarisation, check if the current column value is accepted
        if(findFilters.size() > 0) {
            bool doFilter = false;
            for (const std::string & filter: findFilters) {
                if (filter == colInfo.polarisation) {
                    doFilter = true;
                    break;
                }
            }
            if (!doFilter) {
                continue;
            }
        }
        // we add now the column value to the corresponding line
        InputFileLineInfoType lineInfo;
        lineInfo.Reset();
        lineInfo.fieldId = id;
        // we assume that the mean and stdev are consecutives for the same date and prdtype, polarisation and orbit
        lineInfo.meanVal = std::stod(lineElems[i++], &sz);
        lineInfo.stdDev = std::stod(lineElems[i], &sz);

        // fill the other fields from the header infos
        lineInfo.strDate = colInfo.strDateSeparators;
        lineInfo.ttDate = colInfo.ttDate;
        lineInfo.weekNo = colInfo.weekNo;
        lineInfo.ttDateFloor = colInfo.ttDateFloor;

        uid = id + colInfo.polarisation;
        if (colInfo.prdType == "COHE") {
            uid += colInfo.orbit;   // If it is coherence, make the unique ID orbit dependent
        }
        // Add the new line info in the output map
        containerIt = lineInfos.find(uid);
        if(containerIt != lineInfos.end()) {
            containerIt->second.push_back(lineInfo);
        } else {
            // add it into the vector
            std::vector<InputFileLineInfoType> lineInfosVec;
            lineInfosVec.push_back(lineInfo);
            lineInfos[uid] = lineInfosVec;
        }
    }

    return true;
}
