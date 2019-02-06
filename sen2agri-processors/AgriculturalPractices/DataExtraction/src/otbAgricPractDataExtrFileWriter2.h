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

#ifndef otbAgricPractDataExtrFileWriter2_h
#define otbAgricPractDataExtrFileWriter2_h

#include "itkProcessObject.h"
#include <utility>
#include <string>

namespace otb {

/** \class AgricPractDataExtrFileWriter2
 *  \brief Write in a text file the values stored in a MeasurementVector set as
 *  input
 *
 * The vector can be set as input via AddInput(name, vector) where name
 * is the name of the statistic, and vector the values.
 * Supported vector types are those implementing the method GetElement(idx)
 * and defining the type ValueType.
 *
 *
 * \ingroup OTBIOXML
 */
template < class TMeasurementVector>
class  AgricPractDataExtrFileWriter2 :
    public itk::Object
{
public:
  /** Standard class typedefs */
  typedef AgricPractDataExtrFileWriter2          Self;
  typedef itk::Object                      Superclass;
  typedef itk::SmartPointer< Self >        Pointer;
  typedef itk::SmartPointer<const Self>    ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(AgricPractDataExtrFileWriter2, itk::Object);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  typedef struct {
     time_t date;
     time_t additionalFileDate;
     std::vector<double> values;
  } FieldEntriesType;

  typedef struct {
      std::string fileName;
      std::string fid;
      //std::map<time_t, FieldEntriesType> fieldsEntries;
      std::vector<FieldEntriesType> fieldsEntries;

  } FileFieldsInfoType;

  typedef std::map<std::string, FileFieldsInfoType>         FileFieldsContainer;

  /** Method to add mean and stdev maps with a given type */
//  template <typename MapType>
//  void AddInputMaps(const std::string &fileName, const MapType& mapMeans,  const MapType& mapStdDev);

  /** Method to add a map statistic with a given type */
//  template <typename MapType>
//  void AddInputMap(const std::string &fileName, const MapType& map );

  /** Method to add a map statistic with a given type */
  template <typename MapType, typename MapMinMaxType>
  void AddInputMap(const std::string &fileName, const MapType& map, const MapMinMaxType& mapMins, const MapMinMaxType& mapMax,
                   const MapMinMaxType& mapValidPixels, const MapMinMaxType& mapInvalidPixels);

  void WriteOutputXmlFile();
  void WriteEntriesToXmlOutputFile(std::ofstream &outStream, FileFieldsInfoType &fileFieldsInfos);

  void WriteOutputCsvFormat();
  void WriteCsvHeader(std::ofstream &fileStream, bool individualFieldFile);
  void WriteEntriesToCsvOutputFile(std::ofstream &outStream, FileFieldsInfoType &fileFieldsInfos, bool writeSuffix);
  void RemoveDuplicates(std::vector<FieldEntriesType> &fieldsEntries);

  /** Remove previously added inputs (vectors and maps) */
  void CleanInputs();

  /** Trigger the processing */
  void Update()
  {
    this->GenerateData();
  }

  typedef typename std::vector<std::string>        StringVectorType;

  /** Set the output filename */
  itkSetStringMacro(TargetFileName);
  itkGetStringMacro(TargetFileName);

  /** Set the header fields */
  inline void SetHeaderFields(const StringVectorType &vec) {
      this->m_HeaderFields = vec;
      m_MeanPosInHeader = GetPositionInHeader("mean");
      m_StdevPosInHeader = GetPositionInHeader("stdev");
      m_MinPosInHeader = GetPositionInHeader("min");
      m_MaxPosInHeader = GetPositionInHeader("max");
      m_ValidPixelsPosInHeader = GetPositionInHeader("valid_pixels_cnt");
      m_InvalidPixelsPosInHeader = GetPositionInHeader("invalid_pixels_cnt");
  }
  inline StringVectorType GetHeaderFields() {return this->m_HeaderFields;}

  inline void SetUseLatestFileNamingFormat(bool latestNamingFormat) {
      m_bUseLatestNamingFormat = latestNamingFormat;
  }

  inline void SetOutputCsvFormat(bool bOutCsv) {
      m_bOutputCsvFormat = bOutCsv;
  }

  inline void SetUseMultiFileMode(bool bMultiFileMode) {
      m_bMultiFileMode = bMultiFileMode;
  }

  inline void SetCsvCompactMode(bool bCsvCompactMode) {
      m_bCsvCompactMode = bCsvCompactMode;
  }

  inline void SetUseMinMax(bool bUseMinMax) {
      m_bUseMinMax = bUseMinMax;
  }

protected:

  virtual void GenerateData();

  AgricPractDataExtrFileWriter2();
  ~AgricPractDataExtrFileWriter2() override {}
  void PrintSelf(std::ostream& os, itk::Indent indent) const override;

private:

  int GetPositionInHeader(const std::string &name);
  typedef struct {
        void SetVectComparisonIndex(int idx) {this->m_index = idx;}
        bool operator() (const FieldEntriesType &i, const FieldEntriesType &j) {
            // return ((i.date.compare(j.date)) < 0);
            return (i.date < j.date);
        }

        private:
            int m_index;
  } LinesComparator;

  LinesComparator m_LinesComparator;
  AgricPractDataExtrFileWriter2(const Self&) = delete;
  void operator=(const Self&) = delete;

  std::string                 m_TargetFileName;

  std::vector<std::string>      m_HeaderFields;
  int                           m_HdrSortingIdx;

  FileFieldsContainer           m_FileFieldsContainer;

  bool                          m_bOutputCsvFormat;
  bool                          m_bCsvCompactMode;
  bool                          m_bMultiFileMode;
  bool                          m_bUseLatestNamingFormat;
  bool                          m_bUseDate2;
  bool                          m_bUseMinMax;

private:
    int m_MeanPosInHeader;
    int m_StdevPosInHeader;
    int m_MinPosInHeader;
    int m_MaxPosInHeader;
    int m_ValidPixelsPosInHeader;
    int m_InvalidPixelsPosInHeader;
}; // end of class AgricPractDataExtrFileWriter2

} // end of namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbAgricPractDataExtrFileWriter2.txx"
#endif

#endif
