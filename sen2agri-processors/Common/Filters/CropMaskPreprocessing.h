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
 
#pragma once

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbWrapperApplicationRegistry.h"
#include "otbWrapperInputImageListParameter.h"

#include "otbVectorImage.h"
#include "otbImageList.h"

#include "otbStreamingResampleImageFilter.h"
#include "otbGridResampleImageFilter.h"

//Transform
#include "itkScalableAffineTransform.h"
//#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"
#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"

// Filters
#include "otbMultiChannelExtractROI.h"
#include "otbConcatenateVectorImagesFilter.h"
#include "../Filters/otbCropTypeFeatureExtractionFilter.h"
#include "../Filters/otbTemporalResamplingFilter.h"
#include "../Filters/otbTemporalMergingFilter.h"

#include "../Filters/otbSpotMaskFilter.h"
#include "../Filters/otbLandsatMaskFilter.h"
#include "../Filters/otbSentinelMaskFilter.h"

#include "../Filters/TimeSeriesReader.h"

#include <string>

class CropMaskPreprocessing : public TimeSeriesReader
{
public:
    typedef CropMaskPreprocessing Self;
    typedef TimeSeriesReader Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)
    itkTypeMacro(CropMaskPreprocessing, TimeSeriesReader)

    otb::Wrapper::FloatVectorImageType * GetOutput()
    {
        auto bands = GetBands();

        // Set the temporal resampling / gap filling filter
        m_TemporalResampler->SetInputRaster(std::get<0>(bands));
        m_TemporalResampler->SetInputMask(std::get<1>(bands));
        // The output days will be updated later
        m_TemporalResampler->SetInputData(std::get<2>(bands));

        // Set the feature extractor
        m_FeatureExtractor->SetInput(m_TemporalResampler->GetOutput());

        return m_FeatureExtractor->GetOutput();
    }
};

typedef otb::ObjectList<CropMaskPreprocessing>              CropMaskPreprocessingList;
