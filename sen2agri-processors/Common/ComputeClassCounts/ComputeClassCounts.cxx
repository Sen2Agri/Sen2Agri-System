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

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include "itkFlatStructuringElement.h"
#include "itkLabelErodeImageFilter.h"
//#include "otbStreamingHistogramImageFilter.h"

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

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include "itkFlatStructuringElement.h"

#include "itkBinaryDilateImageFilter.h"
#include "itkLabelErodeImageFilter.h"
#include "itkBinaryMorphologicalOpeningImageFilter.h"
#include "itkBinaryMorphologicalClosingImageFilter.h"
#include "itkBinaryMorphologyImageFilter.h"
#include "itkLabelErodeImageFilter.h"

#include "otbMultiToMonoChannelExtractROI.h"
#include "otbImageList.h"
#include "otbImageListToVectorImageFilter.h"

#include "otbStreamingHistogramImageFilter.h"

namespace otb
{
namespace Wrapper
{

class ComputeClassCounts : public Application
{
public:
/** Standard class typedefs. */
typedef ComputeClassCounts            Self;
typedef Application                   Superclass;
typedef itk::SmartPointer<Self>       Pointer;
typedef itk::SmartPointer<const Self> ConstPointer;


typedef itk::FlatStructuringElement<2>                                         StructuringType;
typedef StructuringType::RadiusType                                            RadiusType;

typedef Int32ImageType                                                         ImageType;
typedef itk::LabelErodeImageFilter<ImageType, ImageType, StructuringType>      ErodeFilterType;
typedef otb::StreamingHistogramImageFilter<ImageType>                          StreamingHistogramFilterType;

/** Standard macro */
itkNewMacro(Self)
itkTypeMacro(ComputeClassCounts, otb::Application)

private:

void DoInit() override
{
  SetName( "ComputeClassCounts" );
  SetDescription( "Applies a 3x3 erosion filter and computes the class counts on a labelled image" );

  AddParameter( ParameterType_InputImage , "in" ,  "Input Image" );
  SetParameterDescription( "in" , "The input image to be filtered." );

  AddParameter( ParameterType_OutputFilename, "out" ,  "Output File" );
  SetParameterDescription( "out" , "Output CSV file with class counts." );

  AddParameter( ParameterType_Int , "bv" , "Background value" );
  SetParameterDescription("bv", "Set the background value, default is 0." );
  SetDefaultParameterInt("bv", 0);
  MandatoryOff("bv");

  AddRAMParameter();

  SetDocExampleParameterValue("in", "qb_RoadExtract.tif");
  SetDocExampleParameterValue("out", "classes.csv");
}

void DoUpdateParameters() override
{
}

void DoExecute() override
{
  Int32ImageType::Pointer inImage = GetParameterInt32Image("in");

  RadiusType rad;
  rad[0] = 1;
  rad[1] = 1;
  StructuringType se = StructuringType::Box(rad);

  int bv = GetParameterInt("bv");
  m_ErodeFilter = ErodeFilterType::New();
  m_ErodeFilter->SetKernel(se);
  m_ErodeFilter->SetInput(inImage);
  m_ErodeFilter->SetBackgroundValue(bv);

  m_HistogramFilter = StreamingHistogramFilterType::New();
  m_HistogramFilter->SetInput(m_ErodeFilter->GetOutput());

  m_HistogramFilter->Update();

  StreamingHistogramFilterType::LabelPopulationMapType populationMap = m_HistogramFilter->GetLabelPopulationMap();

  std::ofstream fout(GetParameterString("out"));
  for (auto it = populationMap.begin(); it != populationMap.end(); ++it)
    if (it->first != bv)
      fout << it->first << ',' << it->second << '\n';
}

ErodeFilterType::Pointer                           m_ErodeFilter;
StreamingHistogramFilterType::Pointer              m_HistogramFilter;

};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::ComputeClassCounts)
