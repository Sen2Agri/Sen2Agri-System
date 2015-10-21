#include "ComputeNDVI.h"
#include "MetadataHelperFactory.h"
#include "otbWrapperMacros.h"

ComputeNDVI::ComputeNDVI()
{
}

void ComputeNDVI::DoInit(std::string &xml, int nRes)
{
    m_inXml = xml;
    m_nResolution = nRes;
}

// The algorithm consists in a applying a formula for computing the NDVI for each pixel,
// using BandMathFilter
ComputeNDVI::OutputImageType::Pointer ComputeNDVI::DoExecute()
{
    auto factory = MetadataHelperFactory::New();
    auto pHelper = factory->GetMetadataHelper(m_inXml);
    // the bands are 1 based
    int nNirBandIdx = pHelper->GetNirBandIndex()-1;
    int nRedBandIdx = pHelper->GetRedBandIndex()-1;
    //Read all input parameters
    m_InputImageReader = ImageReaderType::New();
    std::string imgFileName = pHelper->GetImageFileName();

    std::cout << "ComputeNDVI -> Image File Name: " << imgFileName << std::endl;

    m_InputImageReader->SetFileName(imgFileName);
    m_InputImageReader->UpdateOutputInformation();
    ImageType::Pointer img = m_InputImageReader->GetOutput();
    int curRes = img->GetSpacing()[0];

    m_Functor = FilterType::New();
    m_Functor->GetFunctor().Initialize(nRedBandIdx, nNirBandIdx);
    m_Functor->SetInput(m_InputImageReader->GetOutput());
    m_Functor->UpdateOutputInformation();

    //WriteToOutputFile();
    if(m_nResolution != curRes) {
        float fMultiplicationFactor = ((float)curRes)/m_nResolution;
        return m_ResampledBandsExtractor.getResampler(m_Functor->GetOutput(), fMultiplicationFactor)->GetOutput();
    } else {
        return m_Functor->GetOutput();
    }
}

void ComputeNDVI::WriteToOutputFile()
{
    std::string outFileName("OUT_FILE_NAME.tif");
    WriterType::Pointer writer;
    writer = WriterType::New();
    writer->SetFileName(outFileName);
    if(m_nResolution == 20) {
        float fMultiplicationFactor = 0.5f;
        writer->SetInput(m_ResampledBandsExtractor.getResampler(m_Functor->GetOutput(), fMultiplicationFactor)->GetOutput());
    } else {
        writer->SetInput(m_Functor->GetOutput());
    }
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

