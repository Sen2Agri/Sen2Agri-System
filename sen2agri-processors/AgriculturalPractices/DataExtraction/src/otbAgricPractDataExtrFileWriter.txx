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

#ifndef otbAgricPractDataExtrFileWriter_txx
#define otbAgricPractDataExtrFileWriter_txx

#include "otbAgricPractDataExtrFileWriter.h"
#include "itkMacro.h"
#include "itksys/SystemTools.hxx"
#include "otb_tinyxml.h"
#include "otbStringUtils.h"
#include "CommonFunctions.h"

namespace otb {

template < class TMeasurementVector >
AgricPractDataExtrFileWriter<TMeasurementVector>
::AgricPractDataExtrFileWriter(): m_TargetDir("")
{}

template < class TMeasurementVector >
template <typename MapType>
void
AgricPractDataExtrFileWriter<TMeasurementVector>
::AddInputMap(const std::string &fileName, const std::string &name, const MapType& map )
{
    // check if the name is found in the headers list
    std::vector<std::string>::iterator hdrIt = std::find(m_HeaderFields.begin(), m_HeaderFields.end(), name);
    if (hdrIt == m_HeaderFields.end())
    {
        // TODO: log error here
        return;
    }
    // we exclude from the header the fid and the date
    int posInHeader = hdrIt - m_HeaderFields.begin() - 2;

    std::string fileType;
    std::string polarisation;
    std::string orbit;
    time_t fileDate;
    time_t additionalFileDate;
    if (!getFileInfosFromName(fileName, fileType, polarisation, orbit, fileDate, additionalFileDate))
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

      typename std::map<std::string, FileFieldsInfoType>::iterator containerIt =
              m_FileFieldsContainer.find(fieldOutFileName);
      if(containerIt != m_FileFieldsContainer.end()) {
          // now search the corresponding line if exists
          typename std::vector<FieldEntriesType>::iterator it2;
          bool entryFound = false;
          for(it2 = containerIt->second.fieldsEntries.begin(); it2 != containerIt->second.fieldsEntries.end(); it2++) {
              if (it2->date == fileDate) {
                  it2->values[posInHeader] = it->second[0];
                  entryFound = true;
                  break;
              }
          }
          if (!entryFound) {
              FieldEntriesType fieldEntry;
              fieldEntry.date = fileDate;
              fieldEntry.date2 = additionalFileDate;
              // we exclude from the header the fid and the date
              fieldEntry.values.resize(m_HeaderFields.size() - 2);
              fieldEntry.values[posInHeader] = it->second[0];
              containerIt->second.fieldsEntries.push_back(fieldEntry);
          }

      } else {
          // add it into the container
          FileFieldsInfoType fileFieldsInfoType;
          fileFieldsInfoType.fid = fieldId;
          fileFieldsInfoType.fileName = fieldOutFileName;

          FieldEntriesType fieldEntry;
          fieldEntry.date = fileDate;
          fieldEntry.date2 = additionalFileDate;
          // we exclude from the header the fid and the date
          fieldEntry.values.resize(m_HeaderFields.size() - 2);
          fieldEntry.values[posInHeader] = it->second[0];
          fileFieldsInfoType.fieldsEntries.push_back(fieldEntry);

          m_FileFieldsContainer[fieldOutFileName] = fileFieldsInfoType;
          //m_FileFieldsContainer.push_back(fileFieldsInfoType);
      }
    }
}

template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter<TMeasurementVector>
::WriteOutputFile(const std::vector<std::string>& headerLine, FileFieldsInfoType &fileFieldsInfos)
{
  boost::filesystem::path rootFolder(m_TargetDir);
  std::string fileFullPath = (rootFolder / fileFieldsInfos.fileName).string();

  std::ofstream norm_file{fileFullPath};
  // write the header
  for (int i = 0; i<headerLine.size(); i++) {
      norm_file << (i == 0 ? "" : ";") << headerLine[i];
  }
  norm_file << std::endl;

  // now sort the resVect
  std::sort (fileFieldsInfos.fieldsEntries.begin(), fileFieldsInfos.fieldsEntries.end(), m_LinesComparator);

  for (int i = 0; i<fileFieldsInfos.fieldsEntries.size(); i++) {
      norm_file << fileFieldsInfos.fid << ";" << TimeToString(fileFieldsInfos.fieldsEntries[i].date);
      const std::vector<double> &curLineVect = fileFieldsInfos.fieldsEntries[i].values;
      for (int j = 0; j<curLineVect.size(); j++) {
        norm_file << ";" << boost::lexical_cast<std::string>(curLineVect[j]);
      }
      norm_file << std::endl;
  }
}


template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter<TMeasurementVector>
::GenerateData()
{
  // Check if the input are not null
  if(m_FileFieldsContainer.size() == 0)
    itkExceptionMacro(<<"At least one input is required, please set input using the methods AddInputMap");

  // Check if the filename is not empty
  if(m_TargetDir.empty()) {
    itkExceptionMacro(<<"The output directory TargetDir is empty, please set the target dir name via the method SetTargetDir");
  }

  typename FileFieldsContainer::iterator fileFieldContainerIt;
  for (fileFieldContainerIt = m_FileFieldsContainer.begin(); fileFieldContainerIt != m_FileFieldsContainer.end();
       ++fileFieldContainerIt)
  {
      WriteOutputFile(m_HeaderFields, fileFieldContainerIt->second);
  }
}

template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter<TMeasurementVector>
::CleanInputs()
{
}

template < class TMeasurementVector >
void
AgricPractDataExtrFileWriter<TMeasurementVector>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  // Call superclass implementation
  Superclass::PrintSelf(os, indent);

//  // Print Writer state
//  os << indent << "Output FileName: "<< m_FileName << std::endl;

//  for (typename FileContainers::const_iterator filesIt = m_Containers.begin(); filesIt != m_Containers.end() ; ++filesIt)
//  {
//      os << indent << "Input file name: "<< filesIt->first.c_str() << std::endl;
//      os << indent << "Vector statistics: ";
//      for (unsigned int i=0 ; i < filesIt->second.measContainer.size() ; ++i)
//        {
//        if (i>0) os <<", ";
//        os << filesIt->second.measContainer[i].first;
//        }
//      os << std::endl;
//      os << indent << "Map statistics: ";
//      for (GenericMapContainer::const_iterator it = filesIt->second.mapsContainer.begin() ;
//           it != filesIt->second.mapsContainer.end() ; ++it)
//        {
//        if (it != filesIt->second.mapsContainer.begin()) os <<", ";
//        os << it->first;
//        }
//  }
//  os << std::endl;
}

} // End namespace otb

#endif
