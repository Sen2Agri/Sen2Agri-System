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
#ifndef __otbLandsatMaskFilter_h
#define __otbLandsatMaskFilter_h

#include "otbLandsatMaskFilter.h"

namespace otb
{
/**
 * Constructor.
 */
template <class TImage>
LandsatMaskFilter<TImage>
::LandsatMaskFilter()
{
  this->SetNumberOfRequiredInputs(1);
  this->SetFunctor(LandsatMaskFunctor<typename TImage::PixelType>());
}
/**
 * Destructor.
 */
template <class TImage>
LandsatMaskFilter<TImage>
::~LandsatMaskFilter()
{}

template <class TImage>
void
LandsatMaskFilter<TImage>
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
LandsatMaskFilter<TImage>
::SetInputQualityMask(const TImage *qualityMask)
{
    this->SetInput1(qualityMask);
}

template <class TImage>
void
LandsatMaskFilter<TImage>
::SetInputCloudsMask(const TImage *cloudsMask)
{
    this->SetInput2(cloudsMask);
}


/**
 * PrintSelf method.
 */
template <class TImage>
void
LandsatMaskFilter<TImage>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // end namespace otb
#endif
