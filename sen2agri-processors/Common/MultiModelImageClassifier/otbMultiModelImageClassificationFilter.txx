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
#ifndef __otbImageClassificationFilter_txx
#define __otbImageClassificationFilter_txx

#include "otbMultiModelImageClassificationFilter.h"
#include "itkImageRegionIterator.h"
#include "itkProgressReporter.h"

namespace otb
{
/**
 * Constructor
 */
template <class TInputImage, class TOutputImage, class TMaskImage>
MultiModelImageClassificationFilter<TInputImage, TOutputImage, TMaskImage>
::MultiModelImageClassificationFilter()
    : m_UseModelMask()
{
  this->SetNumberOfRequiredInputs(1);
  m_DefaultLabel = itk::NumericTraits<LabelType>::ZeroValue();
}

template <class TInputImage, class TOutputImage, class TMaskImage>
void
MultiModelImageClassificationFilter<TInputImage, TOutputImage, TMaskImage>
::SetModelMask(const MaskImageType * mask)
{
  this->itk::ProcessObject::SetNthInput(0, const_cast<MaskImageType *>(mask));
}

template <class TInputImage, class TOutputImage, class TMaskImage>
const typename MultiModelImageClassificationFilter<TInputImage, TOutputImage, TMaskImage>
::MaskImageType *
MultiModelImageClassificationFilter<TInputImage, TOutputImage, TMaskImage>
::GetModelMask()
{
  if (this->GetNumberOfInputs() < 2)
    {
    return 0;
    }
  return static_cast<const MaskImageType *>(this->itk::ProcessObject::GetInput(0));
}

template <class TInputImage, class TOutputImage, class TMaskImage>
void
MultiModelImageClassificationFilter<TInputImage, TOutputImage, TMaskImage>
::BeforeThreadedGenerateData()
{
  if (!m_Models)
    {
    itkGenericExceptionMacro(<< "No classification models");
    }
}

template <class TInputImage, class TOutputImage, class TMaskImage>
void
MultiModelImageClassificationFilter<TInputImage, TOutputImage, TMaskImage>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType threadId)
{
  // Get the input pointers
//  InputImageConstPointerType inputPtr     = this->GetInput();
  MaskImageConstPointerType  inputMaskPtr;
  if (m_UseModelMask)
    {
      inputMaskPtr = this->GetModelMask();
    }

  OutputImagePointerType     outputPtr    = this->GetOutput();

  // Progress reporting
  itk::ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  // Define iterators
  typedef itk::ImageRegionConstIterator<InputImageType> InputIteratorType;
  typedef itk::ImageRegionConstIterator<MaskImageType>  MaskIteratorType;
  typedef itk::ImageRegionIterator<OutputImageType>     OutputIteratorType;

  typedef std::vector<InputIteratorType> InputIteratorContainerType;

  auto numInputs = this->GetNumberOfIndexedInputs();
  if (m_UseModelMask)
    {
      numInputs--;
    }

  // Iterators declaration
  InputIteratorContainerType inputIts;
  inputIts.reserve(numInputs);
  for (unsigned int i = 0; i < numInputs; ++i)
    {
    int idx;
    if (m_UseModelMask)
      {
        idx = i + 1;
      }
      else
      {
        idx = i;
      }

    InputIteratorType it(const_cast<InputImageType *>(this->GetInput(idx)), outputRegionForThread);
    it.GoToBegin();

    inputIts.push_back(it);
    }

  OutputIteratorType outIt(outputPtr, outputRegionForThread);

  // Eventually iterate on masks
  MaskIteratorType maskIt;
  if (m_UseModelMask)
    {
    maskIt = MaskIteratorType(inputMaskPtr, outputRegionForThread);
    maskIt.GoToBegin();
    }

  // Walk the part of the image
  for (outIt.GoToBegin(); !outIt.IsAtEnd(); ++outIt)
    {
    unsigned char model;
    if (m_UseModelMask)
      {
      model = maskIt.Get();
      model = m_ModelMap[model];

      ++maskIt;
      }
    else
      {
      model = 1;
      }

    if (model > 0)
      {
      // Classify
      outIt.Set(m_Models->GetNthElement(model - 1)->Predict(inputIts[model - 1].Get())[0]);
      }
    else
      {
      // else, set default value
      outIt.Set(m_DefaultLabel);
      }

    for (unsigned int i = 0; i < numInputs; ++i)
      {
      // Increment the input iterators
      ++inputIts[i];
      }

    progress.CompletedPixel();
    }

}
/**
 * PrintSelf Method
 */
template <class TInputImage, class TOutputImage, class TMaskImage>
void
MultiModelImageClassificationFilter<TInputImage, TOutputImage, TMaskImage>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // End namespace otb
#endif
