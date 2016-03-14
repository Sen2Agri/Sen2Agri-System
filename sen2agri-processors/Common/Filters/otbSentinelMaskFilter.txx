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
#ifndef __otbSentinelMaskFilter_txx
#define __otbSentinelMaskFilter_txx

#include "otbSentinelMaskFilter.h"

namespace otb
{
/**
 * Constructor.
 */
template <class TImage>
SentinelMaskFilter<TImage>
::SentinelMaskFilter()
{
  this->SetNumberOfRequiredInputs(1);
  this->SetFunctor(SentinelMaskFunctor<typename TImage::PixelType>());
}
/**
 * Destructor.
 */
template <class TImage>
SentinelMaskFilter<TImage>
::~SentinelMaskFilter()
{}

template <class TImage>
void
SentinelMaskFilter<TImage>
::GenerateOutputInformation()
{
  // Call to the superclass implementation
  Superclass::GenerateOutputInformation();

  typename Superclass::OutputImagePointer     outputPtr = this->GetOutput();

  const int nbComponentsPerPixel = 1;

  // initialize the number of channels of the output image
  outputPtr->SetNumberOfComponentsPerPixel(nbComponentsPerPixel);
}

template <class TImage>
void
SentinelMaskFilter<TImage>
::SetInputQualityMask(const TImage *qualityMask)
{
    this->SetInput1(qualityMask);
}

template <class TImage>
void
SentinelMaskFilter<TImage>
::SetInputCloudsMask(const TImage *cloudsMask)
{
    this->SetInput2(cloudsMask);
}


/**
 * PrintSelf method.
 */
template <class TImage>
void
SentinelMaskFilter<TImage>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // end namespace otb
#endif
