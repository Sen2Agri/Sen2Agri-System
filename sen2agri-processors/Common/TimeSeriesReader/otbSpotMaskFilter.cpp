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
#include "otbSpotMaskFilter.h"

namespace otb
{
/**
 * Constructor.
 */
SpotMaskFilter
::SpotMaskFilter()
{
  this->SetNumberOfRequiredInputs(3);
  this->SetFunctor(SpotMaskFunctor());
}
/**
 * Destructor.
 */
SpotMaskFilter
::~SpotMaskFilter()
{}

void
SpotMaskFilter
::GenerateOutputInformation()
{
  // Call to the superclass implementation
  Superclass::GenerateOutputInformation();

  typename Superclass::OutputImagePointer     outputPtr = this->GetOutput();

  const int nbComponentsPerPixel = 1;

  // initialize the number of channels of the output image
  outputPtr->SetNumberOfComponentsPerPixel(nbComponentsPerPixel);
}

void
SpotMaskFilter
::SetInputValidityMask(const otb::Wrapper::UInt8ImageType *validityMask)
{
    this->SetInput1(validityMask);
}

void
SpotMaskFilter
::SetInputSaturationMask(const otb::Wrapper::UInt8ImageType *saturationMask)
{
    this->SetInput2(saturationMask);
}

void
SpotMaskFilter
::SetInputCloudsMask(const otb::Wrapper::UInt16ImageType *cloudsMask)
{
    this->SetInput3(cloudsMask);
}


/**
 * PrintSelf method.
 */
void
SpotMaskFilter
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // end namespace otb
