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

typedef otb::CropTypeFeatureExtractionFilter<ImageType> CropTypeFeatureExtractionFilterType;
typedef otb::ObjectList<CropTypeFeatureExtractionFilterType>
    CropTypeFeatureExtractionFilterListType;
class CropTypePreprocessing : public TimeSeriesReader
{
public:
    typedef CropTypePreprocessing Self;
    typedef TimeSeriesReader Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self);
    itkTypeMacro(CropTypePreprocessing, TimeSeriesReader);

    itkSetMacro(IncludeRedEdge, bool);
    itkGetMacro(IncludeRedEdge, bool);
    itkGetMacro(UseSwir2Band, bool);

    CropTypePreprocessing() : m_IncludeRedEdge(), m_UseSwir2Band()
    {
        m_TemporalResamplers = TemporalResamplingFilterListType::New();
        m_FeatureExtractors = CropTypeFeatureExtractionFilterListType::New();
    }

    void getRedEdgeBands(const std::unique_ptr<MetadataHelper<float, uint8_t>> &pHelper,
                         const TileData &td,
                         ImageDescriptor &descriptor) override
    {
        if (!m_IncludeRedEdge) {
            return;
        }
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

    int getBandCount(const std::string &sensor) override
    {
        if (sensor == "SENTINEL") {
            if (m_IncludeRedEdge) {
                return 8;
            } else {
                return 4;
            }
        } else if (sensor == "SPOT" || sensor == "LANDSAT") {
            return 4;
        }
        itkGenericExceptionMacro("Unknown sensor " << sensor);
    }

    otb::Wrapper::FloatVectorImageType *GetOutput(const std::vector<MissionDays> &sensorOutDays)
    {
        auto temporalResampler = TemporalResamplingFilterType::New();
        auto featureExtractor = CropTypeFeatureExtractionFilterType::New();

        m_TemporalResamplers->PushBack(temporalResampler);
        m_FeatureExtractors->PushBack(featureExtractor);

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

        featureExtractor->SetInput(temporalResampler->GetOutput());
        featureExtractor->SetSensorData(sdCollection);

        return featureExtractor->GetOutput();
    }

private:
    bool m_IncludeRedEdge;

    TemporalResamplingFilterListType::Pointer m_TemporalResamplers;
    CropTypeFeatureExtractionFilterListType::Pointer m_FeatureExtractors;
};

typedef otb::ObjectList<CropTypePreprocessing> CropTypePreprocessingList;
