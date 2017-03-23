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
#include "otbImageListToVectorImageFilter.h"

#include "otbStreamingResampleImageFilter.h"

//Transform
#include "itkScalableAffineTransform.h"
//#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"
#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"

// Filters
#include "otbMultiChannelExtractROI.h"

#include "ComputeNDVIFilter.h"
#include "otbTemporalResamplingFilter.h"

#include "otbSpotMaskFilter.h"
#include "otbSentinelMaskFilter.h"

#include "TimeSeriesReader.h"
#include "CropMaskFeaturesSupervised.hxx"

#include <string>

typedef otb::TemporalResamplingFilter<ImageType, MaskType, ImageType>  TemporalResamplingFilterType;
typedef otb::ObjectList<TemporalResamplingFilterType>       TemporalResamplingFilterListType;

typedef ComputeNDVIFilter<ImageType> ComputeNDVIFilterType;

class CropMaskNDVIPreprocessing : public TimeSeriesReader
{
public:
    typedef CropMaskNDVIPreprocessing Self;
    typedef TimeSeriesReader Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)
    itkTypeMacro(CropMaskNDVIPreprocessing, TimeSeriesReader)

    CropMaskNDVIPreprocessing()
    {
        m_TemporalResampler = TemporalResamplingFilterType::New();
        m_ComputeNDVIFilter = ComputeNDVIFilterType::New();
    }

    otb::Wrapper::FloatVectorImageType * GetOutput(const std::map<std::string, std::vector<int> > &sensorOutDays)
    {
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
        m_TemporalResampler->SetInputRaster(m_BandsConcat->GetOutput());
        m_TemporalResampler->SetInputMask(m_MaskConcat->GetOutput());
        // The output days will be updated later
        m_TemporalResampler->SetInputData(sdCollection);

        m_ComputeNDVIFilter->SetInput(m_TemporalResampler->GetOutput());

        return m_ComputeNDVIFilter->GetOutput();
    }

private:
    TemporalResamplingFilterType::Pointer             m_TemporalResampler;
    ComputeNDVIFilterType::Pointer                    m_ComputeNDVIFilter;
};

typedef otb::ObjectList<CropMaskNDVIPreprocessing>    CropMaskNDVIPreprocessingList;
