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
#include "otbTemporalMergingFilter.h"
#include "TemporalMerging.hxx"

#include "otbSpotMaskFilter.h"
#include "otbSentinelMaskFilter.h"

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

    void getSpotBands(const SPOT4Metadata& meta, const boost::filesystem::path &rootFolder, const TileData& td, ImageDescriptor &descriptor) override {
        const auto &imageFile = rootFolder / meta.Files.OrthoSurfCorrPente;
        auto reader = getFloatVectorImageReader(imageFile.string());
        reader->GetOutput()->UpdateOutputInformation();

        auto resampledBands = getResampledBand<FloatVectorImageType>(reader->GetOutput(), td, false);

        auto channelExtractor1 = ExtractFloatChannelFilterType::New();
        channelExtractor1->SetInput(resampledBands);
        channelExtractor1->SetIndex(1);
        m_Filters->PushBack(channelExtractor1);

        auto channelExtractor2 = ExtractFloatChannelFilterType::New();
        channelExtractor2->SetInput(resampledBands);
        channelExtractor2->SetIndex(2);
        m_Filters->PushBack(channelExtractor2);

        descriptor.bands.push_back(channelExtractor1->GetOutput());
        descriptor.bands.push_back(channelExtractor2->GetOutput());
    }

    void getLandsatBands(const MACCSFileMetadata& meta, const boost::filesystem::path &rootFolder, const TileData& td, ImageDescriptor &descriptor) override {
        std::string imageFile = getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE");
        auto reader = getFloatVectorImageReader(imageFile);
        reader->GetOutput()->UpdateOutputInformation();

        auto resampledBands = getResampledBand<FloatVectorImageType>(reader->GetOutput(), td, false);

        auto channelExtractor1 = ExtractFloatChannelFilterType::New();
        channelExtractor1->SetInput(resampledBands);
        channelExtractor1->SetIndex(3);
        m_Filters->PushBack(channelExtractor1);

        auto channelExtractor2 = ExtractFloatChannelFilterType::New();
        channelExtractor2->SetInput(resampledBands);
        channelExtractor2->SetIndex(4);
        m_Filters->PushBack(channelExtractor2);

        descriptor.bands.push_back(channelExtractor1->GetOutput());
        descriptor.bands.push_back(channelExtractor2->GetOutput());
    }

    void getSentinelBands(const MACCSFileMetadata &meta,
                          const boost::filesystem::path &rootFolder,
                          const TileData &td,
                          ImageDescriptor &descriptor) override
    {
        std::string imageFile =
            getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE_R1");
        auto reader = getInt16ImageReader(imageFile);
        reader->GetOutput()->UpdateOutputInformation();

        auto channelExtractor1 = ExtractChannelFilterType::New();
        channelExtractor1->SetInput(reader->GetOutput());
        channelExtractor1->SetIndex(2);
        m_ChannelExtractors->PushBack(channelExtractor1);

        auto channelExtractor2 = ExtractChannelFilterType::New();
        channelExtractor2->SetInput(reader->GetOutput());
        channelExtractor2->SetIndex(3);
        m_ChannelExtractors->PushBack(channelExtractor2);

        // Return the concatenation result
        auto b4Band = getResampledBand<FloatImageType>(channelExtractor1->GetOutput(), td, false);
        auto b8Band = getResampledBand<FloatImageType>(channelExtractor2->GetOutput(), td, false);

        descriptor.bands.push_back(b4Band);
        descriptor.bands.push_back(b8Band);
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
