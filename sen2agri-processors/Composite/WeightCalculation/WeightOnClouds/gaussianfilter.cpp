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
 
#include "gaussianfilter.h"

#include "otbVectorRescaleIntensityImageFilter.h"
//#include "itkVectorIndexSelectionCastImageFilter.h"

GaussianFilter::GaussianFilter()
{
    m_nKernelWidth = 81;
}

void GaussianFilter::SetInputFileName(std::string &inputImageStr)
{
    if (inputImageStr.empty())
    {
        std::cout << "No input Image set...; please set the input image!" << std::endl;
        itkExceptionMacro("No input Image set...; please set the input image");
    }
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(inputImageStr);
    try
    {
        //reader->Update();
        //m_inputImage = reader->GetOutput();
        m_inputReader = reader;
    }
    catch (itk::ExceptionObject& err)
    {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
        itkExceptionMacro("Error reading input");
    }
}

void GaussianFilter::SetOutputFileName(std::string &outFile)
{
    m_outputFileName = outFile;
}

void GaussianFilter::SetInputImageReader(ImageSource::Pointer inputReader)
{
    if (inputReader.IsNull())
    {
        std::cout << "No input Image set...; please set the input image!" << std::endl;
        itkExceptionMacro("No input Image set...; please set the input image");
    }
    m_inputReader = inputReader;
}


void GaussianFilter::SetSigma(float fSigma)
{
    m_fSigma = fSigma;
}

void GaussianFilter::SetKernelWidth(int nKernelWidth)
{
    m_nKernelWidth = nKernelWidth;
}

GaussianFilter::OutImageSource::Pointer GaussianFilter::GetOutputImageSource()
{
    BuildOutputImageSource();
    return (OutImageSource::Pointer)m_gaussianFilter;
}

void GaussianFilter::BuildOutputImageSource()
{
    m_gaussianFilter = DiscreteGaussianFilterType::New();
    m_gaussianFilter->SetInput(m_inputReader->GetOutput());
    // the variance is sigma^2
    m_gaussianFilter->SetVariance(m_fSigma*m_fSigma);
    m_gaussianFilter->SetUseImageSpacing(false);
    //m_gaussianFilter->SetMaximumError(0.00001);
    m_gaussianFilter->SetMaximumKernelWidth(m_nKernelWidth);
}

void GaussianFilter::WriteToOutputFile()
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
            ImageType::Pointer inputImage = m_inputReader->GetOutput();
            ImageType::SpacingType spacing = inputImage->GetSpacing();
            ImageType::PointType origin = inputImage->GetOrigin();
            std::cout << "=================================" << std::endl;
            std::cout << "Origin : " << origin[0] << " " << origin[1] << std::endl;
            std::cout << "Spacing : " << spacing[0] << " " << spacing[1] << std::endl;
            std::cout << "Size : " << inputImage->GetLargestPossibleRegion().GetSize()[0] << " " <<
                         inputImage->GetLargestPossibleRegion().GetSize()[1] << std::endl;

            ImageType::SpacingType outspacing = m_gaussianFilter->GetOutput()->GetSpacing();
            ImageType::PointType outorigin = m_gaussianFilter->GetOutput()->GetOrigin();
            std::cout << "Output Origin : " << outorigin[0] << " " << outorigin[1] << std::endl;
            std::cout << "Output Spacing : " << outspacing[0] << " " << outspacing[1] << std::endl;
            std::cout << "Size : " << m_gaussianFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[0] << " " <<
                         m_gaussianFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[1] << std::endl;

            std::cout << "Sigma : " << m_fSigma << std::endl;
            std::cout << "Kernel Width : " << m_nKernelWidth << std::endl;

            std::cout  << "=================================" << std::endl;
            std::cout << std::endl;

            std::stringstream ss;
            m_gaussianFilter.Print(ss);
            std::cout << ss.str() << std::endl;

        }
        catch (itk::ExceptionObject& err)
        {
            std::cout << "ExceptionObject caught !" << std::endl;
            std::cout << err << std::endl;
            itkExceptionMacro("Error writing output");
        }
    }
}
