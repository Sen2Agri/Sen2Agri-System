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
#include "otbConcatenateVectorImagesFilter.h"

#include "CropMaskFeaturesSupervised.hxx"
#include "CropMaskSupervisedRedEdgeFeaturesFilter.h"
#include "otbTemporalResamplingFilter.h"
#include "otbTemporalMergingFilter.h"

#include "otbSpotMaskFilter.h"
#include "otbSentinelMaskFilter.h"

#include "TimeSeriesReader.h"
#include "TemporalMerging.hxx"

#include <string>

typedef CropMaskFeaturesSupervisedFunctor<ImageType::PixelType>     FeaturesFunctorType;
typedef CropMaskFeaturesSupervisedBMFunctor<ImageType::PixelType>   FeaturesBMFunctorType;

typedef UnaryFunctorImageFilterWithNBands<FeaturesFunctorType>      FeatureExtractorFilterType;
typedef UnaryFunctorImageFilterWithNBands<FeaturesBMFunctorType>    FeatureExtractorBMFilterType;

typedef otb::ObjectList<FeatureExtractorFilterType>                 FeatureExtractorFilterListType;
typedef otb::ObjectList<FeatureExtractorBMFilterType>               FeatureExtractorBMFilterListType;

typedef otb::TemporalResamplingFilter<ImageType, MaskType, ImageType>  TemporalResamplingFilterType;
typedef otb::ObjectList<TemporalResamplingFilterType>                  TemporalResamplingFilterListType;

typedef TemporalMergingFunctor<ImageType::PixelType, MaskType::PixelType>       TemporalMergingFunctorType;
typedef otb::BinaryFunctorImageFilterWithNBands<ImageType, MaskType, ImageType,
        TemporalMergingFunctorType>                                             TemporalMergingFilterType;
typedef otb::ObjectList<TemporalMergingFilterType>                              TemporalMergingFilterListType;

typedef CropMaskSupervisedRedEdgeFeaturesFilter<ImageType>          RedEdgeFeaturesFilterType;
typedef otb::ConcatenateVectorImagesFilter<ImageType>               ConcatenateImagesFilterType;

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
        m_TemporalResamplers = TemporalResamplingFilterListType::New();
        m_TemporalMergers = TemporalMergingFilterListType::New();
        m_FeatureExtractors = FeatureExtractorFilterListType::New();
        m_FeatureExtractorsBM = FeatureExtractorBMFilterListType::New();
    }

    otb::Wrapper::FloatVectorImageType * GetOutput(const std::vector<MissionDays> &sensorOutDays)
    {
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
        auto temporalResampler = TemporalResamplingFilterType::New();
        auto temporalMerger = TemporalMergingFilterType::New();

        m_TemporalResamplers->PushBack(temporalResampler);
        m_TemporalMergers->PushBack(temporalMerger);

        temporalResampler->SetInputRaster(m_BandsConcat->GetOutput());
        temporalResampler->SetInputMask(m_MaskConcat->GetOutput());
        // The output days will be updated later
        temporalResampler->SetInputData(sdCollection);

        if (m_IncludeRedEdge) {
            m_RedEdgeBandConcat->UpdateOutputInformation();
            m_RedEdgeMaskConcat->UpdateOutputInformation();

            auto redEdgeTemporalResampler = TemporalResamplingFilterType::New();
            m_TemporalResamplers->PushBack(redEdgeTemporalResampler);
            redEdgeTemporalResampler->SetInputRaster(m_RedEdgeBandConcat->GetOutput());
            redEdgeTemporalResampler->SetInputMask(m_RedEdgeMaskConcat->GetOutput());

            otb::SensorDataCollection redEdgeSdCollection;
            for (const auto &e : sensorOutDays) {
                if (e.mission == SENTINEL) {
                    otb::SensorData sd;

                    sd.sensorName = e.mission;
                    sd.outDates = e.days;
                    sd.bandCount = 6;

                    for (const ImageDescriptor& id : m_Descriptors) {
                        if (id.mission == e.mission) {
                            int inDay = getDaysFromEpoch(id.aquisitionDate);
                            sd.inDates.push_back(inDay);
                        }
                    }

                    redEdgeSdCollection.emplace_back(sd);
                }
            }

            redEdgeTemporalResampler->SetInputData(redEdgeSdCollection);

            m_RedEdgeFeaturesFilter = RedEdgeFeaturesFilterType::New();
            m_RedEdgeFeaturesFilter->SetInput(redEdgeTemporalResampler->GetOutput());
        }

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


        temporalMerger->SetNumberOfOutputBands(imageBands * od.size());
        temporalMerger->SetFunctor(TemporalMergingFunctor<ImageType::PixelType, MaskType::PixelType>(imgInfos, od.size(), imageBands));

        temporalMerger->SetInput1(temporalResampler->GetOutput());
        temporalMerger->SetInput2(m_MaskConcat->GetOutput());

        otb::Wrapper::FloatVectorImageType *output;
        if (m_BM) {
            auto featureExtractorBM = FeatureExtractorBMFilterType::New();
            m_FeatureExtractorsBM->PushBack(featureExtractorBM);

            featureExtractorBM->GetFunctor().m_W = m_W;
            featureExtractorBM->GetFunctor().m_Delta = m_Delta;
            featureExtractorBM->GetFunctor().m_TSoil = m_TSoil;
            featureExtractorBM->GetFunctor().id = od;
            featureExtractorBM->SetNumberOfOutputBands(26);

            featureExtractorBM->SetInput(temporalMerger->GetOutput());

            output = featureExtractorBM->GetOutput();
        } else {
            auto featureExtractor = FeatureExtractorFilterType::New();
            m_FeatureExtractors->PushBack(featureExtractor);

            featureExtractor->GetFunctor().m_W = m_W;
            featureExtractor->GetFunctor().m_Delta = m_Delta;
            featureExtractor->GetFunctor().m_TSoil = m_TSoil;
            featureExtractor->GetFunctor().id = od;
            featureExtractor->SetNumberOfOutputBands(27);

            featureExtractor->SetInput(temporalMerger->GetOutput());

            output = featureExtractor->GetOutput();
        }

        if (!m_IncludeRedEdge) {
            return output;
        } else {
            m_ConcatenateImagesFilter = ConcatenateImagesFilterType::New();
            m_ConcatenateImagesFilter->PushBackInput(output);
            m_ConcatenateImagesFilter->PushBackInput(m_RedEdgeFeaturesFilter->GetOutput());

            output = m_ConcatenateImagesFilter->GetOutput();
        }

        return output;
    }

private:
    TemporalResamplingFilterListType::Pointer         m_TemporalResamplers;
    TemporalMergingFilterListType::Pointer            m_TemporalMergers;
    FeatureExtractorFilterListType::Pointer           m_FeatureExtractors;
    FeatureExtractorBMFilterListType::Pointer         m_FeatureExtractorsBM;
    RedEdgeFeaturesFilterType::Pointer                m_RedEdgeFeaturesFilter;
    ConcatenateImagesFilterType::Pointer              m_ConcatenateImagesFilter;
    int m_W;
    PixelValueType m_Delta;
    PixelValueType m_TSoil;
    bool m_BM;
};

typedef otb::ObjectList<CropMaskPreprocessing>        CropMaskPreprocessingList;
