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

#include "CropMaskFeaturesSupervised.hxx"
#include "otbTemporalResamplingFilter.h"
#include "otbTemporalMergingFilter.h"

#include "otbSpotMaskFilter.h"
#include "otbLandsatMaskFilter.h"
#include "otbSentinelMaskFilter.h"

#include "TimeSeriesReader.h"
#include "TemporalMerging.hxx"

#include <string>

typedef CropMaskFeaturesSupervisedFunctor<ImageType::PixelType> FeaturesFunctorType;
typedef CropMaskFeaturesSupervisedBMFunctor<ImageType::PixelType> FeaturesBMFunctorType;

typedef UnaryFunctorImageFilterWithNBands<FeaturesFunctorType> UnaryFunctorImageFilterWithNBandsType;
typedef UnaryFunctorImageFilterWithNBands<FeaturesBMFunctorType> UnaryFunctorImageFilterBMWithNBandsType;


class CropMaskPreprocessing : public TimeSeriesReader
{
public:
    typedef CropMaskPreprocessing Self;
    typedef TimeSeriesReader Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)
    itkTypeMacro(CropMaskPreprocessing, TimeSeriesReader)

    itkSetMacro(W, int)
    itkGetMacro(W, int)

    itkSetMacro(Delta, float)
    itkGetMacro(Delta, float)

    itkSetMacro(TSoil, float)
    itkGetMacro(TSoil, float)

    itkSetMacro(BM, bool)
    itkGetMacro(BM, bool)

    CropMaskPreprocessing()
    {
        m_TemporalResampler = TemporalResamplingFilterType::New();
        m_FeatureExtractor = UnaryFunctorImageFilterWithNBandsType::New();
        m_FeatureExtractorBM = UnaryFunctorImageFilterBMWithNBandsType::New();
        m_Merger = otb::BinaryFunctorImageFilterWithNBands<ImageType, TemporalMergingFunctor<ImageType::PixelType>>::New();
    }

    otb::Wrapper::FloatVectorImageType * GetOutput()
    {
        // Merge the rasters and the masks
        ConcatenateVectorImagesFilterType::Pointer bandsConcat = ConcatenateVectorImagesFilterType::New();
        ConcatenateVectorImagesFilterType::Pointer maskConcat = ConcatenateVectorImagesFilterType::New();
        // Also build the image dates structures
        otb::SensorDataCollection sdCollection;

        int index = 0;
        std::string lastMission = "";
        for (const ImageDescriptor& id : m_Descriptors) {
            if (id.mission != lastMission) {
                otb::SensorData sd;
                sd.sensorName = id.mission;
                sd.outDates = m_SensorOutDays[id.mission];
                sdCollection.push_back(sd);
                lastMission = id.mission;
            }

            auto &sd = sdCollection.back();
            int inDay = getDaysFromEpoch(id.aquisitionDate);

            sd.inDates.push_back(inDay);

            bandsConcat->PushBackInput(id.bands);
            maskConcat->PushBackInput(id.mask);
            index++;
        }
        m_ImageMergers->PushBack(bandsConcat);
        m_ImageMergers->PushBack(maskConcat);

        // Set the temporal resampling / gap filling filter
        m_TemporalResampler->SetInputRaster(bandsConcat->GetOutput());
        m_TemporalResampler->SetInputMask(maskConcat->GetOutput());
        // The output days will be updated later
        m_TemporalResampler->SetInputData(sdCollection);

        std::vector<ImageInfo> imgInfos;
        int priority = 10;
        index = 0;
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
        std::cerr << "dates:\n";
        for (auto& imgInfo : imgInfos) {
            if (lastDay != imgInfo.day) {
                std::cerr << imgInfo.day << std::endl;
                od.push_back(imgInfo.day);
                lastDay = imgInfo.day;
            }
        }

        bandsConcat->UpdateOutputInformation();
        maskConcat->UpdateOutputInformation();
        // The number of image bands can be computed as the ratio between the bands in the image and
        // the bands in the mask
        int imageBands = bandsConcat->GetOutput()->GetNumberOfComponentsPerPixel() /
                         maskConcat->GetOutput()->GetNumberOfComponentsPerPixel();


        m_Merger->SetNumberOfOutputBands(imageBands * od.size());
        m_Merger->SetFunctor(TemporalMergingFunctor<ImageType::PixelType>(imgInfos, od.size(), imageBands));

        m_Merger->SetInput(0, m_TemporalResampler->GetOutput());
        m_Merger->SetInput(1, maskConcat->GetOutput());

        if (m_BM) {
            m_FeatureExtractorBM->GetFunctor().m_W = m_W;
            m_FeatureExtractorBM->GetFunctor().m_Delta = m_Delta;
            m_FeatureExtractorBM->GetFunctor().m_TSoil = m_TSoil;
            m_FeatureExtractorBM->GetFunctor().id = od;
            m_FeatureExtractorBM->SetNumberOfOutputBands(26);

            m_FeatureExtractorBM->SetInput(m_Merger->GetOutput());

            return m_FeatureExtractorBM->GetOutput();
        } else {
            m_FeatureExtractor->GetFunctor().m_W = m_W;
            m_FeatureExtractor->GetFunctor().m_Delta = m_Delta;
            m_FeatureExtractor->GetFunctor().m_TSoil = m_TSoil;
            m_FeatureExtractor->GetFunctor().id = od;
            m_FeatureExtractor->SetNumberOfOutputBands(27);

            m_FeatureExtractor->SetInput(m_Merger->GetOutput());

            return m_FeatureExtractor->GetOutput();
        }
    }

private:
    TemporalResamplingFilterType::Pointer             m_TemporalResampler;
    UnaryFunctorImageFilterWithNBandsType::Pointer    m_FeatureExtractor;
    UnaryFunctorImageFilterBMWithNBandsType::Pointer  m_FeatureExtractorBM;
    otb::BinaryFunctorImageFilterWithNBands<ImageType,
        TemporalMergingFunctor<ImageType::PixelType>>
    ::Pointer                                         m_Merger;

    int m_W;
    PixelValueType m_Delta;
    PixelValueType m_TSoil;
    bool m_BM;
};

typedef otb::ObjectList<CropMaskPreprocessing>              CropMaskPreprocessingList;
