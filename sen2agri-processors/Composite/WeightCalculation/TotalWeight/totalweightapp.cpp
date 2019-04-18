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

#include "totalweightcomputation.h"
#include "MetadataHelperFactory.h"

namespace otb
{

namespace Wrapper
{

class TotalWeight : public Application
{

public:
  /** Standard class typedefs. */
  typedef TotalWeight                      Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(TotalWeight, otb::Application);

private:
  void DoInit()
  {
    SetName("TotalWeight");
    SetDescription("Computes the cloud weight value for the given mask cloud and parameters");

    SetDocName("Total Weight Computation");
    SetDocLongDescription("This application computes the total weight for the Composite Processor");

    SetDocLimitations("None");
    SetDocAuthors("CIU");
    SetDocSeeAlso(" ");
    AddDocTag("Util");

    AddParameter(ParameterType_String,  "xml",   "The input metadata xml");
    SetParameterDescription("xml", "The XML file containing the L2A metadata.");

    AddParameter(ParameterType_String,  "waotfile",   "Input AOT weight file name");
    SetParameterDescription("waotfile", "The file name of the image containing the AOT weigth for each pixel.");

    AddParameter(ParameterType_String,  "wcldfile",   "Input cloud weight file name");
    SetParameterDescription("wcldfile", "The file name of the image containing the cloud weigth for each pixel.");

    AddParameter(ParameterType_String, "l3adate", "L3A date, expressed in days");
    SetParameterDescription("l3adate", "The L3A date extracted from metadata, expressed in days.");

    AddParameter(ParameterType_Int, "halfsynthesis", "Delta max");
    SetParameterDescription("halfsynthesis", "Half synthesis period expressed in days.");

    AddParameter(ParameterType_Float, "wdatemin", "Minimum date weight");
    SetParameterDescription("wdatemin", "Minimum weight at edge of synthesis time window.");
    SetDefaultParameterFloat("wdatemin", 0.5);
    MandatoryOff("wdatemin");

    AddParameter(ParameterType_OutputImage, "out", "Output Total Weight Image");
    SetParameterDescription("out","The output image containg the computed total weight for each pixel.");

    // Doc example parameter settings
    SetDocExampleParameterValue("xml", "example1.xml");
    SetDocExampleParameterValue("waotfile", "example2.tif");
    SetDocExampleParameterValue("wcldfile", "example3.tif");
    SetDocExampleParameterValue("l3adate", "20140502");
    SetDocExampleParameterValue("halfsynthesis", "50");
    SetDocExampleParameterValue("wdatemin", "0.10");
    SetDocExampleParameterValue("out", "apTotalWeightOutput.tif");
  }

  void DoUpdateParameters()
  {
  }

  void DoExecute()
  {
    // Get the input image list
    std::string inXml = GetParameterString("xml");
    if (inXml.empty())
    {
        itkExceptionMacro("No xml file given...; please set the input xml");
    }

    auto factory = MetadataHelperFactory::New();
    auto pHelper = factory->GetMetadataHelper<short>(inXml);
    std::string l2aDate = pHelper->GetAcquisitionDate();
    std::string l3aDate = GetParameterString("l3adate");
    int halfSynthesis = GetParameterInt("halfsynthesis");
    float weightOnDateMin = GetParameterFloat("wdatemin");

    std::string inAotFileName = GetParameterString("waotfile");
    std::string inCloudFileName = GetParameterString("wcldfile");

    // weight on sensor parameters
    std::string missionName = pHelper->GetMissionName();
    m_totalWeightComputation.SetMissionName(missionName);

    // weight on date parameters
    m_totalWeightComputation.SetDates(l2aDate, l3aDate);
    m_totalWeightComputation.SetHalfSynthesisPeriodAsDays(halfSynthesis);
    m_totalWeightComputation.SetWeightOnDateMin(weightOnDateMin);


    // Weights for AOT and Clouds
    m_totalWeightComputation.SetAotWeightFile(inAotFileName);
    m_totalWeightComputation.SetCloudsWeightFile(inCloudFileName);

    // Set the output image
    SetParameterOutputImage("out", m_totalWeightComputation.GetOutputImageSource()->GetOutput());
  }

  TotalWeightComputation m_totalWeightComputation;
};

} // namespace Wrapper
} // namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::TotalWeight)

