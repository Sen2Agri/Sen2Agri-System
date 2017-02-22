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
#ifndef __otbListSampleGeneratorRaster_txx
#define __otbListSampleGeneratorRaster_txx

#include "otbListSampleGeneratorRaster.h"

#include "itkImageRegionConstIteratorWithIndex.h"
#include "otbRemoteSensingRegion.h"
#include "otbNumberOfLinesStrippedStreamingManager.h"

#include "otbMacro.h"

namespace otb
{

template<class TImage, class TRaster>
ListSampleGeneratorRaster<TImage, TRaster>
::ListSampleGeneratorRaster() :
  m_MaxTrainingSize(-1),
  m_MaxValidationSize(-1),
  m_ValidationTrainingProportion(0.0),
  m_BoundByMin(true),
  m_NoDataLabel(static_cast<RasterPixelType>(0)),
  m_NumberOfClasses(0),
  m_ClassMinSize(-1)
{
  this->SetNumberOfRequiredInputs(2);
  this->SetNumberOfRequiredOutputs(4);

  // Register the outputs
  this->itk::ProcessObject::SetNthOutput(0, this->MakeOutput(0).GetPointer());
  this->itk::ProcessObject::SetNthOutput(1, this->MakeOutput(1).GetPointer());
  this->itk::ProcessObject::SetNthOutput(2, this->MakeOutput(2).GetPointer());
  this->itk::ProcessObject::SetNthOutput(3, this->MakeOutput(3).GetPointer());

  m_RandomGenerator = RandomGeneratorType::GetInstance();

  typedef NumberOfLinesStrippedStreamingManager<TImage> NumberOfLinesStrippedStreamingManagerType;
  typename NumberOfLinesStrippedStreamingManagerType::Pointer streamingManager = NumberOfLinesStrippedStreamingManagerType::New();
  streamingManager->SetNumberOfLinesPerStrip(10);

  m_StreamingManager = streamingManager;
}


template<class TImage, class TRaster>
void
ListSampleGeneratorRaster<TImage, TRaster>
::GenerateInputRequestedRegion(void)

{
  ImagePointerType img = static_cast<ImageType *>(this->ProcessObject::GetInput(0));

  if(img.IsNotNull())
    {

    // Requested regions will be generated during GenerateData
    // call. For now request an empty region so as to avoid requesting
    // the largest possible region (fixes bug #943 )
    typename ImageType::RegionType dummyRegion;
    typename ImageType::SizeType dummySize;
    dummySize.Fill(0);
    dummyRegion.SetSize(dummySize);
    img->SetRequestedRegion(dummyRegion);
    }
}

template <class TImage, class TRaster>
void
ListSampleGeneratorRaster<TImage, TRaster>
::SetInput(const ImageType * image)
{
  this->ProcessObject::SetNthInput(0, const_cast<ImageType *>(image));
}

template <class TImage, class TRaster>
const TImage *
ListSampleGeneratorRaster<TImage, TRaster>
::GetInput() const
{
  if (this->GetNumberOfInputs() < 1)
    {
    return 0;
    }

  return static_cast<const ImageType *>(this->ProcessObject::GetInput(0));
}

template <class TImage, class TRaster>
void
ListSampleGeneratorRaster<TImage, TRaster>
::SetInputRaster(const RasterType * raster)
{
  this->ProcessObject::SetNthInput(1, const_cast<RasterType *>(raster));
}

template <class TImage, class TRaster>
const TRaster *
ListSampleGeneratorRaster<TImage, TRaster>
::GetInputRaster() const
{
  if (this->GetNumberOfInputs() < 2)
    {
    return 0;
    }

  return static_cast<const RasterType *>(this->ProcessObject::GetInput(1));
}

template <class TImage, class TRaster>
typename ListSampleGeneratorRaster<TImage, TRaster>::DataObjectPointer
ListSampleGeneratorRaster<TImage, TRaster>
::MakeOutput(DataObjectPointerArraySizeType idx)
{
  DataObjectPointer output;
  switch (idx)
    {
    case 0:
      output = static_cast<itk::DataObject*>(ListSampleType::New().GetPointer());
      break;
    case 1:
      output = static_cast<itk::DataObject*>(ListLabelType::New().GetPointer());
      break;
    case 2:
      output = static_cast<itk::DataObject*>(ListSampleType::New().GetPointer());
      break;
    case 3:
      output = static_cast<itk::DataObject*>(ListLabelType::New().GetPointer());
      break;
    default:
      output = static_cast<itk::DataObject*>(ListSampleType::New().GetPointer());
      break;
    }
  return output;
}
// Get the Training ListSample
template <class TImage, class TRaster>
typename ListSampleGeneratorRaster<TImage, TRaster>::ListSampleType*
ListSampleGeneratorRaster<TImage, TRaster>
::GetTrainingListSample()
{
  return dynamic_cast<ListSampleType*>(this->itk::ProcessObject::GetOutput(0));
}
// Get the Training label ListSample
template <class TImage, class TRaster>
typename ListSampleGeneratorRaster<TImage, TRaster>::ListLabelType*
ListSampleGeneratorRaster<TImage, TRaster>
::GetTrainingListLabel()
{
  return dynamic_cast<ListLabelType*>(this->itk::ProcessObject::GetOutput(1));
}

// Get the validation ListSample
template <class TImage, class TRaster>
typename ListSampleGeneratorRaster<TImage, TRaster>::ListSampleType*
ListSampleGeneratorRaster<TImage, TRaster>
::GetValidationListSample()
{
  return dynamic_cast<ListSampleType*>(this->itk::ProcessObject::GetOutput(2));
}


// Get the validation label ListSample
template <class TImage, class TRaster>
typename ListSampleGeneratorRaster<TImage, TRaster>::ListLabelType*
ListSampleGeneratorRaster<TImage, TRaster>
::GetValidationListLabel()
{
  return dynamic_cast<ListLabelType*>(this->itk::ProcessObject::GetOutput(3));
}

template <class TImage, class TRaster>
void
ListSampleGeneratorRaster<TImage, TRaster>
::GenerateData()
{
  // Get the inputs
  ImagePointerType image = const_cast<ImageType*>(this->GetInput());
  RasterPointerType raster = const_cast<RasterType*>(this->GetInputRaster());

  // Get the outputs
  ListSamplePointerType trainingListSample   = this->GetTrainingListSample();
  ListLabelPointerType  trainingListLabel    = this->GetTrainingListLabel();
  ListSamplePointerType validationListSample = this->GetValidationListSample();
  ListLabelPointerType  validationListLabel  = this->GetValidationListLabel();

  // Gather some information about the relative size of the classes
  // We would like to have the same number of samples per class
//  this->GenerateClassStatistics();

  this->ComputeClassSelectionProbability();

  // Clear the sample lists
  trainingListSample->Clear();
  trainingListLabel->Clear();
  validationListSample->Clear();
  validationListLabel->Clear();

  // Set MeasurementVectorSize for each sample list
  trainingListSample->SetMeasurementVectorSize(image->GetNumberOfComponentsPerPixel());
  // stores label as integers,so put the size to 1
  trainingListLabel->SetMeasurementVectorSize(1);
  validationListSample->SetMeasurementVectorSize(image->GetNumberOfComponentsPerPixel());
  // stores label as integers,so put the size to 1
  validationListLabel->SetMeasurementVectorSize(1);

  m_ClassesSamplesNumberTraining.clear();
  m_ClassesSamplesNumberValidation.clear();

  /**
   * Determine of number of pieces to divide the input.  This will be the
   * minimum of what the user specified via SetNumberOfDivisionsStrippedStreaming()
   * and what the Splitter thinks is a reasonable value.
   */
  m_StreamingManager->PrepareStreaming(image, image->GetLargestPossibleRegion());
  const unsigned int numberOfDivisions = m_StreamingManager->GetNumberOfSplits();


  /**
   * Loop over the number of pieces, execute the upstream pipeline on each
   * piece, and copy the results into the output image.
   */
  typedef itk::ImageRegionConstIteratorWithIndex<ImageType> IteratorType;
  ImageRegionType streamRegion;
  for (unsigned int currentDivision = 0; currentDivision < numberOfDivisions ; currentDivision++)
    {
    streamRegion = m_StreamingManager->GetSplit(currentDivision);
    otbMsgDevMacro(<< "Processing region : " << streamRegion );

    image->SetRequestedRegion(streamRegion);
    image->PropagateRequestedRegion();
    image->UpdateOutputData();
    IteratorType it(image, streamRegion);

    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
      {
      double randomValue = m_RandomGenerator->GetUniformVariate(0.0, 1.0);
      RasterPixelType rasterClass = raster->GetPixel(it.GetIndex());
      if (rasterClass == m_NoDataLabel)
        {// ignore pixel
          continue;
        }
      if (randomValue < m_ClassesProbTraining[rasterClass])
        {
        //Add the sample to the training list
        trainingListSample->PushBack(it.Get());
        trainingListLabel->PushBack(rasterClass);
        m_ClassesSamplesNumberTraining[rasterClass] += 1;
        }
      else if (randomValue < m_ClassesProbTraining[rasterClass]
               + m_ClassesProbValidation[rasterClass])
        {
        //Add the sample to the validation list
        validationListSample->PushBack(it.Get());
        validationListLabel->PushBack(rasterClass);
        m_ClassesSamplesNumberValidation[rasterClass] += 1;
        }
      //Note: some samples may not be used at all
      }
    }


  assert(trainingListSample->Size() == trainingListLabel->Size());
  assert(validationListSample->Size() == validationListLabel->Size());
}

template <class TImage, class TRaster>
void
ListSampleGeneratorRaster<TImage, TRaster>
::GenerateClassStatistics()
{
  m_ClassesSize.clear();

  typename RasterType::ConstPointer raster = this->GetInputRaster();

  // Compute the number of pixels for each class
  typedef itk::ImageRegionConstIteratorWithIndex<RasterType> RasterIteratorType;
  RasterIteratorType it(raster, raster->GetLargestPossibleRegion());

  for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
      if (it.Get() != m_NoDataLabel) {
           m_ClassesSize[it.Get()]++;
      }
  }
  m_NumberOfClasses = m_ClassesSize.size();
}

template <class TImage, class TRaster>
void
ListSampleGeneratorRaster<TImage, TRaster>
::ComputeClassSelectionProbability()
{
  m_ClassesProbTraining.clear();
  m_ClassesProbValidation.clear();

  // Sanity check
  if (m_ClassesSize.empty())
    {
    itkGenericExceptionMacro(<< "No training sample found inside image");
    }

  // Go through the classes size to find the smallest one
  int minSize = itk::NumericTraits<int>::max();
  for (ClassesSizeType::const_iterator itmap = m_ClassesSize.begin();
        itmap != m_ClassesSize.end();
        ++itmap)
    {
    if (minSize > itmap->second)
      {
      minSize = itmap->second;
      }
    }

  // Apply the proportion between training and validation samples (all training by default)
  double minSizeTraining   = minSize * (1.0 - m_ValidationTrainingProportion);
  double minSizeValidation = minSize * m_ValidationTrainingProportion;

  // Apply the limit if specified by the user
  if(m_BoundByMin)
    {
    if ((m_MaxTrainingSize != -1) && (m_MaxTrainingSize < minSizeTraining))
      {
      minSizeTraining = m_MaxTrainingSize;
      }
    if ((m_MaxValidationSize != -1) && (m_MaxValidationSize < minSizeValidation))
      {
      minSizeValidation = m_MaxValidationSize;
      }
    }
  // Compute the probability selection for each class
  for (ClassesSizeType::const_iterator itmap = m_ClassesSize.begin();
       itmap != m_ClassesSize.end();
       ++itmap)
    {
    double count = static_cast<double>(itmap->second);
    m_ClassesProbTraining[itmap->first] = minSizeTraining / count;
    m_ClassesProbValidation[itmap->first] = minSizeValidation / count;
    if(!m_BoundByMin)
      {
      long int maxSizeT = count*(1.0 - m_ValidationTrainingProportion);
      long int maxSizeV = count*m_ValidationTrainingProportion;
      maxSizeT = (m_MaxTrainingSize == -1)?maxSizeT:m_MaxTrainingSize;
      maxSizeV = (m_MaxValidationSize == -1)?maxSizeV:m_MaxValidationSize;
    
      //not enough samples to respect the bounds
      if(maxSizeT+maxSizeV > itmap->second)
        {
        maxSizeT = count*(1.0 - m_ValidationTrainingProportion);
        maxSizeV = count*m_ValidationTrainingProportion;
        }
      m_ClassesProbTraining[itmap->first] = maxSizeT/count;
      m_ClassesProbValidation[itmap->first] = maxSizeV/count;
      }
    }
}

template <class TImage, class TRaster>
void
ListSampleGeneratorRaster<TImage, TRaster>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << "* MaxTrainingSize: " << m_MaxTrainingSize << "\n";
  os << indent << "* MaxValidationSize: " << m_MaxValidationSize << "\n";
  os << indent << "* Proportion: " << m_ValidationTrainingProportion << "\n";
  os << indent << "* Input data:\n";
  if (m_ClassesSize.empty())
    {
    os << indent << "Empty\n";
    }
  else
    {
    for (ClassesSizeType::const_iterator itmap = m_ClassesSize.begin();
         itmap != m_ClassesSize.end(); ++itmap)
      {
      os << indent << itmap->first << ": " << itmap->second << "\n";
      }
    }

  os << "\n" << indent << "* Training set:\n";
  if (m_ClassesProbTraining.empty())
    {
    os << indent << "Not computed\n";
    }
  else
    {
    os << indent << "** Selection probability:\n";
    for (std::map<ClassLabelType, double>::const_iterator itmap = m_ClassesProbTraining.begin();
         itmap != m_ClassesProbTraining.end(); ++itmap)
      {
      os << indent << itmap->first << ": " << itmap->second << "\n";
      }
    os << indent << "** Number of selected samples:\n";
    for (std::map<ClassLabelType, int>::const_iterator itmap = m_ClassesSamplesNumberTraining.begin();
         itmap != m_ClassesSamplesNumberTraining.end(); ++itmap)
      {
      os << indent << itmap->first << ": " << itmap->second << "\n";
      }
    }

  os << "\n" << indent << "* Validation set:\n";
  if (m_ClassesProbValidation.empty())
    {
    os << indent << "Not computed\n";
    }
  else
    {
    os << indent << "** Selection probability:\n";
    for (std::map<ClassLabelType, double>::const_iterator itmap = m_ClassesProbValidation.begin();
         itmap != m_ClassesProbValidation.end(); ++itmap)
      {
      os << indent << itmap->first << ": " << itmap->second << "\n";
      }
    os << indent << "** Number of selected samples:\n";
    for (std::map<ClassLabelType, int>::const_iterator itmap = m_ClassesSamplesNumberValidation.begin();
         itmap != m_ClassesSamplesNumberValidation.end(); ++itmap)
      {
      os << indent << itmap->first << ": " << itmap->second << "\n";
      }
    }
}

}

#endif
