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
 
#ifndef CLOUDWEIGHTCOMPUTATION_H
#define CLOUDWEIGHTCOMPUTATION_H

#include "otbWrapperTypes.h"
#include "itkBinaryFunctorImageFilter.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "GlobalDefs.h"

namespace Functor
{
template< class TPixel>
class WeightOnCloudsCalculation
{
public:
  WeightOnCloudsCalculation() {}
  ~WeightOnCloudsCalculation() {}
  bool operator!=(const WeightOnCloudsCalculation &) const
  {
    return false;
  }

  bool operator==(const WeightOnCloudsCalculation & other) const
  {
    return !( *this != other );
  }

  inline TPixel operator()(const TPixel & A,
                            const TPixel & B) const
  {
    const float dA = fabs(static_cast< float >( A ));
    const float dB = fabs(static_cast< float >( B ));
    float weight;
    if(dA >= 1.0 || dB >= 1.0) {
        weight = 0.0;
    } else {
        weight = (1-dA) * (1 - dB);
    }

    return static_cast< TPixel >( weight );
  }
};
}

template <typename TInput, typename TOutput=TInput>
class CloudWeightComputation
{
public:
    //typedef otb::Wrapper::FloatImageType ImageType;
    typedef itk::BinaryFunctorImageFilter< TInput, TInput, TOutput,
                              Functor::WeightOnCloudsCalculation<typename TOutput::PixelType> > FilterType;
    typedef otb::ImageFileReader<TInput> ReaderType;
    typedef otb::ImageFileWriter<TOutput> WriterType;

    typedef itk::ImageSource<TInput> ImageSource;
    typedef typename itk::ImageSource<TOutput> OutImageSource;

public:
    CloudWeightComputation() {}

    void SetInputFileName1(std::string &inputImageStr) {
        if (inputImageStr.empty()) {
            std::cout << "No input Image set...; please set the input image!" << std::endl;
            itkExceptionMacro("No input Image set...; please set the input image");
        }
        // Read the image
        typename ReaderType::Pointer reader = ReaderType::New();
        reader->SetFileName(inputImageStr);
        m_inputReader1 = reader;
    }

    void SetInputFileName2(std::string &inputImageStr) {
        if (inputImageStr.empty()) {
            std::cout << "No input Image set...; please set the input image!" << std::endl;
            itkExceptionMacro("No input Image set...; please set the input image");
        }
        // Read the image
        typename ReaderType::Pointer reader = ReaderType::New();
        reader->SetFileName(inputImageStr);
        m_inputReader2 = reader;
    }

    void SetInputImageReader1(typename ImageSource::Pointer inputReader) {
        if (inputReader.IsNull())
        {
            std::cout << "No input Image set...; please set the input image!" << std::endl;
            itkExceptionMacro("No input Image set...; please set the input image");
        }
        m_inputReader1 = inputReader;
    }

    void SetInputImageReader2(typename ImageSource::Pointer inputReader) {
        if (inputReader.IsNull())
        {
            std::cout << "No input Image set...; please set the input image!" << std::endl;
            itkExceptionMacro("No input Image set...; please set the input image");
        }
        m_inputReader2 = inputReader;
    }

    void SetOutputFileName(std::string &outFile) { m_outputFileName = outFile; }

    const char *GetNameOfClass() { return "CloudWeightComputation";}
    typename OutImageSource::Pointer GetOutputImageSource() {
        BuildOutputImageSource();
        return (typename OutImageSource::Pointer)m_filter;
    }

    void WriteToOutputFile() {
        if(!m_outputFileName.empty())
        {
            typename WriterType::Pointer writer;
            writer = WriterType::New();
            writer->SetFileName(m_outputFileName);
            writer->SetInput(GetOutputImageSource()->GetOutput());
            try
            {
                writer->Update();
                typename TInput::Pointer image1 = m_inputReader1->GetOutput();
                typename TInput::SpacingType spacing = image1->GetSpacing();
                typename TInput::PointType origin = image1->GetOrigin();
                std::cout << "=============CLOUD WEIGHT COMPUTATION====================" << std::endl;
                std::cout << "Origin : " << origin[0] << " " << origin[1] << std::endl;
                std::cout << "Spacing : " << spacing[0] << " " << spacing[1] << std::endl;
                typename TInput::SpacingType outspacing = m_filter->GetOutput()->GetSpacing();
                std::cout << "Size : " << image1->GetLargestPossibleRegion().GetSize()[0] << " " <<
                             image1->GetLargestPossibleRegion().GetSize()[1] << std::endl;

                typename TOutput::PointType outorigin = m_filter->GetOutput()->GetOrigin();
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

private:
    void BuildOutputImageSource() {
        m_filter = FilterType::New();
        m_filter->SetInput1(m_inputReader1->GetOutput());
        m_filter->SetInput2(m_inputReader2->GetOutput());
    }

    typename ImageSource::Pointer m_inputReader1;
    typename ImageSource::Pointer m_inputReader2;
    std::string m_outputFileName;
    typename FilterType::Pointer m_filter;
};

#endif // WEIGHTONCLOUDS_H
