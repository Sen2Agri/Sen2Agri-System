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
#ifndef __otbConcatenateVectorImagesFilter_txx
#define __otbConcatenateVectorImagesFilter_txx

#include "otbConcatenateVectorImagesFilter.h"
#include "itkImageRegionIterator.h"
#include "itkProgressReporter.h"

namespace otb
{
/**
 * Constructor.
 */
template <class TInputImage, class TOutputImage>
ConcatenateVectorImagesFilter<TInputImage, TOutputImage>
::ConcatenateVectorImagesFilter()
{
  this->SetNumberOfRequiredInputs(1);
}
/**
 * Destructor.
 */
template <class TInputImage, class TOutputImage>
ConcatenateVectorImagesFilter<TInputImage, TOutputImage>
::~ConcatenateVectorImagesFilter()
{}

template <class TInputImage, class TOutputImage>
void
ConcatenateVectorImagesFilter<TInputImage, TOutputImage>
::GenerateOutputInformation()
{
  // Call to the superclass implementation
  Superclass::GenerateOutputInformation();

  typename Superclass::OutputImagePointer     outputPtr = this->GetOutput();

  unsigned int nbComponentsPerPixel = 0;
  for (unsigned int i = 0; i < this->GetNumberOfIndexedInputs(); ++i)
    {
    nbComponentsPerPixel += this->GetInput(i)->GetNumberOfComponentsPerPixel();
    }

  // initialize the number of channels of the output image
  outputPtr->SetNumberOfComponentsPerPixel(nbComponentsPerPixel);
}

template <class TInputImage, class TOutputImage>
void
ConcatenateVectorImagesFilter<TInputImage, TOutputImage>
::BeforeThreadedGenerateData()
{
  Superclass::BeforeThreadedGenerateData();

  typename Superclass::InputImageConstPointer inputPtr1 = this->GetInput(0);
  typename Superclass::OutputImagePointer     outputPtr = this->GetOutput();

  for (unsigned int i = 1; i < this->GetNumberOfIndexedInputs(); ++i)
    {
      typename Superclass::InputImageConstPointer inputPtr = this->GetInput(i);
      if (inputPtr1->GetLargestPossibleRegion() != inputPtr->GetLargestPossibleRegion())
        {
        itkExceptionMacro(<< "Input image 1 and input image " << i + 1 << " have different requested regions.");
        }
    }
}

/**
 * Main computation method.
 */
template <class TInputImage, class TOutputImage>
void
ConcatenateVectorImagesFilter<TInputImage, TOutputImage>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                       itk::ThreadIdType threadId)
{
  // retrieves output pointer
  OutputImagePointerType output = this->GetOutput();

  itk::ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  // Define the portion of the input to walk for this thread
  typename InputImageType::RegionType inputRegionForThread;
  this->CallCopyOutputRegionToInputRegion(inputRegionForThread, outputRegionForThread);

  // Iterators typedefs
  typedef itk::ImageRegionIterator<InputImageType>  InputIteratorType;
  typedef itk::ImageRegionIterator<OutputImageType> OutputIteratorType;

  typedef std::vector<InputIteratorType> InputIteratorContainerType;

  // Iterators declaration
  InputIteratorContainerType inputIts;
  std::vector<unsigned int> pixelSizes;
  inputIts.reserve(this->GetNumberOfIndexedInputs());
  pixelSizes.reserve(this->GetNumberOfIndexedInputs());
  for (unsigned int i = 0; i < this->GetNumberOfIndexedInputs(); ++i)
    {
    InputIteratorType it(const_cast<InputImageType *>(this->GetInput(i)), inputRegionForThread);
    it.GoToBegin();
    inputIts.push_back(it);
    pixelSizes.push_back(it.Get().GetSize());
    }

  OutputIteratorType outputIt(output, outputRegionForThread);

  outputIt.GoToBegin();

  typename OutputImageType::PixelType outputPix(outputIt.Get().GetSize());

  // Iterate through the pixels
  while (!outputIt.IsAtEnd())
    {

    unsigned int outputPos = 0;
    for (unsigned int i = 0; i < this->GetNumberOfIndexedInputs(); ++i)
      {
      // Reference to the input pixel
      InputPixelType const& pix = inputIts[i].Get();
      // Check the size of the input pixel
      assert(pixelSizes[i] == pix.GetSize());

      // Loop through each band of the image
      for (unsigned int b = 0; b < pixelSizes[i]; ++b)
        {
        // Fill the output pixel
        outputPix[outputPos++] = static_cast<typename OutputImageType::InternalPixelType>(pix[b]);
        }

      // Increment the input iterator
      ++inputIts[i];
      }
    // Check the size of the input pixel
    assert(outputPos == outputPix.GetSize());

    // Set the output pixel
    outputIt.Set(outputPix);
    // Increment the output iterator
    ++outputIt;
    }

}
/**
 * PrintSelf method.
 */
template <class TInputImage, class TOutputImage>
void
ConcatenateVectorImagesFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // end namespace otb
#endif
