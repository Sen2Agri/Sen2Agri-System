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

#ifndef otbOGRDataToClassStatisticsFilter_txx
#define otbOGRDataToClassStatisticsFilter_txx

#include "otbOGRDataToClassStatisticsFilter.h"

namespace otb
{
// --------- otb::PersistentOGRDataToClassStatisticsFilter ---------------------

template<class TInputImage, class TMaskImage>
PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::PersistentOGRDataToClassStatisticsFilter()
{
  this->SetNumberOfRequiredOutputs(2);
  this->SetNthOutput(0,TInputImage::New());
  this->SetNthOutput(1,ClassCountObjectType::New());
    m_ConvertValuesToDecibels = false;
    m_ComputeMinMax = false;
// NOT NEEDED FOR AGRICULTURAL PRACTICES
//  this->SetNthOutput(2,PolygonSizeObjectType::New());
}

template<class TInputImage, class TMaskImage>
void
PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::Synthetize(void)
{
  otb::ogr::DataSource* vectors = const_cast<otb::ogr::DataSource*>(this->GetOGRData());
  vectors->GetLayer(this->GetLayerIndex()).SetSpatialFilter(nullptr);

// NOT NEEDED FOR AGRICULTURAL PRACTICES
//  ClassCountMapType &classCount = this->GetClassCountOutput()->Get();
//  PolygonSizeMapType &polygonSize = this->GetPolygonSizeOutput()->Get();
  
//  // Reset outputs
//  classCount.clear();
//  polygonSize.clear();
//  // Copy temporary stats to outputs
//  for (unsigned int k=0 ; k < this->GetNumberOfThreads() ; k++)
//    {
//    ClassCountMapType::iterator itClass = m_ElmtsInClassThread[k].begin();
//    for (; itClass != m_ElmtsInClassThread[k].end() ; ++itClass)
//      {
//      if (classCount.count(itClass->first))
//        {
//        classCount[itClass->first] += itClass->second;
//        }
//      else
//        {
//        classCount[itClass->first] = itClass->second;
//        }
//      }
//    PolygonSizeMapType::iterator itPoly = m_PolygonThread[k].begin();
//    for (; itPoly != m_PolygonThread[k].end() ; ++itPoly)
//      {
//      if (polygonSize.count(itPoly->first))
//        {
//        polygonSize[itPoly->first] += itPoly->second;
//        }
//      else
//        {
//        polygonSize[itPoly->first] = itPoly->second;
//        }
//      }
//    }

  // Update temporary accumulator
   AccumulatorMapType outputAcc;

   for (auto const& threadAccMap: m_AccumulatorMaps)
     {
     for(auto const& it: threadAccMap)
       {
       const std::string label = it.first;
       if (outputAcc.count(label) <= 0)
         {
         AccumulatorType newAcc(it.second);
         outputAcc[label] = newAcc;
         }
       else
         {
         outputAcc[label].Update(it.second);
         }
       }
     }

   // Publish output maps
   for(auto& it: outputAcc)
   {
     const std::string label = it.first;
     const RealVectorPixelType count = it.second.GetCount();
     const RealVectorPixelType countInvalid = it.second.GetCountInvalid();
     const RealVectorPixelType sum   = it.second.GetSum();
     const RealVectorPixelType sqSum = it.second.GetSqSum();

//     std::cout << "======================" << std::endl;
//     std::cout << "Pixel values for label " << label << std::endl;
//     for(int i = 0; i<it.second.m_Pixels.size(); i++) {
//         const RealVectorPixelType &pixel = it.second.m_Pixels[i];
//         for (int j = 0; j<pixel.GetSize(); j++) {
//             std::cout << pixel[j] << std::endl;
//         }
//     }

     // Mean & stdev
     RealVectorPixelType mean (sum);
     RealVectorPixelType std (sqSum);
     bool noBandOk = true;
     for (unsigned int band = 0 ; band < mean.GetSize() ; band++)
     {
         // TODO: If population standard deviation is computed, then use count[band] > 0
         if ((count[band] > 1) && (count[band] > 0.1 * (count[band] + countInvalid[band]))) {
             noBandOk = false;
            // Mean
            mean[band] /= count[band];

            // Unbiased standard deviation (not sure unbiased is usefull here)
            // Compute sample standard deviation
            const double variance = (sqSum[band] - (sum[band] * mean[band])) / (count[band] - 1);
            // Compute population standard deviation
            // const double variance = (sqSum[band] - (sum[band] * mean[band])) / (count[band]);
            std[band] = vcl_sqrt(variance);
         }
     }
//     std::cout << "Standard deviation is: " << std << std::endl;
//     std::cout << "Mean is: " << mean << std::endl;
     if (!noBandOk)
     {
//         std::cout << "======================" << std::endl;
//         std::cout << "Pixel values for label " << label << std::endl;
//         for(int i = 0; i<it.second.m_Pixels.size(); i++) {
//             const RealVectorPixelType &pixel = it.second.m_Pixels[i];
//             for (int j = 0; j<pixel.GetSize(); j++) {
//                 std::cout << pixel[j] << std::endl;
//             }
//         }

         // USED m_MeanStdDevRadiometricValue INSTEAD
//         m_MeanRadiometricValue[label] = mean;
//         m_StDevRadiometricValue[label] = std;
         m_MeanStdDevRadiometricValue[label].mean = mean;
         m_MeanStdDevRadiometricValue[label].stdDev = std;

         // Min & max
         if (m_ComputeMinMax) {
            m_MinRadiometricValue[label] = it.second.GetMin();
            m_MaxRadiometricValue[label] = it.second.GetMax();

            m_ValidPixelsCnt[label] = count;
            m_InvalidPixelsCnt[label] = countInvalid;
         }
     }
   }

  m_AccumulatorMaps.clear();
  m_NbPixelsThread.clear();

// NOT NEEDED FOR AGRICULTURAL PRACTICES
//  m_ElmtsInClassThread.clear();
//  m_PolygonThread.clear();
}

template<class TInputImage, class TMaskImage>
void
PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::Reset(void)
{
// NOT NEEDED FOR AGRICULTURAL PRACTICES
//  m_ElmtsInClassThread.clear();
//  m_PolygonThread.clear();
  m_NbPixelsThread.clear();

  m_AccumulatorMaps.clear();
// USED m_MeanStdDevRadiometricValue INSTEAD
//  m_MeanRadiometricValue.clear();
//  m_StDevRadiometricValue.clear();
  m_MeanStdDevRadiometricValue.clear();

  m_MinRadiometricValue.clear();
  m_MaxRadiometricValue.clear();
  m_ValidPixelsCnt.clear();
  m_InvalidPixelsCnt.clear();

// NOT NEEDED FOR AGRICULTURAL PRACTICES
//  m_ElmtsInClassThread.resize(this->GetNumberOfThreads());
//  m_PolygonThread.resize(this->GetNumberOfThreads());
  m_NbPixelsThread.resize(this->GetNumberOfThreads());
  m_CurrentClass.resize(this->GetNumberOfThreads());
  m_CurrentFID.resize(this->GetNumberOfThreads());

  m_AccumulatorMaps.resize(this->GetNumberOfThreads());
}

template<class TInputImage, class TMaskImage>
const typename PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::ClassCountObjectType*
PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetClassCountOutput() const
{
  if (this->GetNumberOfOutputs()<2)
    {
    return nullptr;
    }
  return static_cast<const ClassCountObjectType *>(this->itk::ProcessObject::GetOutput(1));
}

template<class TInputImage, class TMaskImage>
typename PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::ClassCountObjectType* 
PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetClassCountOutput()
{
  if (this->GetNumberOfOutputs()<2)
    {
    return nullptr;
    }
  return static_cast<ClassCountObjectType *>(this->itk::ProcessObject::GetOutput(1));
}

// NOT NEEDED FOR AGRICULTURAL PRACTICES
//template<class TInputImage, class TMaskImage>
//const typename PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::PolygonSizeObjectType*
//PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
//::GetPolygonSizeOutput() const
//{
//  if (this->GetNumberOfOutputs()<3)
//    {
//    return nullptr;
//    }
//  return static_cast<const PolygonSizeObjectType *>(this->itk::ProcessObject::GetOutput(2));
//}

//template<class TInputImage, class TMaskImage>
//typename PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::PolygonSizeObjectType*
//PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
//::GetPolygonSizeOutput()
//{
//    if (this->GetNumberOfOutputs()<3)
//    {
//    return nullptr;
//    }
//  return static_cast<PolygonSizeObjectType *>(this->itk::ProcessObject::GetOutput(2));
//}

  // USED GetMeanStdDevValueMap INSTEAD
//template<class TInputImage, class TMaskImage>
//typename PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::PixelValueMapType
//PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
//::GetMeanValueMap() const
//{
//    return m_MeanRadiometricValue;
//}

//template<class TInputImage, class TMaskImage>
//typename PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::PixelValueMapType
//PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
//::GetStandardDeviationValueMap() const
//{
//    return m_StDevRadiometricValue;
//}

template<class TInputImage, class TMaskImage>
typename PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::PixeMeanStdDevlValueMapType
PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetMeanStdDevValueMap() const
{
    return m_MeanStdDevRadiometricValue;
}


template<class TInputImage, class TMaskImage>
typename PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::PixelValueMapType
PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetMinValueMap() const
{
    return m_MinRadiometricValue;
}

template<class TInputImage, class TMaskImage>
typename PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::PixelValueMapType
PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetMaxValueMap() const
{
    return m_MaxRadiometricValue;
}

template<class TInputImage, class TMaskImage>
typename PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::PixelValueMapType
PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetValidPixelsCntMap() const
{
    return m_ValidPixelsCnt;
}

template<class TInputImage, class TMaskImage>
typename PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::PixelValueMapType
PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetInvalidPixelsCntMap() const
{
    return m_InvalidPixelsCnt;
}

template<class TInputImage, class TMaskImage>
itk::DataObject::Pointer
PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::MakeOutput(DataObjectPointerArraySizeType idx)
{
  switch (idx)
    {
    case 0:
      return static_cast<itk::DataObject*>(TInputImage::New().GetPointer());
      break;
    case 1:
      return static_cast<itk::DataObject*>(ClassCountObjectType::New().GetPointer());
      break;
// NOT NEEDED FOR AGRICULTURAL PRACTICES
//    case 2:
//      return static_cast<itk::DataObject*>(PolygonSizeObjectType::New().GetPointer());
//      break;
    default:
      // might as well make an image
      return static_cast<itk::DataObject*>(TInputImage::New().GetPointer());
      break;
    }
}

template<class TInputImage, class TMaskImage>
void
PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::ProcessSample(const ogr::Feature&,
  typename TInputImage::IndexType&,
  typename TInputImage::PointType&,
  const typename TInputImage::PixelType &value,
  itk::ThreadIdType& threadid)
{
  std::string& className = m_CurrentClass[threadid];

// NOT NEEDED FOR AGRICULTURAL PRACTICES
//  unsigned long& fId = m_CurrentFID[threadid];
//  m_ElmtsInClassThread[threadid][className]++;
//  m_PolygonThread[threadid][fId]++;

  m_NbPixelsThread[threadid]++;
  typename TInputImage::PixelType newValue = value;
  for (unsigned int band = 0 ; band < value.GetSize() ; band++) {
      if (std::isnan(value[band])) {
          newValue[band] = -10000;
      } else {
          if (m_ConvertValuesToDecibels) {
              if (value[band] <= 0) {
                  //invalidValCnt++;
                  newValue[band] = -10000;
              } else {
                    newValue[band] = 10 * log10(value[band]);
              }
          }
      }
  }

//  if (className == "584109910/2") {
//      if (newValue[0] != -10000) {
//        std::cout << value[0] << std::endl;
//      }
//  }

    // Update the accumulator
    if (m_AccumulatorMaps[threadid].count(className) <= 0) //add new element to the map
    {
        AccumulatorType newAcc(newValue);
        m_AccumulatorMaps[threadid][className] = newAcc;
    }
    else
    {
        m_AccumulatorMaps[threadid][className].Update(newValue);
    }
}

template<class TInputImage, class TMaskImage>
void
PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::PrepareFeature(const ogr::Feature& feature,
                 itk::ThreadIdType& threadid)
{
  std::string className(feature.ogr().GetFieldAsString(this->GetFieldIndex()));
  unsigned long fId = feature.ogr().GetFID();
// NOT NEEDED FOR AGRICULTURAL PRACTICES
//  if (!m_ElmtsInClassThread[threadid].count(className))
//    {
//    m_ElmtsInClassThread[threadid][className] = 0;
//    }
//  if (!m_PolygonThread[threadid].count(fId))
//    {
//    m_PolygonThread[threadid][fId] = 0;
//    }
  m_CurrentClass[threadid] = className;
  m_CurrentFID[threadid] = fId;
}

// -------------- otb::OGRDataToClassStatisticsFilter --------------------------

template<class TInputImage, class TMaskImage>
void
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::SetInput(const TInputImage* image)
{
  this->GetFilter()->SetInput(image);
}

template<class TInputImage, class TMaskImage>
const TInputImage*
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetInput()
{
  return this->GetFilter()->GetInput();
}

template<class TInputImage, class TMaskImage>
void
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::SetOGRData(const otb::ogr::DataSource* data)
{
  this->GetFilter()->SetOGRData(data);
}

template<class TInputImage, class TMaskImage>
const otb::ogr::DataSource*
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetOGRData()
{
  return this->GetFilter()->GetOGRData();
}

template<class TInputImage, class TMaskImage>
void
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::SetMask(const TMaskImage* mask)
{
  this->GetFilter()->SetMask(mask);
}

template<class TInputImage, class TMaskImage>
const TMaskImage*
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetMask()
{
  return this->GetFilter()->GetMask();
}

template<class TInputImage, class TMaskImage>
void
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::SetFieldName(const std::string &key)
{
  this->GetFilter()->SetFieldName(key);
}

template<class TInputImage, class TMaskImage>
std::string
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetFieldName()
{
  return this->GetFilter()->GetFieldName();
}

template<class TInputImage, class TMaskImage>
void
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::SetFieldValueFilterIds(const std::map<std::string, int> &filters)
{
  this->GetFilter()->SetFieldValueFilterIds(filters);
}

template<class TInputImage, class TMaskImage>
std::map<std::string, int>
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetFieldValueFilterIds()
{
  return this->GetFilter()->GetFieldValueFilterIds();
}

template<class TInputImage, class TMaskImage>
void
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::SetConvertValuesToDecibels(bool exp)
{
  this->GetFilter()->SetConvertValuesToDecibels(exp);
}

template<class TInputImage, class TMaskImage>
bool OGRDataToClassStatisticsFilter<TInputImage, TMaskImage>::GetConvertValuesToDecibels()
{
  return this->GetFilter()->GetConvertValuesToDecibels();
}

template<class TInputImage, class TMaskImage>
void
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::SetComputeMinMax(bool exp)
{
  this->GetFilter()->SetComputeMinMax(exp);
}

template<class TInputImage, class TMaskImage>
bool OGRDataToClassStatisticsFilter<TInputImage, TMaskImage>::GetComputeMinMax()
{
  return this->GetFilter()->GetComputeMinMax();
}

template<class TInputImage, class TMaskImage>
void
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::SetLayerIndex(int index)
{
  this->GetFilter()->SetLayerIndex(index);
}

template<class TInputImage, class TMaskImage>
int
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetLayerIndex()
{
  return this->GetFilter()->GetLayerIndex();
}

template<class TInputImage, class TMaskImage>
const typename OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::ClassCountObjectType*
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetClassCountOutput() const
{
  return this->GetFilter()->GetClassCountOutput();
}

template<class TInputImage, class TMaskImage>
typename OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::ClassCountObjectType*
OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
::GetClassCountOutput()
{
  return this->GetFilter()->GetClassCountOutput();
}

//template<class TInputImage, class TMaskImage>
//const typename OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::PolygonSizeObjectType*
//OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
//::GetPolygonSizeOutput() const
//{
//  return this->GetFilter()->GetPolygonSizeOutput();
//}

//template<class TInputImage, class TMaskImage>
//typename OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>::PolygonSizeObjectType*
//OGRDataToClassStatisticsFilter<TInputImage,TMaskImage>
//::GetPolygonSizeOutput()
//{
//  return this->GetFilter()->GetPolygonSizeOutput();
//}

  // USED GetMeanStdDevValueMap INSTEAD
//template<class TInputImage, class TMaskImage>
//typename OGRDataToClassStatisticsFilter<TInputImage, TMaskImage>::PixelValueMapType
//OGRDataToClassStatisticsFilter<TInputImage, TMaskImage>
//::GetMeanValueMap() const
//{
//  return this->GetFilter()->GetMeanValueMap();
//}

//template<class TInputVectorImage, class TLabelImage>
//typename OGRDataToClassStatisticsFilter<TInputVectorImage, TLabelImage>::PixelValueMapType
//OGRDataToClassStatisticsFilter<TInputVectorImage, TLabelImage>
//::GetStandardDeviationValueMap() const
//{
//  return this->GetFilter()->GetStandardDeviationValueMap();
//}

template<class TInputVectorImage, class TLabelImage>
typename OGRDataToClassStatisticsFilter<TInputVectorImage, TLabelImage>::PixeMeanStdDevlValueMapType
OGRDataToClassStatisticsFilter<TInputVectorImage, TLabelImage>
::GetMeanStdDevValueMap() const
{
  return this->GetFilter()->GetMeanStdDevValueMap();
}

template<class TInputImage, class TMaskImage>
typename OGRDataToClassStatisticsFilter<TInputImage, TMaskImage>::PixelValueMapType
OGRDataToClassStatisticsFilter<TInputImage, TMaskImage>
::GetMinValueMap() const
{
  return this->GetFilter()->GetMinValueMap();
}

template<class TInputImage, class TMaskImage>
typename OGRDataToClassStatisticsFilter<TInputImage, TMaskImage>::PixelValueMapType
OGRDataToClassStatisticsFilter<TInputImage, TMaskImage>
::GetMaxValueMap() const
{
  return this->GetFilter()->GetMaxValueMap();
}

template<class TInputImage, class TMaskImage>
typename OGRDataToClassStatisticsFilter<TInputImage, TMaskImage>::PixelValueMapType
OGRDataToClassStatisticsFilter<TInputImage, TMaskImage>
::GetValidPixelsCntMap() const
{
  return this->GetFilter()->GetValidPixelsCntMap();
}

template<class TInputImage, class TMaskImage>
typename OGRDataToClassStatisticsFilter<TInputImage, TMaskImage>::PixelValueMapType
OGRDataToClassStatisticsFilter<TInputImage, TMaskImage>
::GetInvalidPixelsCntMap() const
{
  return this->GetFilter()->GetInvalidPixelsCntMap();
}

} // end of namespace otb

#endif
