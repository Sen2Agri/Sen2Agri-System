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
 
#ifndef CLOUDSINTERPOLATION_H
#define CLOUDSINTERPOLATION_H

#include "otbVectorImage.h"
#include "otbWrapperTypes.h"
#include "otbBCOInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

//Transform
#include "otbCompositeTransform.h"
#include "itkScalableAffineTransform.h"
#include "itkTranslationTransform.h"
#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"
#include "itkCenteredRigid2DTransform.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"

#include "otbStreamingResampleImageFilter.h"
#include "ImageResampler.h"

template <typename TInput, typename TOutput>
class CloudsInterpolation
{
public:
    //typedef otb::Wrapper::FloatImageType ImageType;
    typedef otb::ImageFileReader<TInput> ReaderType;
    typedef otb::ImageFileWriter<TOutput> WriterType;
    typedef otb::StreamingResampleImageFilter<TInput, TOutput, double>    ResampleFilterType;
    typedef itk::IdentityTransform<double, TOutput::ImageDimension>      IdentityTransformType;
    typedef itk::ScalableAffineTransform<double, TOutput::ImageDimension> ScalableTransformType;
    typedef typename ScalableTransformType::OutputVectorType                         OutputVectorType;

    typedef itk::ImageSource<TInput> ImageSource;
    typedef typename itk::ImageSource<TOutput> OutImageSource;

public:
    CloudsInterpolation() {
        m_interpolator = Interpolator_BCO;
        m_outForcedWidth = -1;
        m_outForcedHeight = -1;
        m_inputRes = -1;
        m_outputRes = -1;
        m_BCORadius = 2;
    }

    void SetOutputFileName(std::string &outFile) {
        m_outputFileName = outFile;
    }

    void SetInputFileName(std::string &inputImageStr)
    {
        if (inputImageStr.empty())
        {
            std::cout << "No input Image set...; please set the input image!" << std::endl;
            itkExceptionMacro("No input Image set...; please set the input image");
        }
        // Read the image
        typename ReaderType::Pointer reader = ReaderType::New();
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

    void SetInputImageReader(typename ImageSource::Pointer inputReader) {
        if (inputReader.IsNull())
        {
            std::cout << "No input Image set...; please set the input image!" << std::endl;
            itkExceptionMacro("No input Image set...; please set the input image");
        }
        m_inputReader = inputReader;
    }

    void SetInputResolution(int inputRes)
    {
        m_inputRes = inputRes;
    }

    void SetOutputResolution(int outputRes)
    {
        m_outputRes = outputRes;
    }

    void SetOutputForcedSize(long forcedWidth, long forcedHeight) {
        if((forcedWidth > 0) && (forcedHeight > 0))
        {
            m_outForcedWidth = forcedWidth;
            m_outForcedHeight = forcedHeight;
        }
    }

    void GetInputImageDimension(long &width, long &height) {
        m_inputReader->UpdateOutputInformation();
        width = m_inputReader->GetOutput()->GetLargestPossibleRegion().GetSize()[0];
        height = m_inputReader->GetOutput()->GetLargestPossibleRegion().GetSize()[1];
    }

    void SetInterpolator(Interpolator_Type interpolator) {
        m_interpolator = interpolator;
    }

    void SetBicubicInterpolatorRadius(int bcoRadius) {
        m_BCORadius = bcoRadius;
    }

    const char *GetNameOfClass() { return "CloudsInterpolation";}
    typename OutImageSource::Pointer GetOutputImageSource() {
        BuildOutputImageSource();
        return (typename OutImageSource::Pointer)m_Resampler;
    }

    int GetInputImageResolution()
    {
       m_inputReader->UpdateOutputInformation();
       return m_inputReader->GetOutput()->GetSpacing()[0];
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
                typename TOutput::SpacingType spacing = m_inputReader->GetOutput()->GetSpacing();
                typename TOutput::PointType origin = m_inputReader->GetOutput()->GetOrigin();
                std::cout << "==============CLOUD INTERPOLATION===================" << std::endl;
                std::cout << "Origin : " << origin[0] << " " << origin[1] << std::endl;
                std::cout << "Spacing : " << spacing[0] << " " << spacing[1] << std::endl;
                std::cout << "Size : " << m_inputReader->GetOutput()->GetLargestPossibleRegion().GetSize()[0] << " " <<
                             m_inputReader->GetOutput()->GetLargestPossibleRegion().GetSize()[1] << std::endl;

                typename TOutput::SpacingType outspacing = m_Resampler->GetOutput()->GetSpacing();
                typename TOutput::PointType outorigin = m_Resampler->GetOutput()->GetOrigin();
                std::cout << "Output Origin : " << outorigin[0] << " " << outorigin[1] << std::endl;
                std::cout << "Output Spacing : " << outspacing[0] << " " << outspacing[1] << std::endl;
                std::cout << "Size : " << m_Resampler->GetOutput()->GetLargestPossibleRegion().GetSize()[0] << " " <<
                             m_Resampler->GetOutput()->GetLargestPossibleRegion().GetSize()[1] << std::endl;

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
        if(m_outputRes < 0) {
            itkExceptionMacro("Invalid output resolution " << m_outputRes);
        }

        m_inputReader->UpdateOutputInformation();
        typename TInput::Pointer inputImage = m_inputReader->GetOutput();
        if(m_inputRes < 0) {
            m_inputRes = abs(inputImage->GetSpacing()[0]);
        }


        float scaleXY = ((float)m_outputRes)/((float)m_inputRes);
        OutputVectorType scale;
        scale[0] = scaleXY;
        scale[1] = scaleXY;
        m_ImageResampler.SetBicubicInterpolatorParameters(m_BCORadius);
        m_Resampler = m_ImageResampler.getResampler(
                    inputImage, scale, m_outForcedWidth, m_outForcedHeight, m_interpolator);
    }

    typename ImageSource::Pointer m_inputReader;
    int m_BCORadius;
    int m_inputRes;
    int m_outputRes;
    Interpolator_Type m_interpolator;
    ImageResampler<TInput, TOutput>     m_ImageResampler;
    typename ImageResampler<TInput, TOutput>::ResamplerPtr m_Resampler;

    std::string m_outputFileName;

    // during resampling at higher resolutions it might be needed to return with a specified dimension
    long m_outForcedWidth;
    long m_outForcedHeight;
};
#endif // CLOUDSINTERPOLATION_H
