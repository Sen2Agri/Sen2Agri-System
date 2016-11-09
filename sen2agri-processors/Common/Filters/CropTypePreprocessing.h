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

//Transform
#include "itkScalableAffineTransform.h"
//#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"
#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"

// Filters
#include "otbMultiChannelExtractROI.h"

#include "otbCropTypeFeatureExtractionFilter.h"
#include "otbTemporalResamplingFilter.h"
#include "otbTemporalMergingFilter.h"

#include "otbSpotMaskFilter.h"
#include "otbSentinelMaskFilter.h"

#include "TimeSeriesReader.h"

#include <string>

typedef otb::TemporalResamplingFilter<ImageType, MaskType, ImageType>  TemporalResamplingFilterType;
typedef otb::ObjectList<TemporalResamplingFilterType>       TemporalResamplingFilterListType;

typedef otb::CropTypeFeatureExtractionFilter<ImageType>     CropTypeFeatureExtractionFilterType;
typedef otb::ObjectList<CropTypeFeatureExtractionFilterType>
                                                            CropTypeFeatureExtractionFilterListType;
class CropTypePreprocessing : public TimeSeriesReader
{
public:
    typedef CropTypePreprocessing Self;
    typedef TimeSeriesReader Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)
    itkTypeMacro(CropTypePreprocessing, TimeSeriesReader)

    CropTypePreprocessing()
    {
        m_TemporalResamplers = TemporalResamplingFilterListType::New();
        m_FeatureExtractors = CropTypeFeatureExtractionFilterListType::New();
    }

    otb::Wrapper::FloatVectorImageType * GetOutput(const std::map<std::string, std::vector<int> > &sensorOutDays)
    {
        auto temporalResampler = TemporalResamplingFilterType::New();
        auto featureExtractor = CropTypeFeatureExtractionFilterType::New();

        m_TemporalResamplers->PushBack(temporalResampler);
        m_FeatureExtractors->PushBack(featureExtractor);

        // Also build the image dates structures
        otb::SensorDataCollection sdCollection;
        std::string lastMission = "";
        for (const ImageDescriptor& id : m_Descriptors) {
            if (id.mission != lastMission) {
                otb::SensorData sd;
                sd.sensorName = id.mission;
                sd.outDates = sensorOutDays.find(id.mission)->second;
                sd.bandCount = id.bands.size();
                sdCollection.push_back(sd);
                lastMission = id.mission;
            }

            auto &sd = sdCollection.back();
            int inDay = getDaysFromEpoch(id.aquisitionDate);

            sd.inDates.push_back(inDay);
        }

        // Set the temporal resampling / gap filling filter
        temporalResampler->SetInputRaster(m_BandsConcat->GetOutput());
        temporalResampler->SetInputMask(m_MaskConcat->GetOutput());
        // The output days will be updated later
        temporalResampler->SetInputData(sdCollection);

        featureExtractor->SetInput(temporalResampler->GetOutput());
        featureExtractor->SetSensorData(sdCollection);

        return featureExtractor->GetOutput();
    }


private:
    TemporalResamplingFilterListType::Pointer         m_TemporalResamplers;
    CropTypeFeatureExtractionFilterListType::Pointer  m_FeatureExtractors;
};

typedef otb::ObjectList<CropTypePreprocessing>        CropTypePreprocessingList;
