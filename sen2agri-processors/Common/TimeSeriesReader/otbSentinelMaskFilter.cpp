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
#include "otbSentinelMaskFilter.h"

namespace otb
{
/**
 * Constructor.
 */
SentinelMaskFilter
::SentinelMaskFilter()
{
  this->SetNumberOfRequiredInputs(1);
  this->SetFunctor(SentinelMaskFunctor());
}
/**
 * Destructor.
 */
SentinelMaskFilter
::~SentinelMaskFilter()
{}

void
SentinelMaskFilter
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
SentinelMaskFilter
::SetInputQualityMask(const otb::Wrapper::UInt8VectorImageType *qualityMask)
{
    this->SetInput1(qualityMask);
}

void
SentinelMaskFilter
::SetInputCloudsMask(const otb::Wrapper::UInt8ImageType  *cloudsMask)
{
    this->SetInput2(cloudsMask);
}


/**
 * PrintSelf method.
 */
void
SentinelMaskFilter
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // end namespace otb
