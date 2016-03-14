/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __otbTemporalMergingFilter_txx
#define __otbTemporalMergingFilter_txx

#include "otbTemporalMergingFilter.h"

namespace otb
{
/**
 * Constructor.
 */
template <class TImage>
TemporalMergingFilter<TImage>
::TemporalMergingFilter()
{
  this->SetNumberOfRequiredInputs(2);
  this->m_InputData.empty();
  this->m_OutDays.empty();
}
/**
 * Destructor.
 */
template <class TImage>
TemporalMergingFilter<TImage>
::~TemporalMergingFilter()
{}

template <class TImage>
void
TemporalMergingFilter<TImage>
::GenerateOutputInformation()
{
  // Call to the superclass implementation
  Superclass::GenerateOutputInformation();

  typename Superclass::OutputImagePointer     outputPtr = this->GetOutput();

  // Check if data is available
  if (this->m_InputData.size() == 0)
    {
    itkExceptionMacro(<< "No input data available for the TemporalMergingFunctor functor !");
    }

  std::sort(m_InputData.begin(), m_InputData.end(), TemporalMergingFilter::SortImages);
  this->m_OutDays.clear();

  // count the number of output images and create the out days file
  int lastDay = -1;
  int numOutputImages = 0;
  for (auto& imgInfo : this->m_InputData) {
      if (lastDay != imgInfo.day) {
          numOutputImages++;
          this->m_OutDays.push_back(imgInfo.day);
          lastDay = imgInfo.day;
      }
  }

  // The output contains 4 bands for each output image
  unsigned int nbComponentsPerPixel = 4 * numOutputImages;

  // initialize the number of channels of the output image
  outputPtr->SetNumberOfComponentsPerPixel(nbComponentsPerPixel);
}

template <class TImage>
void
TemporalMergingFilter<TImage>
::BeforeThreadedGenerateData()
{
  Superclass::BeforeThreadedGenerateData();

  // Check if data is available
  if (this->m_InputData.size() == 0)
    {
    itkExceptionMacro(<< "No input data available for the TemporalMergingFunctor functor !");
    }

  // Create the functor
  this->SetFunctor(TemporalMergingFunctor<typename TImage::PixelType>(this->m_InputData));
}


/**
 * PrintSelf method.
 */
template <class TImage>
void
TemporalMergingFilter<TImage>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // end namespace otb
#endif
