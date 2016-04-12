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
 
#include "weightonaot.h"

WeightOnAOT::WeightOnAOT()
{
}

void WeightOnAOT::SetInputFileName(std::string &inputImageStr)
{
    if (inputImageStr.empty())
    {
        std::cout << "No input Image set...; please set the input image!" << std::endl;
        itkExceptionMacro("No input Image set...; please set the input image");
    }
    // Read the image
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(inputImageStr);
    try
    {
        //reader->Update();
        //m_image = reader->GetOutput();
        m_inputReader = reader;
    }
    catch (itk::ExceptionObject& err)
    {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
        itkExceptionMacro("Error reading input");
    }
}

void WeightOnAOT::SetInputImageReader(ImageSource::Pointer inputReader)
{
    if (inputReader.IsNull())
    {
        std::cout << "No input Image set...; please set the input image!" << std::endl;
        itkExceptionMacro("No input Image set...; please set the input image");
    }
    m_inputReader = inputReader;
}

void WeightOnAOT::SetOutputFileName(std::string &outFile)
{
    m_outputFileName = outFile;
}

void WeightOnAOT::Initialize(int nBand, float fQuantif, float fAotMax, float fMinWeight, float fMaxWeight) {
    m_nBand = nBand;
    m_fAotQuantificationVal = fQuantif;
    m_fAotMax = fAotMax;
    m_fMinWeightAot = fMinWeight;
    m_fMaxWeightAot = fMaxWeight;
}

WeightOnAOT::OutImageSource::Pointer WeightOnAOT::GetOutputImageSource()
{
    BuildOutputImageSource();
    return (OutImageSource::Pointer)m_filter;
}

int WeightOnAOT::GetInputImageResolution()
{
   m_inputReader->UpdateOutputInformation();
   ImageType::Pointer inputImage = m_inputReader->GetOutput();
   return inputImage->GetSpacing()[0];
}


void WeightOnAOT::BuildOutputImageSource()
{
    m_inputReader->UpdateOutputInformation();
    int nBands = m_inputReader->GetOutput()->GetNumberOfComponentsPerPixel();
    if(m_nBand >= nBands) {
        itkExceptionMacro("Invalid band number " << m_nBand << ". It should be less than " << nBands);
    }
    m_filter = FilterType::New();
    m_filter->GetFunctor().Initialize(m_nBand, m_fAotQuantificationVal,
                                      m_fAotMax, m_fMinWeightAot, m_fMaxWeightAot);
    m_filter->SetInput(m_inputReader->GetOutput());
}

void WeightOnAOT::WriteToOutputFile()
{
    if(!m_outputFileName.empty())
    {
        if(!m_outputFileName.empty())
        {
            WriterType::Pointer writer;
            writer = WriterType::New();
            writer->SetFileName(m_outputFileName);
            writer->SetInput(GetOutputImageSource()->GetOutput());
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
    }
}


