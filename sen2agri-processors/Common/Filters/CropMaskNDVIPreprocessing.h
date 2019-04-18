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

// Filters
#include "otbMultiChannelExtractROI.h"

#include "ComputeNDVIFilter.h"
#include "otbTemporalResamplingFilter.h"
#include "otbTemporalMergingFilter.h"
#include "TemporalMerging.hxx"

#include "TimeSeriesReader.h"
#include "CropMaskFeaturesSupervised.hxx"

#include <string>

typedef otb::TemporalResamplingFilter<ImageType, MaskType, ImageType>  TemporalResamplingFilterType;
typedef otb::ObjectList<TemporalResamplingFilterType>       TemporalResamplingFilterListType;

typedef TemporalMergingFunctor<ImageType::PixelType, MaskType::PixelType>       TemporalMergingFunctorType;
typedef otb::BinaryFunctorImageFilterWithNBands<ImageType, MaskType, ImageType,
        TemporalMergingFunctorType>                                             TemporalMergingFilterType;

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

    void GetProductBands(const std::unique_ptr<MetadataHelper<float, uint8_t>>& pHelper, const TileData& td,
                         ImageDescriptor &descriptor) override {
        // Return extract Green, Red, Nir and Swir bands image
        const std::vector<std::string> &bandNames = {pHelper->GetRedBandName(),
                                                pHelper->GetNirBandName()};

        std::vector<int> relBandIdxs;
        MetadataHelper<float>::VectorImageType::Pointer img = pHelper->GetImage(bandNames, &relBandIdxs);
        img->UpdateOutputInformation();

        auto resampledBands = getResampledBand<FloatVectorImageType>(img, td, false);

        for (int bandIndex: relBandIdxs) {
            auto channelExtractor = ExtractFloatChannelFilterType::New();
            channelExtractor->SetInput(resampledBands);
            channelExtractor->SetIndex(bandIndex);
            m_Filters->PushBack(channelExtractor);
            descriptor.bands.push_back(channelExtractor->GetOutput());
        }
    }


    otb::Wrapper::FloatVectorImageType * GetOutput(const std::vector<MissionDays> &sensorOutDays)
    {
#if 0
        std::map<std::string, std::vector<int> > dayMap;
        for (const auto &e : sensorOutDays) {
            dayMap[e.mission] = e.days;
        }

        // Also build the image dates structures
        otb::SensorDataCollection sdCollection;
        std::string lastMission = "";
        for (const ImageDescriptor& id : m_Descriptors) {
            if (id.mission != lastMission) {
                otb::SensorData sd;
                sd.sensorName = id.mission;
                sd.outDates = dayMap.find(id.mission)->second;
                sd.bandCount = this->getBandCount(id.mission);
                sdCollection.push_back(sd);
                lastMission = id.mission;
            }

            auto &sd = sdCollection.back();
            int inDay = getDaysFromEpoch(id.aquisitionDate);

            sd.inDates.push_back(inDay);
        }

        // Set the temporal resampling / gap filling filter
        m_TemporalResampler = TemporalResamplingFilterType::New();
        m_TemporalMerger = TemporalMergingFilterType::New();

        m_TemporalResampler->SetInputRaster(m_BandsConcat->GetOutput());
        m_TemporalResampler->SetInputMask(m_MaskConcat->GetOutput());
        // The output days will be updated later
        m_TemporalResampler->SetInputData(sdCollection);

        std::vector<ImageInfo> imgInfos;
        int priority = 10;
        int index = 0;
        for (const auto &sd : sdCollection) {
            for (auto date : sd.outDates) {
                ImageInfo ii(index++, date, priority);
                imgInfos.push_back(ii);
            }
            priority--;
        }
        std::sort(imgInfos.begin(), imgInfos.end(), [](const ImageInfo& o1, const ImageInfo& o2) {
            return (o1.day < o2.day) || ((o1.day == o2.day) && (o1.priority > o2.priority));
        });

        // count the number of output images and create the out days file
        std::vector<int> od;
        int lastDay = -1;
        for (auto& imgInfo : imgInfos) {
            if (lastDay != imgInfo.day) {
                od.push_back(imgInfo.day);
                lastDay = imgInfo.day;
            }
        }

        m_BandsConcat->UpdateOutputInformation();
        m_MaskConcat->UpdateOutputInformation();

        // The number of image bands can be computed as the ratio between the bands in the image and
        // the bands in the mask
        int imageBands = m_BandsConcat->GetOutput()->GetNumberOfComponentsPerPixel() /
                         m_MaskConcat->GetOutput()->GetNumberOfComponentsPerPixel();


        m_TemporalMerger->SetNumberOfOutputBands(imageBands * od.size());
        m_TemporalMerger->SetFunctor(TemporalMergingFunctor<ImageType::PixelType, MaskType::PixelType>(imgInfos, od.size(), imageBands));

        m_TemporalMerger->SetInput1(m_TemporalResampler->GetOutput());
        m_TemporalMerger->SetInput2(m_MaskConcat->GetOutput());

        m_ComputeNDVIFilter->SetInput(m_TemporalMerger->GetOutput());

        return m_ComputeNDVIFilter->GetOutput();
#else
        std::map<std::string, std::vector<int> > dayMap;
        for (const auto &e : sensorOutDays) {
            dayMap[e.mission] = e.days;
        }

        // Also build the image dates structures
        otb::SensorDataCollection sdCollection;
        std::string lastMission = "";
        for (const ImageDescriptor& id : m_Descriptors) {
            if (id.mission != lastMission) {
                otb::SensorData sd;
                sd.sensorName = id.mission;
                sd.outDates = dayMap.find(id.mission)->second;
                sd.bandCount = 2;
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
#endif
    }

private:
    TemporalResamplingFilterType::Pointer             m_TemporalResampler;
    TemporalMergingFilterType::Pointer                m_TemporalMerger;
    ComputeNDVIFilterType::Pointer                    m_ComputeNDVIFilter;
};

typedef otb::ObjectList<CropMaskNDVIPreprocessing>    CropMaskNDVIPreprocessingList;
