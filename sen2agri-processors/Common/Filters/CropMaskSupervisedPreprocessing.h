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

class CropTypePreprocessing : public TimeSeriesReader
{
public:
    typedef CropTypePreprocessing Self;
    typedef TimeSeriesReader Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)
    itkTypeMacro(CropTypePreprocessing, TimeSeriesReader)

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

        // Set the feature extractor
        m_FeatureExtractor->SetInput(m_TemporalResampler->GetOutput());

        return m_FeatureExtractor->GetOutput();
    }

protected:

};

typedef otb::ObjectList<CropTypePreprocessing>              CropTypePreprocessingList;

std::map<std::string, std::vector<int>> getOutputDays(CropTypePreprocessingList::Pointer preprocessors, bool resample, const std::map<std::string, int> &sp)
{
    std::map<std::string, std::set<int> >    sensorInDays;
    std::map<std::string, std::vector<int> > sensorOutDays;

    for (unsigned int i = 0; i < preprocessors->Size(); i++) {
        for (const auto &id : preprocessors->GetNthElement(i)->GetDescriptorList()) {
            auto inDay = getDaysFromEpoch(id.aquisitionDate);
            sensorInDays[id.mission].emplace(inDay);
        }
    }

    // loop through the sensors to determinte the output dates
    for (const auto& sensor : sensorInDays) {
        std::vector<int> outDates;
        if (resample) {
            auto it = sp.find(sensor.first);
            if (it == sp.end()) {
                itkGenericExceptionMacro("Sampling rate required for sensor " << sensor.first);
            }
            auto rate = it->second;

            auto last = *sensor.second.rbegin();
            for (int date = *sensor.second.begin(); date <= last; date += rate) {
                outDates.emplace_back(date);
            }
        } else {
            outDates.insert(outDates.end(), sensor.second.begin(), sensor.second.end());
        }
        sensorOutDays[sensor.first] = outDates;
    }

    return sensorOutDays;
}
