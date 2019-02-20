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
#include "otbWrapperApplicationFactory.h"
#include "otbWrapperInputImageParameter.h"

#include "otbStatisticsXMLFileReader.h"
#include "../MultiModelImageClassifier/otbMultiModelImageClassificationFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbImageToVectorImageCastFilter.h"
#include "otbWrapperTypes.h"
#include "otbObjectList.h"
#include "otbVectorImage.h"
#include "otbImageList.h"

#include "../Filters/otbStreamingStatisticsMapFromLabelImageFilter.h"

namespace otb
{

namespace Wrapper
{
class ClassStatistics : public Application
{
public:
    typedef ClassStatistics Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self);
    itkTypeMacro(ClassStatistics, otb::Application);

    typedef FloatVectorImageType                                                     FeatureImageType;
    typedef Int32ImageType                                                           ClassImageType;

    typedef otb::StreamingStatisticsMapFromLabelImageFilter<FeatureImageType, ClassImageType> StatisticsFilterType;

private:
    void DoInit() override
    {
        SetName("ClassStatistics");
        SetDescription("Computes per-class statistics for an image.");

        SetDocName("ClassStatistics");
        SetDocLongDescription("Computes per-class statistics for an image.");
        SetDocLimitations("None");
        SetDocAuthors("LN");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Raster);

        AddParameter(ParameterType_InputImage, "in", "Input image");
        SetParameterDescription("in", "The input image.");

        AddParameter(ParameterType_InputImage, "ref", "Support image");
        SetParameterDescription("ref", "Support image for computing statistics.");

        AddParameter(ParameterType_Float, "bv", "Background value");
        SetParameterDescription("bv", "Background value to ignore in computation.");
        MandatoryOff("bv");

        AddParameter(ParameterType_OutputFilename, "outmean", "Class means");
        SetParameterDescription("outmean", "Per-class means output CSV");
        AddParameter(ParameterType_OutputFilename, "outdev", "Class stddev");
        SetParameterDescription("outdev", "Per-class standard deviation output CSV");
        AddParameter(ParameterType_OutputFilename, "outcount", "Class counts");
        SetParameterDescription("outcount", "Per-class pixel count output CSV");

        AddRAMParameter();

        SetDocExampleParameterValue("in", "image.tif");
        SetDocExampleParameterValue("outmean", "mean.csv");
        SetDocExampleParameterValue("outdev", "dev.csv");
        SetDocExampleParameterValue("outcount", "count.csv");
    }

    void DoUpdateParameters() override
    {
    }

    void DoExecute() override
    {
        auto inputImage = GetParameterImage("in");
        auto classImage = GetParameterInt32Image("ref");

        m_StatisticsFilter = StatisticsFilterType::New();
        m_StatisticsFilter->SetInput(inputImage);
        m_StatisticsFilter->SetInputLabelImage(classImage);

        if (HasValue("bv"))
          {
          m_StatisticsFilter->SetUseNoDataValue(true);
          m_StatisticsFilter->SetNoDataValue(GetParameterFloat("bv"));
          }

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

    StatisticsFilterType::Pointer           m_StatisticsFilter;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::ClassStatistics)
