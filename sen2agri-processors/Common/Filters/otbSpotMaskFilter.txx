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
#ifndef __otbSpotMaskFilter_h
#define __otbSpotMaskFilter_h

#include "otbSpotMaskFilter.h"

namespace otb
{
/**
 * Constructor.
 */
template <class TImage>
SpotMaskFilter<TImage>
::SpotMaskFilter()
{
  this->SetNumberOfRequiredInputs(1);
  this->SetFunctor(SpotMaskFunctor<typename TImage::PixelType>());
}
/**
 * Destructor.
 */
template <class TImage>
SpotMaskFilter<TImage>
::~SpotMaskFilter()
{}

template <class TImage>
void
SpotMaskFilter<TImage>
::GenerateOutputInformation()
{
  // Call to the superclass implementation
  Superclass::GenerateOutputInformation();

  typename Superclass::OutputImagePointer     outputPtr = this->GetOutput();

  // initialize the number of channels of the output image
  outputPtr->SetNumberOfComponentsPerPixel(nbComponentsPerPixel);
}

template <class TImage>
void
SpotMaskFilter<TImage>
::SetInputValidityMask(const TImage *validityMask)
{
    this->SetInput1(validityMask);
}

template <class TImage>
void
SpotMaskFilter<TImage>
::SetInputSaturationMask(const TImage *saturationMask)
{
    this->SetInput2(saturationMask);
}

template <class TImage>
void
SpotMaskFilter<TImage>
::SetInputCloudsMask(const TImage *cloudsMask)
{
    this->SetInput3(cloudsMask);
}


/**
 * PrintSelf method.
 */
template <class TImage>
void
SpotMaskFilter<TImage>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // end namespace otb
#endif
