#include "PracticeCsvReader.h"

#include <boost/algorithm/string.hpp>

bool PracticeCsvReader::ExtractFeatures(std::function<bool (const FeatureDescription&)> fnc)
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

std::vector<std::string> PracticeCsvReader::CsvFeatureDescription::LineToVector(const std::string &line)
{
    std::vector<std::string> results;
    boost::algorithm::split(results, line, [](char c){return c == ';';});
    return results;
}


bool PracticeCsvReader::CsvFeatureDescription::ExtractHeaderInfos(const std::string &line)
{
    m_InputFileHeader = LineToVector(line);
    if (m_InputFileHeader.size() != HEADER_SIZE && m_InputFileHeader.size() != HEADER_SIZE_WITH_SEQ_ID) {
        std::cout << "Header " << line << " does not correspond with the expected one for file " << m_source << std::endl;
        return false;
    }
    m_FieldIdFieldIdx = GetPosInHeader("FIELD_ID");
    m_SeqFieldIdFieldIdx = GetPosInHeader("SEQ_ID");
    m_CountryFieldIdx = GetPosInHeader("COUNTRY");
    m_YearFieldIdx = GetPosInHeader("YEAR");
    m_MainCropFieldIdx = GetPosInHeader("MAIN_CROP");
    m_VegStartFieldIdx = GetPosInHeader("VEG_START");
    m_HarvestStartFieldIdx = GetPosInHeader("H_START");
    m_HarvestEndFieldIdx = GetPosInHeader("H_END");
    m_PracticeFieldIdx = GetPosInHeader("PRACTICE");
    m_PracticeTypeFieldIdx = GetPosInHeader("P_TYPE");
    m_PracticeStartFieldIdx = GetPosInHeader("P_START");
    m_PracticeEndFieldIdx = GetPosInHeader("P_END");
    m_S1PixIdx = GetPosInHeader("S1Pix");
    if (m_FieldIdFieldIdx == -1 || m_CountryFieldIdx == -1 ||
            m_YearFieldIdx == -1 || m_MainCropFieldIdx == -1 ||
            m_VegStartFieldIdx == -1 || m_HarvestStartFieldIdx == -1 ||
            m_HarvestEndFieldIdx == -1 || m_PracticeFieldIdx == -1 ||
            m_PracticeTypeFieldIdx == -1 || m_PracticeStartFieldIdx == -1 ||
            m_PracticeEndFieldIdx == -1 ) {
        std::cout << "One of the expected entries was not found in the header of file " << m_source << std::endl;
        return false;
    }
    return true;
}


int PracticeCsvReader::CsvFeatureDescription::GetPosInHeader(const std::string &item)
{
    auto result = std::find(m_InputFileHeader.begin(), m_InputFileHeader.end(), item);
    if (result != m_InputFileHeader.end()) {
        return std::distance(m_InputFileHeader.begin(), std::find(m_InputFileHeader.begin(), m_InputFileHeader.end(), item));
    }
    return -1;
}


bool PracticeCsvReader::CsvFeatureDescription::ExtractLineInfos(const std::string &line)
{
    const std::vector<std::string> &lineEntries = LineToVector(line);
    if (lineEntries.size() > 0 && lineEntries.size() == m_InputFileHeader.size()) {
        m_FieldIdVal       = lineEntries[m_FieldIdFieldIdx];
        m_CountryVal       = lineEntries[m_CountryFieldIdx];
        m_YearVal          = lineEntries[m_YearFieldIdx];
        m_MainCropVal      = lineEntries[m_MainCropFieldIdx];
        m_VegStartVal      = lineEntries[m_VegStartFieldIdx];
        m_HarvestStartVal  = lineEntries[m_HarvestStartFieldIdx];
        m_HarvestEndVal    = lineEntries[m_HarvestEndFieldIdx];
        m_PracticeVal      = lineEntries[m_PracticeFieldIdx];
        m_PracticeTypeVal  = lineEntries[m_PracticeTypeFieldIdx];
        m_PracticeStartVal = lineEntries[m_PracticeStartFieldIdx];
        m_PracticeEndVal   = lineEntries[m_PracticeEndFieldIdx];
        if (m_SeqFieldIdFieldIdx != -1) {
            m_SeqFieldIdVal = lineEntries[m_SeqFieldIdFieldIdx];
        }
        if (m_S1PixIdx != -1) {
            m_S1Pix = lineEntries[m_S1PixIdx];
        }

        m_bIsValid = true;
        return true;
    }
    return false;
}


std::string PracticeCsvReader::CsvFeatureDescription::GetFieldId() const
{
    return m_FieldIdVal;
}

std::string PracticeCsvReader::CsvFeatureDescription::GetFieldSeqId() const
{
    return m_SeqFieldIdVal;
}

std::string PracticeCsvReader::CsvFeatureDescription::GetCountryCode() const
{
    return m_CountryVal;
}

std::string PracticeCsvReader::CsvFeatureDescription::GetYear() const
{
    return m_YearVal;
}

std::string PracticeCsvReader::CsvFeatureDescription::GetMainCrop() const
{
    return m_MainCropVal;
}

std::string PracticeCsvReader::CsvFeatureDescription::GetVegetationStart() const
{
    return m_VegStartVal;
}

std::string PracticeCsvReader::CsvFeatureDescription::GetHarvestStart() const
{
    return m_HarvestStartVal;
}

std::string PracticeCsvReader::CsvFeatureDescription::GetHarvestEnd() const
{
    return m_HarvestEndVal;
}

std::string PracticeCsvReader::CsvFeatureDescription::GetPractice() const
{
    return m_PracticeVal;
}

std::string PracticeCsvReader::CsvFeatureDescription::GetPracticeType() const
{
    return m_PracticeTypeVal;
}

std::string PracticeCsvReader::CsvFeatureDescription::GetPracticeStart() const
{
    return m_PracticeStartVal;
}

std::string PracticeCsvReader::CsvFeatureDescription::GetPracticeEnd() const
{
    return m_PracticeEndVal;
}

std::string PracticeCsvReader::CsvFeatureDescription::GetS1Pix() const
{
    return m_S1Pix;
}
