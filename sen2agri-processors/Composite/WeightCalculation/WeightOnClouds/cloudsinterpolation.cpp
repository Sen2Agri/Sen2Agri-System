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
 
#include "cloudsinterpolation.h"

#include "itkComposeImageFilter.h"

CloudsInterpolation::CloudsInterpolation()
{
    m_interpolator = Interpolator_BCO;
    m_BCORadius = 2;
    m_outForcedWidth = -1;
    m_outForcedHeight = -1;
    m_inputRes = -1;
    m_outputRes = -1;
}

void CloudsInterpolation::SetInputFileName(std::string &inputImageStr)
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

void CloudsInterpolation::SetOutputFileName(std::string &outFile)
{
    m_outputFileName = outFile;
}

void CloudsInterpolation::SetInputImageReader(ImageSource::Pointer inputReader)
{
    if (inputReader.IsNull())
    {
        std::cout << "No input Image set...; please set the input image!" << std::endl;
        itkExceptionMacro("No input Image set...; please set the input image");
    }
    m_inputReader = inputReader;
}

void CloudsInterpolation::SetInputResolution(int inputRes)
{
    m_inputRes = inputRes;
}

void CloudsInterpolation::SetOutputResolution(int outputRes)
{
    m_outputRes = outputRes;
}

void CloudsInterpolation::GetInputImageDimension(long &width, long &height)
{
   m_inputReader->UpdateOutputInformation();
   ImageType::Pointer inputImage = m_inputReader->GetOutput();
   width = inputImage->GetLargestPossibleRegion().GetSize()[0];
   height = inputImage->GetLargestPossibleRegion().GetSize()[1];
}

void CloudsInterpolation::SetOutputForcedSize(long forcedWidth, long forcedHeight)
{
    if((forcedWidth > 0) && (forcedHeight > 0))
    {
        m_outForcedWidth = forcedWidth;
        m_outForcedHeight = forcedHeight;
    }
}

void CloudsInterpolation::SetInterpolator(Interpolator_Type interpolator)
{
    m_interpolator = interpolator;
}

void CloudsInterpolation::SetBicubicInterpolatorRadius(int bcoRadius)
{
    m_BCORadius = bcoRadius;
}

CloudsInterpolation::OutImageSource::Pointer CloudsInterpolation::GetOutputImageSource()
{
    BuildOutputImageSource();
    return (OutImageSource::Pointer)m_Resampler;
}

void CloudsInterpolation::BuildOutputImageSource()
{
    if(m_outputRes < 0) {
        itkExceptionMacro("Invalid output resolution " << m_outputRes);
    }

    m_inputReader->UpdateOutputInformation();
    ImageType::Pointer inputImage = m_inputReader->GetOutput();
    m_Resampler = ResampleFilterType::New();
    m_Resampler->SetInput(inputImage);

    if(m_inputRes < 0) {
        m_inputRes = abs(inputImage->GetSpacing()[0]);
    }

    // Get Interpolator
    switch ( m_interpolator )
    {
    case Interpolator_Linear:
    {
        typedef itk::LinearInterpolateImageFunction<ImageType,
                double>          LinearInterpolationType;
        LinearInterpolationType::Pointer interpolator = LinearInterpolationType::New();
        m_Resampler->SetInterpolator(interpolator);
    }
        break;
    case Interpolator_NNeighbor:
    {
        typedef itk::NearestNeighborInterpolateImageFunction<ImageType,
                double> NearestNeighborInterpolationType;
        NearestNeighborInterpolationType::Pointer interpolator = NearestNeighborInterpolationType::New();
        m_Resampler->SetInterpolator(interpolator);
    }
        break;
    case Interpolator_BCO:
    {
        typedef otb::BCOInterpolateImageFunction<ImageType> BCOInterpolationType;
        BCOInterpolationType::Pointer interpolator = BCOInterpolationType::New();
        interpolator->SetRadius(m_BCORadius);
        m_Resampler->SetInterpolator(interpolator);
    }
        break;
    }

    IdentityTransformType::Pointer transform = IdentityTransformType::New();

    m_Resampler->SetOutputParametersFromImage( inputImage );
    // Scale Transform
    float scaleXY = ((float)m_inputRes)/((float)m_outputRes);

    OutputVectorType scale;
    scale[0] = 1.0 / scaleXY;
    scale[1] = 1.0 / scaleXY;

    // Evaluate spacing
    ImageType::SpacingType spacing = inputImage->GetSpacing();

    //NOTE: If computation is performed with spacing and scale we might have
    //      a model pixel scale tag of 9.9999 instead of 10 that can result in
    //      different number of pixels
    ImageType::SpacingType OutputSpacing;
    OutputSpacing[0] = std::round(spacing[0] * scale[0]);
    OutputSpacing[1] = std::round(spacing[1] * scale[1]);

    m_Resampler->SetOutputSpacing(OutputSpacing);

    ImageType::PointType origin = inputImage->GetOrigin();
    ImageType::PointType outputOrigin;
    outputOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale[0] - 1.0);
    outputOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale[1] - 1.0);

    m_Resampler->SetOutputOrigin(outputOrigin);
    m_Resampler->SetTransform(transform);

    // Evaluate size
    ResampleFilterType::SizeType recomputedSize;
    if((m_outForcedWidth != -1) && (m_outForcedHeight != -1))
    {
        recomputedSize[0] = m_outForcedWidth;
        recomputedSize[1] = m_outForcedHeight;
    } else {
        recomputedSize[0] = inputImage->GetLargestPossibleRegion().GetSize()[0] / scale[0];
        recomputedSize[1] = inputImage->GetLargestPossibleRegion().GetSize()[1] / scale[1];
    }

    m_Resampler->SetOutputSize(recomputedSize);
    //otbAppLogINFO( << "Output image size : " << recomputedSize );

    ImageType::PixelType defaultValue;
    itk::NumericTraits<ImageType::PixelType>::SetLength(defaultValue, inputImage->GetNumberOfComponentsPerPixel());
    m_Resampler->SetEdgePaddingValue(defaultValue);

    //m_Resampler->UpdateOutputInformation();
    // Output Image
    //SetParameterOutputImage("out", m_Resampler->GetOutput());

}

void CloudsInterpolation::WriteToOutputFile()
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

            ImageType::SpacingType outspacing = m_Resampler->GetOutput()->GetSpacing();
            ImageType::PointType outorigin = m_Resampler->GetOutput()->GetOrigin();
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
