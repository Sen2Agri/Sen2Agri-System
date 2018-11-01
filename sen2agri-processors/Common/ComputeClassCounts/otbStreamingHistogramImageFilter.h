/*
 * Copyright (C) 1999-2011 Insight Software Consortium
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

#ifndef otbStreamingHistogramImageFilter_h
#define otbStreamingHistogramImageFilter_h

#include "otbPersistentImageFilter.h"
#include "itkNumericTraits.h"
#include "itkArray.h"
#include "itkSimpleDataObjectDecorator.h"
#include "otbPersistentFilterStreamingDecorator.h"

#include <unordered_map>

namespace otb
{

class StatisticsAccumulator
{
public:

  // Constructor (default)
  StatisticsAccumulator() : m_Count(1) {}

  // Constructor (other)
  StatisticsAccumulator(const StatisticsAccumulator & other)
  {
    m_Count = other.m_Count;
  }

  // Destructor
  ~StatisticsAccumulator(){}

  // Function update (pixel)
  void Update()
  {
    m_Count++;
  }

  // Function update (self)
  void Update(const StatisticsAccumulator & other)
  {
    m_Count += other.m_Count;
  }

  // Accessors
  itkGetMacro(Count, uint64_t);

protected:
  uint64_t m_Count;
};

template<class TLabelImage>
class ITK_EXPORT PersistentStreamingHistogramImageFilter :
public PersistentImageFilter<TLabelImage, TLabelImage>
{
public:
  /** Standard Self typedef */
  typedef PersistentStreamingHistogramImageFilter         Self;
  typedef PersistentImageFilter<TLabelImage, TLabelImage> Superclass;
  typedef itk::SmartPointer<Self>                         Pointer;
  typedef itk::SmartPointer<const Self>                   ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(PersistentStreamingHistogramImageFilter, PersistentImageFilter);

  /** Image related typedefs. */
  typedef TLabelImage                         LabelImageType;
  typedef typename TLabelImage::Pointer       LabelImagePointer;

  typedef typename LabelImageType::RegionType                           RegionType;
  typedef typename LabelImageType::PixelType                            LabelPixelType;
  typedef StatisticsAccumulator                                         AccumulatorType;
  typedef std::unordered_map<LabelPixelType, AccumulatorType >          AccumulatorMapType;
  typedef std::vector<AccumulatorMapType>                               AccumulatorMapCollectionType;
  typedef std::unordered_map<LabelPixelType, uint64_t>                  LabelPopulationMapType;

  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TLabelImage::ImageDimension);

  /** Image related typedefs. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TLabelImage::ImageDimension);

  /** Smart Pointer type to a DataObject. */
  typedef typename itk::DataObject::Pointer DataObjectPointer;
  typedef itk::ProcessObject::DataObjectPointerArraySizeType DataObjectPointerArraySizeType;

  typedef itk::ImageBase<InputImageDimension> ImageBaseType;
  typedef typename ImageBaseType::RegionType InputImageRegionType;

  /** Return the computed number of labeled pixels for each label in the input label image */
  LabelPopulationMapType GetLabelPopulationMap() const;

  using Superclass::MakeOutput;

  /** Pass the input through unmodified. Do this by Grafting in the
   *  AllocateOutputs method.
   */
  void AllocateOutputs() override;

  void GenerateOutputInformation() override;

  void Synthetize(void) override;

  void Reset(void) override;

  /** Due to heterogeneous input template GenerateInputRequestedRegion must be reimplemented using explicit cast **/
  /** This new implementation is inspired by the one of itk::ImageToImageFilter **/
  void GenerateInputRequestedRegion() override;

protected:
  PersistentStreamingHistogramImageFilter();
  ~PersistentStreamingHistogramImageFilter() override {}
  void PrintSelf(std::ostream& os, itk::Indent indent) const override;

  void ThreadedGenerateData(const RegionType& outputRegionForThread, itk::ThreadIdType threadId ) override;

private:
  PersistentStreamingHistogramImageFilter(const Self &) = delete;
  void operator =(const Self&) = delete;

  AccumulatorMapCollectionType           m_AccumulatorMaps;

  LabelPopulationMapType                 m_LabelPopulation;

}; // end of class PersistentStreamingHistogramImageFilter


/*===========================================================================*/

template<class TLabelImage>
class ITK_EXPORT StreamingHistogramImageFilter :
public PersistentFilterStreamingDecorator<PersistentStreamingHistogramImageFilter<TLabelImage> >
{
public:
  /** Standard Self typedef */
  typedef StreamingHistogramImageFilter Self;
  typedef PersistentFilterStreamingDecorator
      <PersistentStreamingHistogramImageFilter<TLabelImage> > Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Type macro */
  itkNewMacro(Self);

  /** Creation through object factory macro */
  itkTypeMacro(StreamingHistogramImageFilter, PersistentFilterStreamingDecorator);

  typedef TLabelImage                         LabelImageType;

  typedef typename Superclass::FilterType::LabelPopulationMapType    LabelPopulationMapType;

  /** Set input image */
  using Superclass::SetInput;
  void SetInput(const TLabelImage * input)
  {
    this->GetFilter()->SetInput(input);
  }

  /** Get input image */
  const TLabelImage * GetInput()
  {
    return this->GetFilter()->GetInput();
  }

  /** Return the computed number of labeled pixels for each label */
  LabelPopulationMapType GetLabelPopulationMap() const
  {
    return this->GetFilter()->GetLabelPopulationMap();
  }

protected:
  /** Constructor */
  StreamingHistogramImageFilter() {}
  /** Destructor */
  ~StreamingHistogramImageFilter() override {}

private:
  StreamingHistogramImageFilter(const Self &) = delete;
  void operator =(const Self&) = delete;
};

} // end namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbStreamingHistogramImageFilter.hxx"
#endif

#endif
