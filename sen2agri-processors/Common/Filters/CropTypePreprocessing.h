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

    itkSetMacro(IncludeRedEdge, bool)
    itkGetMacro(IncludeRedEdge, bool)

    CropTypePreprocessing()
        : m_IncludeRedEdge()
    {
        m_TemporalResamplers = TemporalResamplingFilterListType::New();
        m_FeatureExtractors = CropTypeFeatureExtractionFilterListType::New();
    }

    void getSentinelRedEdgeBands(const MACCSFileMetadata &meta,
                          const TileData &td,
                          ImageDescriptor &descriptor,
                          Int16ImageReaderType::Pointer,
                          Int16ImageReaderType::Pointer reader2) override
    {
        if (!m_IncludeRedEdge) {
            return;
        }

//        auto resampledBands = getResampledBand2<otb::Wrapper::Int16VectorImageType>(reader2->GetOutput(), td, false);
        ExtractChannelFilterType::Pointer channelExtractor;

        int b5Index = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B5");
        channelExtractor = ExtractChannelFilterType::New();
        channelExtractor->SetInput(reader2->GetOutput());
//        channelExtractor->SetInput(resampledBands);
        channelExtractor->SetIndex(b5Index - 1);
        m_Filters->PushBack(channelExtractor);
        auto b5Band = getResampledBand<FloatImageType>(channelExtractor->GetOutput(), td, false);
//        auto b5Band = channelExtractor->GetOutput();

        int b6Index = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B6");
        channelExtractor = ExtractChannelFilterType::New();
        channelExtractor->SetInput(reader2->GetOutput());
//        channelExtractor->SetInput(resampledBands);
        channelExtractor->SetIndex(b6Index - 1);
        m_Filters->PushBack(channelExtractor);
        auto b6Band = getResampledBand<FloatImageType>(channelExtractor->GetOutput(), td, false);
//        auto b6Band = channelExtractor->GetOutput();

        int b7Index = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B7");
        channelExtractor = ExtractChannelFilterType::New();
        channelExtractor->SetInput(reader2->GetOutput());
//        channelExtractor->SetInput(resampledBands);
        channelExtractor->SetIndex(b7Index - 1);
        m_Filters->PushBack(channelExtractor);
        auto b7Band = getResampledBand<FloatImageType>(channelExtractor->GetOutput(), td, false);
//        auto b7Band = channelExtractor->GetOutput();

        int b8aIndex = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B8A");
        channelExtractor = ExtractChannelFilterType::New();
        channelExtractor->SetInput(reader2->GetOutput());
//        channelExtractor->SetInput(resampledBands);
        channelExtractor->SetIndex(b8aIndex - 1);
        m_Filters->PushBack(channelExtractor);
        auto b8aBand = getResampledBand<FloatImageType>(channelExtractor->GetOutput(), td, false);
//        auto b8aBand = channelExtractor->GetOutput();

        descriptor.bands.push_back(b5Band);
        descriptor.bands.push_back(b6Band);
        descriptor.bands.push_back(b7Band);
        descriptor.bands.push_back(b8aBand);
    }

    int getBandCount(const std::string &sensor) override {
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


    otb::Wrapper::FloatVectorImageType * GetOutput(const std::vector<MissionDays> &sensorOutDays)
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

            for (const ImageDescriptor& id : m_Descriptors) {
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

    TemporalResamplingFilterListType::Pointer         m_TemporalResamplers;
    CropTypeFeatureExtractionFilterListType::Pointer  m_FeatureExtractors;
};

typedef otb::ObjectList<CropTypePreprocessing>        CropTypePreprocessingList;
