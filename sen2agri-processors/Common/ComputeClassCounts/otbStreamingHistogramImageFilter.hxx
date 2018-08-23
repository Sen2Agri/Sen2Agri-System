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

#ifndef otbStreamingHistogramImageFilter_hxx
#define otbStreamingHistogramImageFilter_hxx
#include "otbStreamingHistogramImageFilter.h"

#include "itkInputDataObjectIterator.h"
#include "itkImageRegionIterator.h"
#include "itkProgressReporter.h"
#include "otbMacro.h"
#include <cmath>

namespace otb
{

template<class TLabelImage>
PersistentStreamingHistogramImageFilter<TLabelImage>
::PersistentStreamingHistogramImageFilter()
{
  // first output is a copy of the image, DataObject created by
  // superclass
  //

  this->Reset();
}

template<class TLabelImage>
typename PersistentStreamingHistogramImageFilter<TLabelImage>::LabelPopulationMapType
PersistentStreamingHistogramImageFilter<TLabelImage>
::GetLabelPopulationMap() const
{
  return m_LabelPopulation;
}

template<class TLabelImage>
void
PersistentStreamingHistogramImageFilter<TLabelImage>
::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();

  if (this->GetInput())
    {
    this->GetOutput()->CopyInformation(this->GetInput());
    this->GetOutput()->SetLargestPossibleRegion(this->GetInput()->GetLargestPossibleRegion());

    if (this->GetOutput()->GetRequestedRegion().GetNumberOfPixels() == 0)
      {
      this->GetOutput()->SetRequestedRegion(this->GetOutput()->GetLargestPossibleRegion());
      }
    }
}

template<class TLabelImage>
void
PersistentStreamingHistogramImageFilter<TLabelImage>
::AllocateOutputs()
{
  // This is commented to prevent the streaming of the whole image for the first stream strip
  // It shall not cause any problem because the output image of this filter is not intended to be used.
  //InputImagePointer image = const_cast< TInputImage * >( this->GetInput() );
  //this->GraftOutput( image );
  // Nothing that needs to be allocated for the remaining outputs
}

template<class TLabelImage>
void
PersistentStreamingHistogramImageFilter<TLabelImage>
::Synthetize()
 {
  // Update temporary accumulator
  AccumulatorMapType outputAcc;

  for (auto const& threadAccMap: m_AccumulatorMaps)
    {
    for(auto const& it: threadAccMap)
      {
      const LabelPixelType label = it.first;
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

  // Publish output map
  for(auto& it: outputAcc)
    {
    const LabelPixelType label = it.first;
    const double count = it.second.GetCount();

    // Count
    m_LabelPopulation[label] = count;
    }

 }

template<class TLabelImage>
void
PersistentStreamingHistogramImageFilter<TLabelImage>
::Reset()
{
  m_AccumulatorMaps.clear();

  m_LabelPopulation.clear();
  m_AccumulatorMaps.resize(this->GetNumberOfThreads());

}

template<class TLabelImage>
void
PersistentStreamingHistogramImageFilter<TLabelImage>
::GenerateInputRequestedRegion()
{
  // The Requested Regions of all the inputs are set to their Largest Possible Regions
  this->itk::ProcessObject::GenerateInputRequestedRegion();

  // Iteration over all the inputs of the current filter (this)
  for( itk::InputDataObjectIterator it( this ); !it.IsAtEnd(); it++ )
    {
    // Check whether the input is an image of the appropriate dimension
    // dynamic_cast of all the input images as itk::ImageBase objects
    // in order to pass the if ( input ) test whatever the inputImageType (vectorImage or labelImage)
    ImageBaseType * input = dynamic_cast< ImageBaseType *>( it.GetInput() );

    if ( input )
      {
      // Use the function object RegionCopier to copy the output region
      // to the input.  The default region copier has default implementations
      // to handle the cases where the input and output are the same
      // dimension, the input a higher dimension than the output, and the
      // input a lower dimension than the output.
      InputImageRegionType inputRegion;
      this->CallCopyOutputRegionToInputRegion( inputRegion, this->GetOutput()->GetRequestedRegion() );
      input->SetRequestedRegion(inputRegion);
      }
    }
}

template<class TLabelImage>
void
PersistentStreamingHistogramImageFilter<TLabelImage>
::ThreadedGenerateData(const RegionType& outputRegionForThread, itk::ThreadIdType threadId )
{
  /**
   * Grab the input
   */
  LabelImagePointer labelInputPtr =  const_cast<TLabelImage *>(this->GetInput());

  itk::ImageRegionConstIterator<TLabelImage> labelIt(labelInputPtr, outputRegionForThread);

  typename LabelImageType::PixelType label;

  // do the work
  for (labelIt.GoToBegin();
       !labelIt.IsAtEnd();
       ++labelIt)
    {
      label = labelIt.Get();

      // Update the accumulator
      if (m_AccumulatorMaps[threadId].count(label) <= 0) //add new element to the map
        {
        AccumulatorType newAcc;
        m_AccumulatorMaps[threadId][label] = newAcc;
        }
      else
        {
        m_AccumulatorMaps[threadId][label].Update();
        }
    }
}

template<class TLabelImage>
void
PersistentStreamingHistogramImageFilter<TLabelImage>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

} // end namespace otb
#endif
