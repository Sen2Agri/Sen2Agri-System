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

/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "otbWrapperApplication.h"
#include "otbWrapperInputImageListParameter.h"

#include "../MultiModelImageClassifier/otbMultiModelImageClassificationFilter.h"
#include "otbImageList.h"
#include "otbImageToVectorImageCastFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbObjectList.h"
#include "otbStatisticsXMLFileReader.h"
#include "otbVectorImage.h"
#include "otbWrapperTypes.h"

#include "../Filters/CropTypePreprocessing.h"
#include "../Filters/RedEdgeBandsExtractor.h"
#include "../Filters/otbStreamingStatisticsMapFromLabelImageFilter.h"

namespace otb
{

namespace Wrapper
{
class OpticalFeatures : public Application
{
public:
    typedef OpticalFeatures Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self);

    itkTypeMacro(OpticalFeatures, otb::Application);

    typedef Int16VectorImageType OutputImageType;
    typedef FloatVectorImageType FeatureImageType;
    typedef Int32ImageType ClassImageType;

    typedef otb::StreamingStatisticsMapFromLabelImageFilter<FeatureImageType, ClassImageType>
        StatisticsFilterType;

private:
    void DoInit() override
    {
        SetName("OpticalFeatures");
        SetDescription("Computes optical features statistics for a set of tiles");

        SetDocName("OpticalFeatures");
        SetDocLongDescription("Computes optical features statistics for a set of tiles.");
        SetDocLimitations("None");
        SetDocAuthors("LN");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Vector);

        AddParameter(ParameterType_InputFilenameList, "il", "Input descriptors");
        SetParameterDescription("il", "The list of descriptors. They must be sorted by tiles.");

        AddParameter(ParameterType_InputImage, "ref", "Support image");
        SetParameterDescription("ref", "Support image for computing statistics.");

        AddParameter(ParameterType_InputFilename, "dates", "Sampling dates");
        SetParameterDescription("dates", "The list of days on which to sample the inputs");
        MandatoryOff("dates");

        AddParameter(ParameterType_OutputFilename, "outdates", "Sampling dates");
        SetParameterDescription("outdates", "The list of days on which to sample the inputs");
        MandatoryOff("outdates");

        AddParameter(ParameterType_OutputFilename, "outmean", "Class means");
        SetParameterDescription("outmean", "Per-class means output CSV");
        AddParameter(ParameterType_OutputFilename, "outdev", "Class stddev");
        SetParameterDescription("outdev", "Per-class standard deviation output CSV");
        AddParameter(ParameterType_OutputFilename, "outcount", "Class counts");
        SetParameterDescription("outcount", "Per-class pixel count output CSV");

        AddParameter(ParameterType_StringList, "sp", "Temporal sampling rate");
        MandatoryOff("sp");

        AddParameter(ParameterType_Choice, "mode", "Mode");
        SetParameterDescription("mode", "Specifies the choice of output dates (default: resample)");
        AddChoice("mode.resample", "Specifies the temporal resampling mode");
        AddChoice("mode.gapfill", "Specifies the gapfilling mode");
        AddChoice("mode.gapfillmain", "Specifies the gapfilling mode, but only use non-main series "
                                      "products to fill in the main one");
        SetParameterString("mode", "resample");

        AddParameter(ParameterType_Float, "pixsize", "The size of a pixel, in meters");
        SetDefaultParameterFloat("pixsize", 10.0); // The default value is 10 meters
        SetMinimumParameterFloatValue("pixsize", 1.0);
        MandatoryOff("pixsize");

        AddParameter(ParameterType_String, "mission",
                     "The main raster series that will be used. By default, SENTINEL is used");
        MandatoryOff("mission");

        AddParameter(ParameterType_Empty, "rededge",
                     "Include Sentinel-2 vegetation red edge bands");
        MandatoryOff("rededge");

        AddRAMParameter();

        SetDocExampleParameterValue("il", "image1.xml image2.xml");
        SetDocExampleParameterValue("out", "features.tif");
    }

    void DoUpdateParameters() override {}

    void DoExecute() override
    {
        // Get the list of input files
        const std::vector<std::string> &descriptors = this->GetParameterStringList("il");
        if (descriptors.size() == 0) {
            itkExceptionMacro("No input file set...");
        }

        // get the required pixel size
        auto pixSize = this->GetParameterFloat("pixsize");
        // get the main mission
        std::string mission = SENTINEL;
        if (HasValue("mission")) {
            mission = this->GetParameterString("mission");
        }
        auto redEdge = GetParameterEmpty("rededge");

        TileData td;
        if (redEdge) {
            m_RedEdgeBandsExtractor = RedEdgeBandsExtractor::New();
            m_RedEdgeBandsExtractor->SetPixelSize(pixSize);
            m_RedEdgeBandsExtractor->SetMission(mission);
            m_RedEdgeBandsExtractor->SetUseSwir2Band(true);

            m_RedEdgeBandsExtractor->updateRequiredImageSize(descriptors, 0, descriptors.size(),
                                                             td);
            m_RedEdgeBandsExtractor->Build(descriptors.begin(), descriptors.end(), td);
        } else {
            m_CropTypePreprocessing = CropTypePreprocessing::New();
            m_CropTypePreprocessing->SetPixelSize(pixSize);
            m_CropTypePreprocessing->SetMission(mission);
            m_CropTypePreprocessing->SetUseSwir2Band(true);

            // compute the desired size of the processed rasters
            m_CropTypePreprocessing->updateRequiredImageSize(descriptors, 0, descriptors.size(),
                                                             td);
            m_CropTypePreprocessing->Build(descriptors.begin(), descriptors.end(), td);
        }

        TemporalResamplingMode resamplingMode = TemporalResamplingMode::Resample;
        const auto &modeStr = GetParameterString("mode");
        if (modeStr == "gapfill") {
            resamplingMode = TemporalResamplingMode::GapFill;
        } else if (modeStr == "gapfillmain") {
            resamplingMode = TemporalResamplingMode::GapFillMainMission;
        }

        std::vector<SensorPreferences> sp;
        if (HasValue("sp")) {
            const auto &spValues = GetParameterStringList("sp");
            sp = parseSensorPreferences(spValues);
        } else {
            sp.emplace_back(SensorPreferences{ "SENTINEL", 0, 10 });
            sp.emplace_back(SensorPreferences{ "SPOT", 1, 5 });
            sp.emplace_back(SensorPreferences{ "LANDSAT", 2, 16 });
        }

        FloatVectorImageType *featureImage;
        if (redEdge) {
            auto preprocessors = RedEdgeBandsExtractorList::New();
            preprocessors->PushBack(m_RedEdgeBandsExtractor);

            std::vector<MissionDays> sensorOutDays;
            if (HasValue("dates")) {
                sensorOutDays = readOutputDays(GetParameterString("dates"));
            } else {
                sensorOutDays = getOutputDays(preprocessors, resamplingMode, mission, sp);
            }

            if (HasValue("outdates")) {
                writeOutputDays(sensorOutDays, GetParameterString("outdates"));
            }

            featureImage = m_RedEdgeBandsExtractor->GetOutput(sensorOutDays);
        } else {
            auto preprocessors = CropTypePreprocessingList::New();
            preprocessors->PushBack(m_CropTypePreprocessing);

            std::vector<MissionDays> sensorOutDays;
            if (HasValue("dates")) {
                sensorOutDays = readOutputDays(GetParameterString("dates"));
            } else {
                sensorOutDays = getOutputDays(preprocessors, resamplingMode, mission, sp);
            }

            if (HasValue("outdates")) {
                writeOutputDays(sensorOutDays, GetParameterString("outdates"));
            }

            featureImage = m_CropTypePreprocessing->GetOutput(sensorOutDays);
        }
        featureImage->UpdateOutputInformation();

        auto classImage = GetParameterInt32Image("ref");

        m_StatisticsFilter = StatisticsFilterType::New();
        m_StatisticsFilter->SetInput(featureImage);
        m_StatisticsFilter->SetInputLabelImage(classImage);
        m_StatisticsFilter->SetNoDataValue(-10000);
        m_StatisticsFilter->SetUseNoDataValue(true);

        AddProcess(m_StatisticsFilter->GetStreamer(), "Computing features...");

        m_StatisticsFilter->Update();

        const auto &meanValues = m_StatisticsFilter->GetMeanValueMap();
        const auto &stdDevValues = m_StatisticsFilter->GetStandardDeviationValueMap();
        const auto &countValues = m_StatisticsFilter->GetPixelCountMap();

        const auto &outmean = GetParameterString("outmean");
        const auto &outdev = GetParameterString("outdev");
        const auto &outcount = GetParameterString("outcount");

        std::ofstream fmean(outmean);
        std::ofstream fdev(outdev);
        std::ofstream fcount(outcount);

        for (auto it = meanValues.begin(); it != meanValues.end(); ++it) {
            fmean << it->first;
            for (unsigned int i = 0; i < it->second.Size(); i++)
                fmean << ',' << it->second[i];
            fmean << '\n';
        }
        fmean.close();
        if (!fmean) {
            itkGenericExceptionMacro("Unable to save " + outmean);
        }

        for (auto it = stdDevValues.begin(); it != stdDevValues.end(); ++it) {
            fdev << it->first;
            for (unsigned int i = 0; i < it->second.Size(); i++)
                fdev << ',' << it->second[i];
            fdev << '\n';
        }
        fdev.close();
        if (!fdev) {
            itkGenericExceptionMacro("Unable to save " + outdev);
        }

        for (auto it = countValues.begin(); it != countValues.end(); ++it) {
            fcount << it->first;
            for (unsigned int i = 0; i < it->second.Size(); i++)
                fcount << ',' << it->second[i];
            fcount << '\n';
        }
        fcount.close();
        if (!fcount) {
            itkGenericExceptionMacro("Unable to save " + outcount);
        }
    }

    CropTypePreprocessing::Pointer m_CropTypePreprocessing;
    RedEdgeBandsExtractor::Pointer m_RedEdgeBandsExtractor;
    StatisticsFilterType::Pointer m_StatisticsFilter;
};
} // namespace Wrapper
} // namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::OpticalFeatures)
