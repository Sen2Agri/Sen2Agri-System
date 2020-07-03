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

#include "otbImageList.h"
#include "otbVectorImage.h"

#include "otbStreamingResampleImageFilter.h"

// Transform
#include "itkScalableAffineTransform.h"
//#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

// Filters
#include "otbMultiChannelExtractROI.h"

#include "otbCropTypeFeatureExtractionFilter.h"
#include "otbTemporalResamplingFilter.h"

//#include "otbSpotMaskFilter.h"
//#include "otbSentinelMaskFilter.h"

#include "TimeSeriesReader.h"

#include <string>

typedef otb::TemporalResamplingFilter<ImageType, MaskType, ImageType> TemporalResamplingFilterType;
typedef otb::ObjectList<TemporalResamplingFilterType> TemporalResamplingFilterListType;

class RedEdgeBandsExtractor : public TimeSeriesReader
{
public:
    typedef RedEdgeBandsExtractor Self;
    typedef TimeSeriesReader Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self);
    itkTypeMacro(RedEdgeBandsExtractor, TimeSeriesReader);

    itkSetMacro(UseSwir2Band, bool);
    itkGetMacro(UseSwir2Band, bool);

    RedEdgeBandsExtractor() : m_UseSwir2Band()
    {
        m_TemporalResamplers = TemporalResamplingFilterListType::New();
    }

    void getRedEdgeBands(const std::unique_ptr<MetadataHelper<float, uint8_t>> &pHelper,
                         const TileData &td,
                         ImageDescriptor &descriptor) override
    {
        descriptor.bands.clear();

        std::vector<std::string> redEdgeBands = pHelper->GetRedEdgeBandNames();
        if (redEdgeBands.size() == 0) {
            return;
        }
        if (this->m_UseSwir2Band) {
            const std::string &swir2BandName = pHelper->GetSwir2BandName();
            if (swir2BandName.size() == 0) {
                return;
            }
            redEdgeBands.push_back(swir2BandName);
        } else {
            const std::string &narrowNirBandName = pHelper->GetNarrowNirBandName();
            if (narrowNirBandName.size() == 0) {
                return;
            }
            redEdgeBands.push_back(narrowNirBandName);
        }

        ExtractFloatChannelFilterType::Pointer channelExtractor;

        std::vector<int> relBandsIdxs;
        MetadataHelper<float, uint8_t>::VectorImageType::Pointer img =
            pHelper->GetImage(redEdgeBands, &relBandsIdxs);
        img->UpdateOutputInformation();

        for (int bandIndex : relBandsIdxs) {
            channelExtractor = ExtractFloatChannelFilterType::New();
            channelExtractor->SetInput(img);
            channelExtractor->SetIndex(bandIndex);
            m_Filters->PushBack(channelExtractor);
            auto resampledBand =
                getResampledBand<FloatImageType>(channelExtractor->GetOutput(), td, false);
            descriptor.bands.push_back(resampledBand);
        }
    }

    int getBandCount(const std::string &) override { return 4; }

    otb::Wrapper::FloatVectorImageType *GetOutput(const std::vector<MissionDays> &sensorOutDays)
    {
        auto temporalResampler = TemporalResamplingFilterType::New();

        m_TemporalResamplers->PushBack(temporalResampler);

        otb::SensorDataCollection sdCollection;
        for (const auto &e : sensorOutDays) {
            otb::SensorData sd;

            sd.sensorName = e.mission;
            sd.outDates = e.days;
            sd.bandCount = this->getBandCount(e.mission);

            for (const ImageDescriptor &id : m_Descriptors) {
                if (id.mission == e.mission) {
                    int inDay = getDaysFromEpoch(id.aquisitionDate);
                    sd.inDates.push_back(inDay);
                }
            }

            sdCollection.emplace_back(sd);
        }

        // Set the temporal resampling / gap filling filter
        temporalResampler->SetInputRaster(m_BandsConcat->GetOutput());
        temporalResampler->SetInputMask(m_MaskConcat->GetOutput());
        // The output days will be updated later
        temporalResampler->SetInputData(sdCollection);

        return temporalResampler->GetOutput();
    }

private:
    bool m_UseSwir2Band;

    TemporalResamplingFilterListType::Pointer m_TemporalResamplers;
};

typedef otb::ObjectList<RedEdgeBandsExtractor> RedEdgeBandsExtractorList;
