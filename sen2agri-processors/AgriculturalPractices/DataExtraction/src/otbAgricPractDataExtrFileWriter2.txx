/*
 * Copyright (C) 2005-2017 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef otbAgricPractDataExtrFileWriter2_txx
#define otbAgricPractDataExtrFileWriter2_txx

#include "otbAgricPractDataExtrFileWriter2.h"
#include "itkMacro.h"
#include "itksys/SystemTools.hxx"
#include "otb_tinyxml.h"
#include "otbStringUtils.h"
#include "CommonFunctions.h"

namespace otb {

template < class TMeasurementVector >
AgricPractDataExtrFileWriter2<TMeasurementVector>
::AgricPractDataExtrFileWriter2(): m_TargetFileName(""),
    m_bOutputCsvFormat(true), m_bCsvCompactMode(true), m_bMultiFileMode(false), m_bUseLatestNamingFormat(true),
    m_bUseDate2(false), m_bUseMinMax(false)
{
    m_MeanPosInHeader = -1;
    m_StdevPosInHeader = -1;
    m_MinPosInHeader = -1;
    m_MaxPosInHeader = -1;
    m_ValidPixelsPosInHeader = -1;
    m_InvalidPixelsPosInHeader = -1;
}

/*
template < class TMeasurementVector >
template <typename MapType>
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::AddInputMaps(const std::string &fileName, const MapType& mapMeans,  const MapType& mapStdDev)
{
    // TODO: This is a copy/paste with small changes of the function below
    //      It should be unified with the other one
    std::string fileType;
    std::string polarisation;
    std::string orbit;
    time_t fileDate;
    time_t additionalFileDate = 0;
    if (!GetFileInfosFromName(fileName, fileType, polarisation, orbit, fileDate, additionalFileDate, m_bUseLatestNamingFormat))
    {
        std::cout << "Error extracting file informations from file name " << fileName << std::endl;
        return;
    }
    if (additionalFileDate) {
        m_bUseDate2 = true;
    }

    typename MapType::const_iterator it;
    std::string fieldId;
    for ( it = mapMeans.begin() ; it != mapMeans.end() ; ++it)
    {
      fieldId = boost::lexical_cast<std::string>(it->first);
      const auto &meanVal = it->second[0];
      const auto &stdDevVal = mapStdDev.at(it->first)[0];

      const std::string &fieldOutFileName = BuildOutputFileName(fieldId, fileType, polarisation, orbit, fileDate, additionalFileDate);
      const std::string &uniqueId = fieldId + fieldOutFileName;

      typename std::map<std::string, FileFieldsInfoType>::iterator containerIt =
              m_FileFieldsContainer.find(uniqueId);
      if(containerIt != m_FileFieldsContainer.end()) {
          FieldEntriesType fieldEntry;
          fieldEntry.date = fileDate;
          fieldEntry.additionalFileDate = additionalFileDate;
          // we exclude from the header the fid and the date
          fieldEntry.values.resize(m_HeaderFields.size() - 2);
          fieldEntry.values[m_MeanPosInHeader] = meanVal;
          fieldEntry.values[m_StdevPosInHeader] = stdDevVal;
          containerIt->second.fieldsEntries.push_back(fieldEntry);

      } else {
          // add it into the container
          FileFieldsInfoType fileFieldsInfoType;
          fileFieldsInfoType.fid = fieldId;
          fileFieldsInfoType.fileName = fieldOutFileName;

          FieldEntriesType fieldEntry;
          fieldEntry.date = fileDate;
          fieldEntry.additionalFileDate = additionalFileDate;
          // we exclude from the header the fid and the date
          fieldEntry.values.resize(m_HeaderFields.size() - 2);
          fieldEntry.values[m_MeanPosInHeader] = meanVal;
          fieldEntry.values[m_StdevPosInHeader] = stdDevVal;
          fileFieldsInfoType.fieldsEntries.push_back(fieldEntry);

          m_FileFieldsContainer[uniqueId] = fileFieldsInfoType;
      }
    }
}
*/
/*
template < class TMeasurementVector >
template <typename MapType>
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::AddInputMap(const std::string &fileName, const MapType& map )
{
    MapType mapMins;
    MapType mapMax;
    AddInputMap(fileName, map, mapMins, mapMax);
}
*/
template < class TMeasurementVector >
template <typename MapType, typename MapMinMaxType>
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::AddInputMap(const std::string &fileName, const MapType& map, const MapMinMaxType& mapMins, const MapMinMaxType& mapMax,
              const MapMinMaxType& mapValidPixels, const MapMinMaxType& mapInvalidPixels)
{
    std::string fileType;
    std::string polarisation;
    std::string orbit;
    time_t fileDate;
    time_t additionalFileDate = 0;
    if (!GetFileInfosFromName(fileName, fileType, polarisation, orbit, fileDate, additionalFileDate, m_bUseLatestNamingFormat))
    {
        std::cout << "Error extracting file informations from file name " << fileName << std::endl;
        return;
    }
    if (additionalFileDate) {
        m_bUseDate2 = true;
    }

    // We ensure that we have the same number of values for these maps as in the input map
    bool useMinMax = false;
    if (m_bUseMinMax) {
        if (map.size() == mapMins.size() && map.size() == mapMax.size() &&
                map.size() == mapValidPixels.size() &&
                map.size() == mapInvalidPixels.size()) {
            useMinMax = true;
        }
    }

    typename MapType::const_iterator it;
    typename MapMinMaxType::const_iterator itMin = mapMins.begin();
    typename MapMinMaxType::const_iterator itMax = mapMax.begin();
    typename MapMinMaxType::const_iterator itValidPixelsCnt = mapValidPixels.begin();
    typename MapMinMaxType::const_iterator itInvalidPixelsCnt = mapInvalidPixels.begin();

    std::string fieldId;
    for ( it = map.begin() ; it != map.end() ; ++it)
    {
      fieldId = boost::lexical_cast<std::string>(it->first);
//      if (fieldId != "584109910/2") {
//          continue;
//      }
      const auto &meanVal = it->second.mean[0];
      const auto &stdDevVal = it->second.stdDev[0];

      const std::string &fieldOutFileName = m_bUseMinMax ?
                  boost::filesystem::path(fileName).filename().c_str() :
                  BuildOutputFileName(fieldId, fileType, polarisation, orbit, fileDate, additionalFileDate);
      const std::string &uniqueId = fieldId + fieldOutFileName;

      typename std::map<std::string, FileFieldsInfoType>::iterator containerIt =
              m_FileFieldsContainer.find(uniqueId);

      FieldEntriesType fieldEntry;
      fieldEntry.date = fileDate;
      fieldEntry.additionalFileDate = additionalFileDate;
      // we exclude from the header the fid and the date
      fieldEntry.values.resize(m_HeaderFields.size() - 2);
      fieldEntry.values[m_MeanPosInHeader] = meanVal;
      fieldEntry.values[m_StdevPosInHeader] = stdDevVal;
      if (useMinMax && m_MinPosInHeader > 0 && m_MaxPosInHeader > 0 &&
              m_ValidPixelsPosInHeader > 0 && m_InvalidPixelsPosInHeader > 0) {
          fieldEntry.values[m_MinPosInHeader] = itMin->second[0];
          fieldEntry.values[m_MaxPosInHeader] = itMax->second[0];
          fieldEntry.values[m_ValidPixelsPosInHeader] = itValidPixelsCnt->second[0];
          fieldEntry.values[m_InvalidPixelsPosInHeader] = itInvalidPixelsCnt->second[0];
      }

      if(containerIt != m_FileFieldsContainer.end()) {
          containerIt->second.fieldsEntries.push_back(fieldEntry);
      } else {
          // add it into the container
          FileFieldsInfoType fileFieldsInfoType;
          fileFieldsInfoType.fid = fieldId;
          fileFieldsInfoType.fileName = fieldOutFileName;
          fileFieldsInfoType.fieldsEntries.push_back(fieldEntry);

          m_FileFieldsContainer[uniqueId] = fileFieldsInfoType;
      }
      if (useMinMax) {
          ++itMin;
          ++itMax;
          ++itValidPixelsCnt;
          ++itInvalidPixelsCnt;
      }
    }
}

template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::WriteOutputXmlFile()
{
    std::ofstream xmlFile;
    xmlFile.open(m_TargetFileName, std::ios_base::trunc | std::ios_base::out);
    xmlFile << "<fids>\n";

    typename FileFieldsContainer::iterator fileFieldContainerIt;
    for (fileFieldContainerIt = m_FileFieldsContainer.begin(); fileFieldContainerIt != m_FileFieldsContainer.end();
        ++fileFieldContainerIt)
    {
        WriteEntriesToXmlOutputFile(xmlFile, fileFieldContainerIt->second);
    }
    xmlFile << "</fids>";
}

template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::WriteEntriesToXmlOutputFile(std::ofstream &outStream, FileFieldsInfoType &fileFieldsInfos)
{
    // now sort the resVect
    std::sort (fileFieldsInfos.fieldsEntries.begin(), fileFieldsInfos.fieldsEntries.end(), m_LinesComparator);

    outStream << " <fid id=\"" << fileFieldsInfos.fid.c_str() << "\" name=\"" << fileFieldsInfos.fileName.c_str() << "\">\n";

    for (int i = 0; i<fileFieldsInfos.fieldsEntries.size(); i++) {
        const std::vector<double> &curLineVect = fileFieldsInfos.fieldsEntries[i].values;
        outStream << "  <info date=\"" << TimeToString(fileFieldsInfos.fieldsEntries[i].date).c_str() << "\" ";
        if (fileFieldsInfos.fieldsEntries[i].additionalFileDate != 0) {
            outStream << "date2=\"" << TimeToString(fileFieldsInfos.fieldsEntries[i].additionalFileDate).c_str() << "\" ";
        }
        for (int j = 0; j<curLineVect.size(); j++) {
            //outStream << m_HeaderFields[j + 2] << "=\"" << boost::lexical_cast<std::string>(curLineVect[j]).c_str() << "\" ";
            outStream << m_HeaderFields[j + 2] << "=\"" << DoubleToString(curLineVect[j]).c_str() << "\" ";
        }
        outStream << "/>\n";
    }
    outStream << " " << "</fid>\n";
}

template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::WriteOutputCsvFormat()
{
    if (m_bMultiFileMode) {
        typename FileFieldsContainer::iterator fileFieldContainerIt;
        for (fileFieldContainerIt = m_FileFieldsContainer.begin(); fileFieldContainerIt != m_FileFieldsContainer.end();
            ++fileFieldContainerIt)
        {
            std::string fileName = fileFieldContainerIt->first;
            NormalizeFieldId(fileName);
            const std::string &targetFile = GetIndividualFieldFileName(m_TargetFileName, fileName);
            std::ofstream fileStream;
            fileStream.open(targetFile, std::ios_base::trunc | std::ios_base::out);
            if (!fileStream.is_open()) {
                std::cout << "Cannot open the file " << targetFile << std::endl;
                continue;
            }

            // write the header for the file
            WriteCsvHeader(fileStream, true);

            // write the entries in the file
            WriteEntriesToCsvOutputFile(fileStream, fileFieldContainerIt->second, false);
        }

    } else {
        std::ofstream fileStream;
        fileStream.open(m_TargetFileName, std::ios_base::trunc | std::ios_base::out);

        // write the header
        WriteCsvHeader(fileStream, false);

        typename FileFieldsContainer::iterator fileFieldContainerIt;
        for (fileFieldContainerIt = m_FileFieldsContainer.begin(); fileFieldContainerIt != m_FileFieldsContainer.end();
            ++fileFieldContainerIt)
        {
            WriteEntriesToCsvOutputFile(fileStream, fileFieldContainerIt->second, true);
        }
    }
}

template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::WriteEntriesToCsvOutputFile(std::ofstream &outStream, FileFieldsInfoType &fileFieldsInfos, bool writeSuffix)
{
    // Remove the duplicate items
    RemoveDuplicates(fileFieldsInfos.fieldsEntries);

    // now sort the resVect
    // Normaly this is not needed anymore as they are sorted during duplicates removal
    // std::sort (fileFieldsInfos.fieldsEntries.begin(), fileFieldsInfos.fieldsEntries.end(), m_LinesComparator);

    outStream << fileFieldsInfos.fid.c_str() << ";";
    if (writeSuffix) {
        outStream << fileFieldsInfos.fileName.c_str() << ";";
    }

    int fieldEntriesSize = fileFieldsInfos.fieldsEntries.size();
    for (int i = 0; i<fieldEntriesSize; i++) {
        const std::vector<double> &curLineVect = fileFieldsInfos.fieldsEntries[i].values;
        outStream << TimeToString(fileFieldsInfos.fieldsEntries[i].date).c_str() << ";";
        if (fileFieldsInfos.fieldsEntries[i].additionalFileDate != 0) {
            outStream << TimeToString(fileFieldsInfos.fieldsEntries[i].additionalFileDate).c_str() << ";";
        }
        for (int j = 0; j<curLineVect.size(); j++) {
            //outStream << boost::lexical_cast<std::string>(curLineVect[j]).c_str() << ";";
            outStream << DoubleToString(curLineVect[j]).c_str();
            if (j < curLineVect.size() - 1 ) {
                outStream << ";";
            }
        }
        outStream << (m_bCsvCompactMode ? ((i == fieldEntriesSize - 1) ? "\n" : "|") : "\n");
    }
}

template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::RemoveDuplicates(std::vector<FieldEntriesType> &fieldsEntries) {

    auto comp = [this] ( const FieldEntriesType& lhs, const FieldEntriesType& rhs ) {
        // TODO: For now we keep the ones that have different mean values
        return ((lhs.date == rhs.date) && (lhs.additionalFileDate == rhs.additionalFileDate)/* &&
                (lhs.values[m_MeanPosInHeader] == rhs.values[m_MeanPosInHeader])*/);};

    auto pred = []( const FieldEntriesType& lhs, const FieldEntriesType& rhs ) {return (lhs.date < rhs.date);};
    std::sort(fieldsEntries.begin(), fieldsEntries.end(), pred);
    auto last = std::unique(fieldsEntries.begin(), fieldsEntries.end(), comp);
    fieldsEntries.erase(last, fieldsEntries.end());
}


template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::WriteCsvHeader(std::ofstream &fileStream, bool individualFieldFile) {
    for (int i = 0; i<m_HeaderFields.size(); i++) {
        fileStream << m_HeaderFields[i];
        if (i < m_HeaderFields.size()-1) {
            fileStream << ";";
        }
        if (m_FileFieldsContainer.size() > 0) {
            if (i == 0) {
                if (!individualFieldFile) {
                    fileStream << "suffix" << ";";
                }
            } else if (i == 1 && m_bUseDate2 != 0) {
                fileStream << "date2" << ";";
            }
        }
    }
    fileStream << "\n";
}

template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::GenerateData()
{
    // Check if the input are not null
    if(m_FileFieldsContainer.size() == 0) {
        std::cout << "At least one input is required, please set input using the methods AddInputMap" << std::endl;
        // Commented to allow writing of even empty files (containing only header), just output the warning
//        return;
    }

    // Check if the filename is not empty
    if(m_TargetFileName.empty()) {
        itkExceptionMacro(<<"The output directory TargetDir is empty, please set the target dir name via the method SetTargetDir");
    }

    if (m_bOutputCsvFormat) {
        WriteOutputCsvFormat();
    } else {
        WriteOutputXmlFile();
    }
}

template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::CleanInputs()
{
}

template < class TMeasurementVector >
int
AgricPractDataExtrFileWriter2<TMeasurementVector>
::GetPositionInHeader(const std::string &name)
{
    // check if the name is found in the headers list
    std::vector<std::string>::iterator hdrIt = std::find(m_HeaderFields.begin(), m_HeaderFields.end(), name);
    if (hdrIt == m_HeaderFields.end())
    {
        return -1;
    }
    // we exclude from the header the fid and the date
    return hdrIt - m_HeaderFields.begin() - 2;
}

template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  // Call superclass implementation
  Superclass::PrintSelf(os, indent);

//  // Print Writer state
//  os << indent << "Output FileName: "<< m_FileName << std::endl;
}

} // End namespace otb

#endif
