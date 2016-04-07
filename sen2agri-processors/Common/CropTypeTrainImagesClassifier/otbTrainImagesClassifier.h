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
#include "otbConfigure.h"

#include "otbWrapperApplicationFactory.h"

#include <iostream>

//Image
#include "otbVectorImage.h"
#include "otbVectorData.h"

#include "otbImageFileReader.h"
#include "otbWrapperTypes.h"
#include "otbWrapperElevationParametersHandler.h"

#include "otbVectorDataProjectionFilter.h"
#include "otbVectorDataIntoImageProjectionFilter.h"

#include "../Filters/CropTypePreprocessing.h"

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"
#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"

// Filters
#include "otbMultiChannelExtractROI.h"
#include "otbStreamingResampleImageFilter.h"

#include "otbConcatenateVectorImagesFilter.h"
#include "../Filters/otbCropTypeFeatureExtractionFilter.h"
#include "../Filters/otbTemporalResamplingFilter.h"
#include "../Filters/otbTemporalMergingFilter.h"

#include "../Filters/otbSpotMaskFilter.h"
#include "../Filters/otbLandsatMaskFilter.h"
#include "../Filters/otbSentinelMaskFilter.h"

typedef otb::VectorImage<float, 2>                                 ImageType;
typedef otb::ObjectList<ImageType>                                 ImageListType;

typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;

typedef itk::MACCSMetadataReader                                   MACCSMetadataReaderType;
typedef itk::SPOT4MetadataReader                                   SPOT4MetadataReaderType;

namespace otb
{
namespace Wrapper
{

class CropTypeTrainImagesClassifier : public Application
{
public:
  /** Standard class typedefs. */
  typedef CropTypeTrainImagesClassifier Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self)

  itkTypeMacro(CropTypeTrainImagesClassifier, otb::Application)

  typedef FloatVectorImageType::PixelType         PixelType;
  typedef FloatVectorImageType::InternalPixelType InternalPixelType;

  // Training vectordata
  typedef itk::VariableLengthVector<InternalPixelType> MeasurementType;

  // VectorData projection filter
  typedef otb::VectorDataProjectionFilter<VectorDataType, VectorDataType> VectorDataProjectionFilterType;

  // Extract ROI
  typedef otb::VectorDataIntoImageProjectionFilter<VectorDataType, FloatVectorImageType> VectorDataReprojectionType;

protected:
  using Superclass::AddParameter;
  friend void InitSVMParams(CropTypeTrainImagesClassifier & app);

private:
  void DoInit();

  void DoUpdateParameters();

#ifdef OTB_USE_LIBSVM
  void InitLibSVMParams();
#endif
  
#ifdef OTB_USE_OPENCV
  void InitBoostParams();
  void InitSVMParams();
  void InitDecisionTreeParams();
  void InitGradientBoostedTreeParams();
  void InitNeuralNetworkParams();
  void InitNormalBayesParams();
  void InitRandomForestsParams();
  void InitKNNParams();
#endif

  void DoExecute();

  VectorDataReprojectionType::Pointer vdreproj;
  //  Software Guide :EndCodeSnippet

  std::string                           m_mission;
  float                                 m_pixSize;

  ImageReaderListType::Pointer          m_ImageReaderList;
  ResampleFilterListType::Pointer       m_ResamplersList;

  SpotMaskFilterListType::Pointer                   m_SpotMaskFilters;
  LandsatMaskFilterListType::Pointer                m_LandsatMaskFilters;
  SentinelMaskFilterListType::Pointer               m_SentinelMaskFilters;
  MultiChannelExtractROIListType::Pointer           m_ChannelExtractors;
  ConcatenateVectorImagesFilterListType::Pointer    m_ImageMergers;
  TemporalResamplingFilterListType::Pointer         m_TempResamplers;
  CropTypeFeatureExtractionFilterListType::Pointer  m_FeatureExtrators;
  ImageListType::Pointer                            m_InputImages;
};

}
}


