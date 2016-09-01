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
#ifndef __otbCropMaskFeatureExtractionFilter_txx
#define __otbCropMaskFeatureExtractionFilter_txx

#include "otbCropMaskFeatureExtractionFilter.h"

namespace otb
{
/**
 * Constructor.
 */
template <class TImage>
CropMaskFeatureExtractionFilter<TImage>
::CropMaskFeatureExtractionFilter()
{
  this->SetNumberOfRequiredInputs(1);
  this->SetFunctor(CropMaskFeatureTimeSeriesFunctor<typename TImage::PixelType>());
}
/**
 * Destructor.
 */
template <class TImage>
CropMaskFeatureExtractionFilter<TImage>
::~CropMaskFeatureExtractionFilter()
{}

template <class TImage>
void
CropMaskFeatureExtractionFilter<TImage>
::GenerateOutputInformation()
{
  // Call to the superclass implementation
  Superclass::GenerateOutputInformation();

  typename Superclass::OutputImagePointer     outputPtr = this->GetOutput();
  typename Superclass::InputImagePointer      inputPtr = this->GetInput(0);

  // The output contains 3 bands for each pixel
  unsigned int nbComponentsPerPixel = (inputPtr->GetNumberOfComponentsPerPixel() / 4 ) * 3;

  // initialize the number of channels of the output image
  outputPtr->SetNumberOfComponentsPerPixel(nbComponentsPerPixel);
}


/**
 * PrintSelf method.
 */
template <class TImage>
void
CropMaskFeatureExtractionFilter<TImage>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // end namespace otb
#endif
