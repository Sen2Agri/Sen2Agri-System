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
template <class TInputImage, class TMask, class TOutputImage>
TemporalResamplingFilter<TInputImage, TMask, TOutputImage>
::TemporalResamplingFilter()
{
  this->SetNumberOfRequiredInputs(2);
}
/**
 * Destructor.
 */
template <class TInputImage, class TMask, class TOutputImage>
TemporalResamplingFilter<TInputImage, TMask, TOutputImage>
::~TemporalResamplingFilter()
{}

template <class TInputImage, class TMask, class TOutputImage>
void
TemporalResamplingFilter<TInputImage, TMask, TOutputImage>
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

  unsigned int nbBands = 0;
  for (const SensorData &sd : this->m_InputData) {
      nbBands += sd.outDates.size() * sd.bandCount;
  }

  // initialize the number of channels of the output image
  outputPtr->SetNumberOfComponentsPerPixel(nbBands);
}

template <class TInputImage, class TMask, class TOutputImage>
void
TemporalResamplingFilter<TInputImage, TMask, TOutputImage>
::BeforeThreadedGenerateData()
{
  Superclass::BeforeThreadedGenerateData();

  // Check if data is available
  if (this->m_InputData.size() == 0)
    {
    itkExceptionMacro(<< "No input data available for the GapFilling functor !");
    }

  // Create the functor
  this->SetFunctor(GapFillingFunctor<typename TInputImage::PixelType, typename TMask::PixelType, typename TOutputImage::PixelType>(this->m_InputData));
}

template <class TInputImage, class TMask, class TOutputImage>
void
TemporalResamplingFilter<TInputImage, TMask, TOutputImage>
::SetInputRaster(const TInputImage *raster)
{
    this->SetInput1(raster);
}

template <class TInputImage, class TMask, class TOutputImage>
void
TemporalResamplingFilter<TInputImage, TMask, TOutputImage>
::SetInputMask(const TMask *mask)
{
    this->SetInput2(mask);
}

} // end namespace otb
#endif
