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
 
#include "cloudmaskbinarization.h"

CloudMaskBinarization::CloudMaskBinarization()
{
}

void CloudMaskBinarization::SetInputFileName(std::string &inputImageStr)
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
        m_inputReader = reader;
    }
    catch (itk::ExceptionObject& err)
    {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
        itkExceptionMacro("Error reading input");
    }
}

void CloudMaskBinarization::SetInputImageReader(ImageSource::Pointer inputReader)
{
    if (inputReader.IsNull())
    {
        std::cout << "No input Image set...; please set the input image!" << std::endl;
        itkExceptionMacro("No input Image set...; please set the input image");
    }
    m_inputReader = inputReader;
}

void CloudMaskBinarization::SetOutputFileName(std::string &outFile)
{
    m_outputFileName = outFile;
}

CloudMaskBinarization::OutImageSource::Pointer CloudMaskBinarization::GetOutputImageSource()
{
    BuildOutputImageSource();
    return (OutImageSource::Pointer)m_filter;
}

int CloudMaskBinarization::GetInputImageResolution()
{
   m_inputReader->UpdateOutputInformation();
   ImageType::Pointer inputImage = m_inputReader->GetOutput();
   return inputImage->GetSpacing()[0];
}

void CloudMaskBinarization::BuildOutputImageSource()
{
    m_filter = FilterType::New();
    m_filter->SetInput(m_inputReader->GetOutput());
}

void CloudMaskBinarization::WriteToOutputFile()
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
            ImageType::Pointer image = m_inputReader->GetOutput();
            ImageType::SpacingType spacing = image->GetSpacing();
            ImageType::PointType origin = image->GetOrigin();
            std::cout << "=================================" << std::endl;
            std::cout << "Input Origin : " << origin[0] << " " << origin[1] << std::endl;
            std::cout << "Input Spacing : " << spacing[0] << " " << spacing[1] << std::endl;
            std::cout << "Size : " << image->GetLargestPossibleRegion().GetSize()[0] << " " <<
                         image->GetLargestPossibleRegion().GetSize()[1] << std::endl;

            ImageType::SpacingType outspacing = m_filter->GetOutput()->GetSpacing();
            ImageType::PointType outorigin = m_filter->GetOutput()->GetOrigin();
            std::cout << "Output Origin : " << outorigin[0] << " " << outorigin[1] << std::endl;
            std::cout << "Output Spacing : " << outspacing[0] << " " << outspacing[1] << std::endl;
            std::cout << "Size : " << m_filter->GetOutput()->GetLargestPossibleRegion().GetSize()[0] << " " <<
                         m_filter->GetOutput()->GetLargestPossibleRegion().GetSize()[1] << std::endl;

            std::cout  << "=================================" << std::endl;
            std::cout << std::endl;
        }
        catch (itk::ExceptionObject& err)
        {
            std::cout << "ExceptionObject caught !" << std::endl;
            std::cout << err << std::endl;
            itkExceptionMacro("Error writing output");
        }
    }
}

