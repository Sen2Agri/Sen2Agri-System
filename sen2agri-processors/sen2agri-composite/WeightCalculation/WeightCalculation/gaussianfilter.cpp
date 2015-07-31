#include "gaussianfilter.h"

#include "otbVectorRescaleIntensityImageFilter.h"
//#include "itkVectorIndexSelectionCastImageFilter.h"

GaussianFilter::GaussianFilter()
{

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

void GaussianFilter::SetOutputFileName(std::string &outFile)
{
    m_outputFileName = outFile;
}

void GaussianFilter::SetInputImage(GaussianFilter::ImageType::Pointer inputImage)
{
    if (inputImage.IsNull())
    {
        std::cout << "No input Image set...; please set the input image!" << std::endl;
        itkExceptionMacro("No input Image set...; please set the input image");
    }
    m_inputImage = inputImage;
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

GaussianFilter::ImageType::Pointer GaussianFilter::GetProducedImage()
{
    return m_gaussianFilter->GetOutput();
}

GaussianFilter::OutImageSource::Pointer GaussianFilter::GetOutputImageSource()
{
    return (OutImageSource::Pointer)m_gaussianFilter;
}

void GaussianFilter::Update()
{
/*    typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, ScalarImageType> IndexSelectionType;
    IndexSelectionType::Pointer indexSelectionFilter = IndexSelectionType::New();
    indexSelectionFilter->SetIndex(0);
    indexSelectionFilter->SetInput(m_inputImage);
    indexSelectionFilter->Update();
*/
    m_gaussianFilter = DiscreteGaussianFilterType::New();
    m_gaussianFilter->SetInput(m_inputReader->GetOutput());
    m_gaussianFilter->SetVariance(m_fSigma);
    m_gaussianFilter->SetMaximumKernelWidth(32);
    //m_gaussianFilter->Update();

//    typedef otb::VectorRescaleIntensityImageFilter<
//        ScalarImageType, ScalarImageType> RescaleFilterType;

    //ScalarImageType::Pointer output = ScalarImageType::New();
    //ScalarImageType::PixelType outMin, outMax;
    //output->SetRegions(m_inputReader->GetOutput()->GetRequestedRegion());
    //output->Allocate();

    //outMin.SetSize( filter->GetNumberOfComponentsPerPixel() );
    //outMax.SetSize( output->GetNumberOfComponentsPerPixel() );
    //outMin.Fill( 0 );
    //outMax.Fill( 255 );

    // TODO: Not sure if rescaling is really needed
    //m_rescaler = RescaleFilterType::New();
    //m_rescaler->SetOutputMinimum(0/*itk::NumericTraits< OutputPixelType >::min()*/);
    //m_rescaler->SetOutputMaximum( 1/* itk::NumericTraits< OutputPixelType >::max()*/);
    //m_rescaler->SetInput(m_gaussianFilter->GetOutput());

}

void GaussianFilter::WriteToOutputFile()
{
    if(!m_outputFileName.empty())
    {
        WriterType::Pointer writer;
        writer = WriterType::New();
        writer->SetFileName(m_outputFileName);
        writer->SetInput(m_gaussianFilter->GetOutput());
        try
        {
            writer->Update();
            m_inputImage = m_inputReader->GetOutput();
            ImageType::SpacingType spacing = m_inputImage->GetSpacing();
            ImageType::PointType origin = m_inputImage->GetOrigin();
            std::cout << "=================================" << std::endl;
            std::cout << "Origin : " << origin[0] << " " << origin[1] << std::endl;
            std::cout << "Spacing : " << spacing[0] << " " << spacing[1] << std::endl;
            std::cout << "Size : " << m_inputImage->GetLargestPossibleRegion().GetSize()[0] << " " <<
                         m_inputImage->GetLargestPossibleRegion().GetSize()[1] << std::endl;

            ImageType::SpacingType outspacing = m_gaussianFilter->GetOutput()->GetSpacing();
            ImageType::PointType outorigin = v->GetOutput()->GetOrigin();
            std::cout << "Output Origin : " << outorigin[0] << " " << outorigin[1] << std::endl;
            std::cout << "Output Spacing : " << outspacing[0] << " " << outspacing[1] << std::endl;
            std::cout << "Size : " << m_gaussianFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[0] << " " <<
                         m_gaussianFilter->GetOutput()->GetLargestPossibleRegion().GetSize()[1] << std::endl;

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
