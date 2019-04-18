/*=========================================================================
  *
  * Program:      Sen2agri-Processors
  * Language:     C++
  * Copyright:    2015-2016, CS Romania, office@c-s.ro
  * See COPYRIGHT file for details.
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.

 =========================================================================*/

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

#include "../Filters/CropMaskPreprocessing.h"

// Filters
#include "otbMultiChannelExtractROI.h"
#include "otbStreamingResampleImageFilter.h"

#include "../Filters/otbTemporalResamplingFilter.h"

#include "otbSpotMaskFilter.h"
#include "otbSentinelMaskFilter.h"

typedef otb::VectorImage<float, 2>                                 ImageType;
typedef otb::ObjectList<ImageType>                                 ImageListType;

typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;

namespace otb
{
namespace Wrapper
{

class CropMaskTrainImagesClassifier : public Application
{
public:
  /** Standard class typedefs. */
  typedef CropMaskTrainImagesClassifier Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self)

  itkTypeMacro(CropMaskTrainImagesClassifier, otb::Application)

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
  friend void InitSVMParams(CropMaskTrainImagesClassifier & app);

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

//  SpotMaskFilterListType::Pointer                   m_SpotMaskFilters;
//  SentinelMaskFilterListType::Pointer               m_SentinelMaskFilters;
  TemporalResamplingFilterListType::Pointer         m_TempResamplers;
  ImageListType::Pointer                            m_InputImages;
};

}
}


