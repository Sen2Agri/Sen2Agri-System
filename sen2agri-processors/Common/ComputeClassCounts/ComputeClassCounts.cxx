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
#include "otbStreamingHistogramImageFilter.h"

#include <algorithm>
#include <tuple>

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

typedef Int32ImageType                                                         ImageType;
typedef otb::StreamingHistogramImageFilter<ImageType>                          StreamingHistogramFilterType;

/** Standard macro */
itkNewMacro(Self)
itkTypeMacro(ComputeClassCounts, otb::Application)

private:

void DoInit() override
{
  SetName( "ComputeClassCounts" );
  SetDescription( "Computes the class counts on a labelled image" );

  AddParameter( ParameterType_InputImage , "in" ,  "Input Image" );
  SetParameterDescription( "in" , "The input image to be filtered." );

  AddParameter( ParameterType_OutputFilename, "out" ,  "Output File" );
  SetParameterDescription( "out" , "Output CSV file with class counts." );

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

  m_HistogramFilter = StreamingHistogramFilterType::New();
  m_HistogramFilter->SetInput(inImage);

  m_HistogramFilter->Update();

  const auto &populationMap = m_HistogramFilter->GetLabelPopulationMap();

  std::vector<std::tuple<int32_t, uint64_t>> counts;
  counts.reserve(populationMap.size());

  for (auto it = populationMap.begin(); it != populationMap.end(); ++it)
    counts.emplace_back(std::make_tuple(it->first, it->second));

  std::sort(counts.begin(), counts.end());

  std::ofstream fout(GetParameterString("out"));
  for (const auto &p : counts)
    fout << std::get<0>(p) << ',' << std::get<1>(p) << '\n';
}

StreamingHistogramFilterType::Pointer              m_HistogramFilter;

};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::ComputeClassCounts)
