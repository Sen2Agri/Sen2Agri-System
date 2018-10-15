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
::AgricPractDataExtrFileWriter2(): m_TargetFileName(""), m_AppendMode(false),
    m_MultiFileMode(false), m_bWriteInDb(false), m_bUseLatestNamingFormat(true)
{}

template < class TMeasurementVector >
template <typename MapType>
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::AddInputMap(const std::string &fileName, const MapType& map )
{
    // we exclude from the header the fid and the date
    int meanPosInHeader = GetPositionInHeader("mean");
    int stdevPosInHeader = GetPositionInHeader("stdev");

    std::string fileType;
    std::string polarisation;
    std::string orbit;
    time_t fileDate;
    time_t additionalFileDate = 0;
    if (!getFileInfosFromName(fileName, fileType, polarisation, orbit, fileDate, additionalFileDate, m_bUseLatestNamingFormat))
    {
        // TODO: log error here
        return;
    }

    typename MapType::const_iterator it;
    std::string fieldId;
    for ( it = map.begin() ; it != map.end() ; ++it)
    {
      fieldId = boost::lexical_cast<std::string>(it->first);

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
          fieldEntry.values[meanPosInHeader] = it->second.mean[0];
          fieldEntry.values[stdevPosInHeader] = it->second.stdDev[0];
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
          fieldEntry.values[meanPosInHeader] = it->second.mean[0];
          fieldEntry.values[stdevPosInHeader] = it->second.stdDev[0];
          fileFieldsInfoType.fieldsEntries.push_back(fieldEntry);

          m_FileFieldsContainer[uniqueId] = fileFieldsInfoType;
      }
    }
}

template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::WriteOutputFile()
{
    const std::string strEndFids("</fids>");

  bool validFileExists = false;
  if (m_AppendMode && boost::filesystem::exists(m_TargetFileName)) {
      std::ifstream infile(m_TargetFileName);
      std::string line;
      if (std::getline(infile, line) && line == "<fids>") {
          validFileExists = true;
      }
  }

  std::ofstream xmlFile;
  xmlFile.open(m_TargetFileName, (m_AppendMode ? std::ios_base::app : std::ios_base::trunc) | std::ios_base::out);
  if (validFileExists) {
      long pos = xmlFile.tellp();
      if (pos > 0) {
          // TODO: aparently, this is not working very well - DISABLED for now through m_AppendMode
          // go back to overwrite </fids>
          xmlFile.seekp(pos- (strEndFids.size()));
      }
  } else {
      xmlFile << "<fids>\n";
  }

  typename FileFieldsContainer::iterator fileFieldContainerIt;
  for (fileFieldContainerIt = m_FileFieldsContainer.begin(); fileFieldContainerIt != m_FileFieldsContainer.end();
       ++fileFieldContainerIt)
  {
      WriteEntriesToOutputFile(xmlFile, fileFieldContainerIt->second);
  }
  xmlFile << "</fids>";
}

template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter2<TMeasurementVector>
::WriteEntriesToOutputFile(std::ofstream &outStream, FileFieldsInfoType &fileFieldsInfos)
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
::GenerateData()
{
  // Check if the input are not null
  if(m_FileFieldsContainer.size() == 0)
    itkExceptionMacro(<<"At least one input is required, please set input using the methods AddInputMap");

  // Check if the filename is not empty
  if(m_TargetFileName.empty()) {
    itkExceptionMacro(<<"The output directory TargetDir is empty, please set the target dir name via the method SetTargetDir");
  }

  WriteOutputFile();
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
        // TODO: log error here
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
