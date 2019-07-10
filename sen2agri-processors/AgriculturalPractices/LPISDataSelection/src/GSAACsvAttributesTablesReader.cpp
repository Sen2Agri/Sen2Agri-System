#include "GSAACsvAttributesTablesReader.h"

#include <boost/algorithm/string.hpp>
#include "CommonFunctions.h"

bool GSAACsvAttributesTablesReader::ExtractAttributes(std::function<void (const AttributeEntry&)> fnc)
{
    std::ifstream fStream(m_source);
    if (!fStream.is_open()) {
        std::cout << "Error opennig file " << m_source << std::endl;
        return false;
    }
    std::string line;
    if (std::getline(fStream, line)) {
        CsvFeatureDescription feature;
        if (!feature.ExtractHeaderInfos(line)) {
            std::cout << "Error header infos from file " << m_source << std::endl;
            return false;
        }
        while (std::getline(fStream, line)) {
            if(feature.ExtractLineInfos(line)) {
                fnc(feature);
            }
        }
    }
    return true;
}

std::vector<std::string> GSAACsvAttributesTablesReader::CsvFeatureDescription::LineToVector(const std::string &line)
{
    std::vector<std::string> results;
    boost::algorithm::split(results, line, [](char c){return (c == ',' || c == ';');});
    for(size_t i = 0; i < results.size(); i++) {
        results[i] = trim(results[i]);
    }
    return results;
}


bool GSAACsvAttributesTablesReader::CsvFeatureDescription::ExtractHeaderInfos(const std::string &line)
{
    std::vector<std::string> inputFileHeader = LineToVector(line);
    if(inputFileHeader.size() == 0)
    {
        std::cout << "Header with size 0 found for file " << m_source << std::endl;
        return false;
    }
    // we are filling the header values as lowercase
    for (size_t i = 0; i<inputFileHeader.size(); i++) {
        std::string str = inputFileHeader[i];
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        m_InputFileHeader[str] = i;
    }
    return true;
}

bool GSAACsvAttributesTablesReader::CsvFeatureDescription::ExtractLineInfos(const std::string &line)
{
    m_lineEntries = LineToVector(line);
    if (m_lineEntries.size() > 0 && m_lineEntries.size() == m_InputFileHeader.size()) {
        m_bIsValid = true;
        return true;
    }
    return false;
}


int GSAACsvAttributesTablesReader::CsvFeatureDescription::GetFieldIndex(const char *pszName) const
{
    std::string strName(pszName);
    std::transform(strName.begin(), strName.end(), strName.begin(), ::tolower);
    std::map<std::string, int>::const_iterator itMap = m_InputFileHeader.find(strName);
    if (itMap == m_InputFileHeader.end()) {
        return -1;
    }
    return itMap->second;
}

const char* GSAACsvAttributesTablesReader::CsvFeatureDescription::GetFieldAsString(int idx) const
{
    if (idx >=0 && (size_t)idx < m_lineEntries.size())
        return m_lineEntries[idx].c_str();
    return "";
}

double GSAACsvAttributesTablesReader::CsvFeatureDescription::GetFieldAsDouble(int idx) const
{
    return std::atof(GetFieldAsString(idx));
}

int GSAACsvAttributesTablesReader::CsvFeatureDescription::GetFieldAsInteger(int idx) const
{
    return std::atoi(GetFieldAsString(idx));
}
