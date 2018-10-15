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

#ifndef otbAgricPractDataExtrFileWriter_h
#define otbAgricPractDataExtrFileWriter_h

#include "itkProcessObject.h"
#include <utility>
#include <string>

namespace otb {

/** \class AgricPractDataExtrFileWriter
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
class  AgricPractDataExtrFileWriter :
    public itk::Object
{
public:
  /** Standard class typedefs */
  typedef AgricPractDataExtrFileWriter          Self;
  typedef itk::Object                      Superclass;
  typedef itk::SmartPointer< Self >        Pointer;
  typedef itk::SmartPointer<const Self>    ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(AgricPractDataExtrFileWriter, itk::Object);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  typedef struct {
     time_t date;
     time_t date2;
     std::vector<double> values;
  } FieldEntriesType;

  typedef struct {
      std::string fileName;
      std::string fid;
      std::vector<FieldEntriesType> fieldsEntries;

  } FileFieldsInfoType;

  typedef std::map<std::string, FileFieldsInfoType>         FileFieldsContainer;

  /** Method to add a map statistic with a given type */
  template <typename MapType>
  void AddInputMap(const std::string &fileName, const std::string &name, const MapType& map );

  inline void SetWriteMode(bool append) { m_AppendMode = append;}

  void WriteOutputFile(const std::vector<std::string>& headerLine, FileFieldsInfoType &fileFieldsInfos);

  /** Remove previously added inputs (vectors and maps) */
  void CleanInputs();

  /** Trigger the processing */
  void Update()
  {
    this->GenerateData();
  }

  typedef typename std::vector<std::string>        StringVectorType;

  /** Set the output filename */
  itkSetStringMacro(TargetDir);
  itkGetStringMacro(TargetDir);

  /** Set the header fields */
  inline void SetHeaderFields(const StringVectorType &vec) {
      this->m_HeaderFields = vec;
  }
  inline StringVectorType GetHeaderFields() {return this->m_HeaderFields;}

protected:

  virtual void GenerateData();

  AgricPractDataExtrFileWriter();
  ~AgricPractDataExtrFileWriter() override {}
  void PrintSelf(std::ostream& os, itk::Indent indent) const override;

private:

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
  AgricPractDataExtrFileWriter(const Self&) = delete;
  void operator=(const Self&) = delete;

  std::string                 m_TargetDir;

  std::vector<std::string>      m_HeaderFields;
  int                           m_HdrSortingIdx;

  FileFieldsContainer           m_FileFieldsContainer;

  bool                          m_AppendMode;
  bool                          m_SingleFileMode;

}; // end of class AgricPractDataExtrFileWriter

} // end of namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbAgricPractDataExtrFileWriter.txx"
#endif

#endif
