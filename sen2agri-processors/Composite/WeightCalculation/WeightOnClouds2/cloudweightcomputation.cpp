#include "cloudweightcomputation.h"

CloudWeightComputation::CloudWeightComputation()
{
}

void CloudWeightComputation::SetInputFileName1(std::string &inputImageStr)
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
        //m_image1 = reader->GetOutput();
        m_inputReader1 = reader;
    }
    catch (itk::ExceptionObject& err)
    {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
        itkExceptionMacro("Error reading input");
    }
}

void CloudWeightComputation::SetInputFileName2(std::string &inputImageStr)
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
        m_inputReader2 = reader;
    }
    catch (itk::ExceptionObject& err)
    {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
        itkExceptionMacro("Error reading input");
    }
}

void CloudWeightComputation::SetInputImageReader1(CloudWeightComputation::ImageSource::Pointer inputReader)
{
    if (inputReader.IsNull())
    {
        std::cout << "No input Image set...; please set the input image!" << std::endl;
        itkExceptionMacro("No input Image set...; please set the input image");
    }
    m_inputReader1 = inputReader;
}

void CloudWeightComputation::SetInputImageReader2(CloudWeightComputation::ImageSource::Pointer inputReader)
{
    if (inputReader.IsNull())
    {
        std::cout << "No input Image set...; please set the input image!" << std::endl;
        itkExceptionMacro("No input Image set...; please set the input image");
    }
    m_inputReader2 = inputReader;
}

void CloudWeightComputation::SetOutputFileName(std::string &outFile)
{
    m_outputFileName = outFile;
}

CloudWeightComputation::OutImageSource::Pointer CloudWeightComputation::GetOutputImageSource()
{
    BuildOutputImageSource();
    return (OutImageSource::Pointer)m_filter;
}

void CloudWeightComputation::BuildOutputImageSource()
{
    m_filter = FilterType::New();
    m_filter->SetInput1(m_inputReader1->GetOutput());
    m_filter->SetInput2(m_inputReader2->GetOutput());
}

void CloudWeightComputation::WriteToOutputFile()
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
            ImageType::Pointer image1 = m_inputReader1->GetOutput();
            ImageType::SpacingType spacing = image1->GetSpacing();
            ImageType::PointType origin = image1->GetOrigin();
            std::cout << "=============CLOUD WEIGHT COMPUTATION====================" << std::endl;
            std::cout << "Origin : " << origin[0] << " " << origin[1] << std::endl;
            std::cout << "Spacing : " << spacing[0] << " " << spacing[1] << std::endl;
            ImageType::SpacingType outspacing = m_filter->GetOutput()->GetSpacing();
            std::cout << "Size : " << image1->GetLargestPossibleRegion().GetSize()[0] << " " <<
                         image1->GetLargestPossibleRegion().GetSize()[1] << std::endl;

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
