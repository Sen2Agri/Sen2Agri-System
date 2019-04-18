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
 
#include "ComputeNDVI.h"
#include "MetadataHelperFactory.h"
#include "otbWrapperMacros.h"

ComputeNDVI::ComputeNDVI()
{
}

void ComputeNDVI::DoInit(std::string &xml, int nRes)
{
    m_inXml = xml;
    m_nResolution = nRes;
}

// The algorithm consists in a applying a formula for computing the NDVI for each pixel,
// using BandMathFilter
ComputeNDVI::OutputImageType::Pointer ComputeNDVI::DoExecute()
{
    auto factory = MetadataHelperFactory::New();
    m_pHelper = factory->GetMetadataHelper<short>(m_inXml);
    std::vector<std::string> bandNames = {m_pHelper->GetRedBandName(), m_pHelper->GetNirBandName()};
    std::vector<int> resRelIdxs;
    MetadataHelper<short>::VectorImageType::Pointer img = m_pHelper->GetImage(bandNames, &resRelIdxs);
    img->UpdateOutputInformation();
    int curRes = img->GetSpacing()[0];

    m_Functor = FilterType::New();
    m_Functor->GetFunctor().Initialize(resRelIdxs[0], resRelIdxs[1]);
    m_Functor->SetInput(img);
    m_Functor->UpdateOutputInformation();

    //WriteToOutputFile();
    if(m_nResolution != curRes) {
        float fMultiplicationFactor = ((float)curRes)/m_nResolution;
        return m_ResampledBandsExtractor.getResampler(m_Functor->GetOutput(), fMultiplicationFactor)->GetOutput();
    } else {
        return m_Functor->GetOutput();
    }
}

void ComputeNDVI::WriteToOutputFile()
{
    std::string outFileName("OUT_FILE_NAME.tif");
    WriterType::Pointer writer;
    writer = WriterType::New();
    writer->SetFileName(outFileName);
    if(m_nResolution == 20) {
        float fMultiplicationFactor = 0.5f;
        writer->SetInput(m_ResampledBandsExtractor.getResampler(m_Functor->GetOutput(), fMultiplicationFactor)->GetOutput());
    } else {
        writer->SetInput(m_Functor->GetOutput());
    }
    try
    {
        writer->Update();
    }
    catch (itk::ExceptionObject& err)
    {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
        itkExceptionMacro("Error writing output");
    }
}

