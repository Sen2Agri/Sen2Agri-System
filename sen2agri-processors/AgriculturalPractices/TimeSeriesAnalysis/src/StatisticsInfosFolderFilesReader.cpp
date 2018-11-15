#include "StatisticsInfosFolderFilesReader.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>

#include "TimeSeriesAnalysisUtils.h"

StatisticsInfosFolderFilesReader::StatisticsInfosFolderFilesReader()
{
    m_InputFileHeader = {"KOD_PB", "date", "mean", "stdev"};
    m_CoheInputFileHeader = {"KOD_PB", "date1", "date2", "mean", "stdev"};
}

void StatisticsInfosFolderFilesReader::Initialize(const std::string &source, const std::vector<std::string> &filters, int year)
{
    (void)filters; //suppress not used warning
    m_year = year;
    this->m_InfoFiles = GetFilesInFolder(source);
}

bool StatisticsInfosFolderFilesReader::GetEntriesForField(const std::string &fieldId, const std::vector<std::string> &filters,
                                                          std::map<std::string, std::vector<InputFileLineInfoType>> &retMap)
{
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
}

bool StatisticsInfosFolderFilesReader::ExtractFileInfosForFilter(const FileInfoType &fileInfo, const std::string &filter,
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

std::vector<FileInfoType> StatisticsInfosFolderFilesReader::GetFilesInFolder(const std::string &targetPath) {
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

std::vector<FileInfoType> StatisticsInfosFolderFilesReader::FindFilesForFieldId(const std::string &fieldId)
{
    std::vector<FileInfoType> allMatchingFiles;
    const std::string &cmpFieldId = fieldId + "_";
    typename std::vector<FileInfoType>::const_iterator filesIterator;
    for (filesIterator = m_InfoFiles.begin(); filesIterator != m_InfoFiles.end(); ++filesIterator) {
        if (filesIterator->fileName.compare(0, cmpFieldId.length(), cmpFieldId) == 0) {
            allMatchingFiles.push_back(*filesIterator);
        }
    }
    return allMatchingFiles;
//            boost::smatch what;
//            // Skip if no match
//            const boost::filesystem::path &path = i->path().filename();
//            const std::string &pathStr = path.string();
//            if (!boost::regex_match( pathStr, what, fieldIdFileFilter)) {
//                continue;
//            }
//            allMatchingFiles.push_back(pathStr);
//        return allMatchingFiles;
}

std::vector<std::string> StatisticsInfosFolderFilesReader::GetInputFileLineElements(const std::string &line) {
    std::vector<std::string> results;
    boost::algorithm::split(results, line, [](char c){return c == ';';});
    return results;
}

bool StatisticsInfosFolderFilesReader::ExtractInfosFromLine(const std::string &fileLine, InputFileLineInfoType &lineInfo)
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
        lineInfo.fieldId = lineElems[0]; // Normally, here the name should be normalized
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

bool StatisticsInfosFolderFilesReader::ExtractLineInfos(const std::string &filePath, std::vector<InputFileLineInfoType> &retValidLines)
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
        lineInfo.Reset();
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

