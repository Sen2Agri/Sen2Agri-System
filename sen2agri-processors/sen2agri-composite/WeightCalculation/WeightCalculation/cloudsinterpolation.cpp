#include "cloudsinterpolation.h"

#include "itkComposeImageFilter.h"

CloudsInterpolation::CloudsInterpolation()
{
    m_interpolator = Interpolator_BCO;
    m_BCORadius = 2;
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
        reader->Update();
        m_inputImage = reader->GetOutput();
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

void CloudsInterpolation::SetInputImage(CloudsInterpolation::ImageType::Pointer inputImage)
{
    if (inputImage.IsNull())
    {
        std::cout << "No input Image set...; please set the input image!" << std::endl;
        itkExceptionMacro("No input Image set...; please set the input image");
    }
    m_inputImage = inputImage;
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

/*
void CloudsInterpolation::SetInputImage(ScalarImageType::Pointer inputImage)
{
    if (inputImage.IsNull())
    {
        itkExceptionMacro("No input Image set...; please set the input image");
    }
    m_inputImage = FloatImageType::New();
    m_inputImage->CopyInformation( inputImage );
    m_inputImage->SetRegions(inputImage->GetRequestedRegion());
    m_inputImage->SetVectorLength(1);
    m_inputImage->Allocate();
    m_inputImage->Update();
/*
    typedef itk::ComposeImageFilter<ScalarImageType> ImageToVectorImageFilterType;
    ImageToVectorImageFilterType::Pointer imageToVectorImageFilter = ImageToVectorImageFilterType::New();
    imageToVectorImageFilter->SetInput(0, inputImage);
    imageToVectorImageFilter->Update();
    //m_inputImage = imageToVectorImageFilter->GetOutput();
//    m_inputImage = static_cast< FloatVectorImageType >(imageToVectorImageFilter->GetOutput());
*/
/*}*/

void CloudsInterpolation::SetInputResolution(int inputRes)
{
    m_inputRes = inputRes;
}

void CloudsInterpolation::SetOutputResolution(int outputRes)
{
    m_outputRes = outputRes;
}

void CloudsInterpolation::SetInterpolator(Interpolator_Type interpolator)
{
    m_interpolator = interpolator;
}

void CloudsInterpolation::SetBicubicInterpolatorRadius(int bcoRadius)
{
    m_BCORadius = bcoRadius;
}

const CloudsInterpolation::ImageType::Pointer CloudsInterpolation::GetProducedImage()
{
    return m_Resampler->GetOutput();
}

CloudsInterpolation::OutImageSource::Pointer CloudsInterpolation::GetOutputImageSource()
{
    return (OutImageSource::Pointer)m_Resampler;
}


void CloudsInterpolation::Update()
{
    m_Resampler = ResampleFilterType::New();
    m_Resampler->SetInput(m_inputReader->GetOutput());
    m_inputReader->Update();
    m_inputImage = m_inputReader->GetOutput();

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

    m_Resampler->SetOutputParametersFromImage( m_inputReader->GetOutput() );
    // Scale Transform
    float scaleXY = ((float)m_inputRes)/((float)m_outputRes);

    OutputVectorType scale;
    scale[0] = 1.0 / scaleXY;
    scale[1] = 1.0 / scaleXY;

    // Evaluate spacing
//    ImageType::SpacingType spacing = m_inputImage->GetSpacing();
    //NOTE: If computation is performed with spacing and scale we might have
    //      a model pixel scale tag of 9.9999 instead of 10 that can result in
    //      different number of pixels
    ImageType::SpacingType OutputSpacing;
    OutputSpacing[0] = m_outputRes; //spacing[0] * scale[0];
    OutputSpacing[1] = m_outputRes; //spacing[1] * scale[1];

    m_Resampler->SetOutputSpacing(OutputSpacing);

    ImageType::PointType origin = m_inputImage->GetOrigin();
    ImageType::PointType outputOrigin;
    outputOrigin[0] = origin[0] + 0.5 * m_outputRes/*spacing[0]*/ * (scale[0] - 1.0);
    outputOrigin[1] = origin[1] + 0.5 * m_outputRes/*spacing[1]*/ * (scale[1] - 1.0);

    m_Resampler->SetOutputOrigin(outputOrigin);
    m_Resampler->SetTransform(transform);

    // Evaluate size
    ResampleFilterType::SizeType recomputedSize;
    recomputedSize[0] = m_inputImage->GetLargestPossibleRegion().GetSize()[0] / scale[0];
    recomputedSize[1] = m_inputImage->GetLargestPossibleRegion().GetSize()[1] / scale[1];

    m_Resampler->SetOutputSize(recomputedSize);
    //otbAppLogINFO( << "Output image size : " << recomputedSize );

    ImageType::PixelType defaultValue;
    itk::NumericTraits<ImageType::PixelType>::SetLength(defaultValue, m_inputImage->GetNumberOfComponentsPerPixel());
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
        writer->SetInput(m_Resampler->GetOutput());
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
