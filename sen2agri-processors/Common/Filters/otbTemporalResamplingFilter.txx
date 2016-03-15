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
#ifndef __otbTemporalResamplingFilter_txx
#define __otbTemporalResamplingFilter_txx

#include "otbTemporalResamplingFilter.h"

namespace otb
{
/**
 * Constructor.
 */
template <class TImage>
TemporalResamplingFilter<TImage>
::TemporalResamplingFilter()
{
  this->SetNumberOfRequiredInputs(2);
}
/**
 * Destructor.
 */
template <class TImage>
TemporalResamplingFilter<TImage>
::~TemporalResamplingFilter()
{}

template <class TImage>
void
TemporalResamplingFilter<TImage>
::GenerateOutputInformation()
{
  // Call to the superclass implementation
  Superclass::GenerateOutputInformation();

  typename Superclass::OutputImagePointer     outputPtr = this->GetOutput();

  // Check if data is available
  if (this->m_InputData.size() == 0)
    {
    itkExceptionMacro(<< "No input data available for the GapFilling functor !");
    }

  // The output contains 4 bands for each output image
  unsigned int nbComponentsPerPixel = 0;
  for (const SensorData &sd : this->m_InputData) {
      nbComponentsPerPixel += sd.outDates.size();
  }

  // initialize the number of channels of the output image
  outputPtr->SetNumberOfComponentsPerPixel(nbComponentsPerPixel);
}

template <class TImage>
void
TemporalResamplingFilter<TImage>
::BeforeThreadedGenerateData()
{
  Superclass::BeforeThreadedGenerateData();

  // Check if data is available
  if (this->m_InputData.size() == 0)
    {
    itkExceptionMacro(<< "No input data available for the GapFilling functor !");
    }

  // Create the functor
  this->SetFunctor(GapFillingFunctor<typename TImage::PixelType>(this->m_InputData));
}

template <class TImage>
void
TemporalResamplingFilter<TImage>
::SetInputRaster(const TImage *raster)
{
    this->SetInput1(raster);
}

template <class TImage>
void
TemporalResamplingFilter<TImage>
::SetInputMask(const TImage *mask)
{
    this->SetInput2(mask);
}



/**
 * PrintSelf method.
 */
template <class TImage>
void
TemporalResamplingFilter<TImage>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // end namespace otb
#endif
