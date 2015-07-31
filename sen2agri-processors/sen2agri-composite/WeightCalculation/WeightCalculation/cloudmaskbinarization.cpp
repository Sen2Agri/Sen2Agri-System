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
        reader->Update();
        m_image = reader->GetOutput();
        m_inputReader = reader;
    }
    catch (itk::ExceptionObject& err)
    {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
        itkExceptionMacro("Error reading input");
    }
}

void CloudMaskBinarization::SetInputImage(ImageType::Pointer image)
{
    m_image = image;
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

CloudMaskBinarization::OutImageType::Pointer CloudMaskBinarization::GetProducedImage()
{
    return m_filter->GetOutput();
}

CloudMaskBinarization::OutImageSource::Pointer CloudMaskBinarization::GetOutputImageSource()
{
    return (OutImageSource::Pointer)m_filter;
}

void CloudMaskBinarization::Update()
{
    m_filter = FilterType::New();
    m_filter->SetInput(m_inputReader->GetOutput());
}

void CloudMaskBinarization::WriteToOutputFile()
{
    if(!m_outputFileName.empty())
    {
        if(!m_outputFileName.empty())
        {
            WriterType::Pointer writer;
            writer = WriterType::New();
            writer->SetFileName(m_outputFileName);
            writer->SetInput(m_filter->GetOutput());
            try
            {
                writer->Update();
                m_image = m_inputReader->GetOutput();
                ImageType::SpacingType spacing = m_image->GetSpacing();
                ImageType::PointType origin = m_image->GetOrigin();
                std::cout << "=================================" << std::endl;
                std::cout << "Input Origin : " << origin[0] << " " << origin[1] << std::endl;
                std::cout << "Input Spacing : " << spacing[0] << " " << spacing[1] << std::endl;
                std::cout << "Size : " << m_image->GetLargestPossibleRegion().GetSize()[0] << " " <<
                             m_image->GetLargestPossibleRegion().GetSize()[1] << std::endl;

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
}

