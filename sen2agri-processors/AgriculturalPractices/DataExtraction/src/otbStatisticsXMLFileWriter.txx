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

#ifndef otbStatisticsXMLFileWriter_txx
#define otbStatisticsXMLFileWriter_txx

#include "otbStatisticsXMLFileWriter.h"
#include "itkMacro.h"
#include "itksys/SystemTools.hxx"
#include "otb_tinyxml.h"
#include "otbStringUtils.h"

namespace otb {


template < class TMeasurementVector >
StatisticsXMLFileWriter<TMeasurementVector>
::StatisticsXMLFileWriter(): m_FileName("")
{}


template < class TMeasurementVector >
void
StatisticsXMLFileWriter<TMeasurementVector>
::AddInput(const std::string &fileName, const std::string &name,  const MeasurementVectorType& inputVector )
{
  InputDataType    inputData;
  inputData.first  = name;

  ContainersType containers;
  auto containerIt = m_Containers.find(fileName);
  if(containerIt != m_Containers.end())
  {
      containers = containerIt->second;
  }

  // Check if the statistic name is already added
  for(unsigned int idx= 0; idx< containers.measContainer.size(); ++idx)
    {
    if(strcmp(containers.measContainer[idx].first.c_str(), name) == 0 )
      {
      itkExceptionMacro(<<"Token selected ("
                        <<name<<") is already added to the XML file");
      }
    }

  inputData.second = inputVector;
  containers.measContainer.push_back(inputData);
  m_Containers[fileName] = containers;
}

template < class TMeasurementVector >
void
StatisticsXMLFileWriter<TMeasurementVector>
::GenerateData()
{
  // Check if the input are not null
  if(m_Containers.size() == 0)
    itkExceptionMacro(<<"At least one input is required, please set input using the methods AddInput or AddInputMap");

  // Check if the filename is not empty
  if(m_FileName.empty())
    itkExceptionMacro(<<"The XML output FileName is empty, please set the filename via the method SetFileName");

  // Check that the right extension is given : expected .xml */
  const std::string extension = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
  if (itksys::SystemTools::LowerCase(extension) != ".xml")
    {
    itkExceptionMacro(<<extension
                      <<" is a wrong Extension FileName : Expected .xml");
    }

  // Write the XML file
  TiXmlDocument doc;

  TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );
  doc.LinkEndChild( decl );

  typename FileContainers::const_iterator fileContainerIt;
  for ( fileContainerIt = m_Containers.begin() ; fileContainerIt != m_Containers.end() ; ++fileContainerIt)
  {
      TiXmlElement *fileRoot = new TiXmlElement( "File");
      fileRoot->SetAttribute("name", fileContainerIt->first.c_str());
      doc.LinkEndChild( fileRoot );

      const MeasurementVectorContainer &measContainer = fileContainerIt->second.measContainer;
      if (measContainer.size())
      {
          TiXmlElement * features = new TiXmlElement( "FeatureStatistics");
          fileRoot->LinkEndChild( features );

          // Iterate through the input
          for (unsigned int i = 0; i < measContainer.size(); ++i)
          {
            std::string            featureName              = measContainer[i].first;
            MeasurementVectorType  currentMeasurementVector = measContainer[i].second;

            // The current statistic
            TiXmlElement * feature = new TiXmlElement("Statistic");
            feature->SetAttribute("name", featureName.c_str());
            features->LinkEndChild( feature );

            // Store the value for this statistic
            for(unsigned int cindex = 0; cindex < currentMeasurementVector.Size(); ++cindex)
            {
              // For each value in Measurementvector
              TiXmlElement * curStatisticVector = new TiXmlElement("StatisticVector");
              curStatisticVector->SetDoubleAttribute("value", currentMeasurementVector.GetElement(cindex));
              feature->LinkEndChild(curStatisticVector);
            }
          }
      }

      const GenericMapContainer &mapContainer = fileContainerIt->second.mapsContainer;
      // Iterate on map containers
      TiXmlElement * mapRoot = nullptr;
      if (mapContainer.size())
        {
        mapRoot = new TiXmlElement( "GeneralStatistics");
        fileRoot->LinkEndChild( mapRoot );
        }

      GenericMapContainer::const_iterator containerIt;
      for ( containerIt = mapContainer.begin() ; containerIt != mapContainer.end() ; ++containerIt)
        {
        std::string mapName = containerIt->first;
        GenericMapType::const_iterator mapIter;

        // The current statistic
        TiXmlElement * feature = new TiXmlElement("Statistic");
        feature->SetAttribute("name", mapName.c_str());
        mapRoot->LinkEndChild( feature );

        // Store the value for this statistic
        for( mapIter = containerIt->second.begin() ; mapIter != containerIt->second.end() ; ++mapIter )
          {
          // For each value in Measurementvector
          TiXmlElement * curStatisticMap = new TiXmlElement("StatisticMap");
          curStatisticMap->SetAttribute("key" , mapIter->first.c_str());
          curStatisticMap->SetAttribute("value", mapIter->second.c_str());
          feature->LinkEndChild(curStatisticMap);
          }
        }
    }

  // Finally, write the file
  if (! doc.SaveFile( m_FileName.c_str() ) )
    {
    itkExceptionMacro(<<"Unable to write the XML file in "
                      << itksys::SystemTools::GetFilenamePath(m_FileName)
                      << " (permission issue? Directory does not exist?)." );
    }

}

template < class TMeasurementVector >
template <typename MapType>
void
StatisticsXMLFileWriter<TMeasurementVector>
::AddInputMap(const std::string &fileName, const std::string &name, const MapType& map )
{
  ContainersType fileContainer;
  auto containerIt = m_Containers.find(fileName);
  if(containerIt != m_Containers.end())
  {
      fileContainer = containerIt->second;
  }

  std::string token(name);
  if(fileContainer.mapsContainer.count(token) > 0)
    {
    itkExceptionMacro(<<"Token selected ("
                      <<name<<") is already added to the XML file");
    }
  
  typename MapType::const_iterator it;
  GenericMapType insideMap;
  std::string tmpKey;
  std::string tmpVal;
  for ( it = map.begin() ; it != map.end() ; ++it)
    {
    tmpKey = boost::lexical_cast<std::string>(it->first);
    tmpVal = boost::lexical_cast<std::string>(it->second);
    insideMap[tmpKey] = tmpVal;
    }
  fileContainer.mapsContainer[token] = insideMap;
  m_Containers[fileName] = fileContainer;
}

template < class TMeasurementVector >
void
StatisticsXMLFileWriter<TMeasurementVector>
::CleanInputs()
{
  // clear both containers
  m_Containers.clear();
}

template < class TMeasurementVector >
void
StatisticsXMLFileWriter<TMeasurementVector>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  // Call superclass implementation
  Superclass::PrintSelf(os, indent);

  // Print Writer state
  os << indent << "Output FileName: "<< m_FileName << std::endl;

  for (typename FileContainers::const_iterator filesIt = m_Containers.begin(); filesIt != m_Containers.end() ; ++filesIt)
  {
      os << indent << "Input file name: "<< filesIt->first.c_str() << std::endl;
      os << indent << "Vector statistics: ";
      for (unsigned int i=0 ; i < filesIt->second.measContainer.size() ; ++i)
        {
        if (i>0) os <<", ";
        os << filesIt->second.measContainer[i].first;
        }
      os << std::endl;
      os << indent << "Map statistics: ";
      for (GenericMapContainer::const_iterator it = filesIt->second.mapsContainer.begin() ;
           it != filesIt->second.mapsContainer.end() ; ++it)
        {
        if (it != filesIt->second.mapsContainer.begin()) os <<", ";
        os << it->first;
        }
  }
  os << std::endl;
}

} // End namespace otb

#endif
