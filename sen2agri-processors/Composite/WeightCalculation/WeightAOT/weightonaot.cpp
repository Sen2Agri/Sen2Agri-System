#include "weightonaot.h"

WeightOnAOT::WeightOnAOT()
{
}

void WeightOnAOT::SetInputFileName(std::string &inputImageStr)
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
        //m_image = reader->GetOutput();
        m_inputReader = reader;
    }
    catch (itk::ExceptionObject& err)
    {
        std::cout << "ExceptionObject caught !" << std::endl;
        std::cout << err << std::endl;
        itkExceptionMacro("Error reading input");
    }
}

void WeightOnAOT::SetInputImageReader(ImageSource::Pointer inputReader)
{
    if (inputReader.IsNull())
    {
        std::cout << "No input Image set...; please set the input image!" << std::endl;
        itkExceptionMacro("No input Image set...; please set the input image");
    }
    m_inputReader = inputReader;
}

void WeightOnAOT::SetOutputFileName(std::string &outFile)
{
    m_outputFileName = outFile;
}

void WeightOnAOT::SetBand(int band)
{
    m_nBand = band;
}

void WeightOnAOT::SetAotQuantificationValue(float fQuantifVal)
{
    m_fAotQuantificationVal = fQuantifVal;
}

void WeightOnAOT::SetAotMaxValue(int nMaxAot)
{
    m_nAotMax = nMaxAot;
}

void WeightOnAOT::SetMinAotWeight(float fMinWeightAot)
{
    m_fMinWeightAot = fMinWeightAot;
}

void WeightOnAOT::SetMaxAotWeight(float fMaxWeightAot)
{
    m_fMaxWeightAot = fMaxWeightAot;
}

WeightOnAOT::OutImageSource::Pointer WeightOnAOT::GetOutputImageSource()
{
    BuildOutputImageSource();
    return (OutImageSource::Pointer)m_Filter;
}

int WeightOnAOT::GetInputImageResolution()
{
   m_inputReader->UpdateOutputInformation();
   ImageType::Pointer inputImage = m_inputReader->GetOutput();
   return inputImage->GetSpacing()[0];
}


void WeightOnAOT::BuildOutputImageSource()
{
    ImageType::Pointer image;
    image = m_inputReader->GetOutput();

    image->UpdateOutputInformation();

    std::cout << "Input Image has " << image->GetNumberOfComponentsPerPixel() << " components" << std::endl;

    m_ChannelExtractorList = ExtractROIFilterListType::New();
    m_Filter               = BandMathImageFilterType::New();

    for (unsigned int j = 0; j < image->GetNumberOfComponentsPerPixel(); j++)
    {
        std::ostringstream tmpParserVarName;
        tmpParserVarName << "im1" << "b" << j + 1;

        m_ExtractROIFilter = ExtractROIFilterType::New();
        m_ExtractROIFilter->SetInput(m_inputReader->GetOutput());
        m_ExtractROIFilter->SetChannel(j + 1);
        m_ExtractROIFilter->GetOutput()->UpdateOutputInformation();
        m_ChannelExtractorList->PushBack(m_ExtractROIFilter);
        m_Filter->SetNthInput(j, m_ChannelExtractorList->Back()->GetOutput(), tmpParserVarName.str());
    }

    float fAotMax = m_nAotMax / m_fAotQuantificationVal;

    // The following formula is implemented
    // if (AOT(pix) <= AOTMax):
    //      WAOT = WAOTMin + (WAOTMax - WAOTMin) * (1 - AOT(p)/AOTMax)
    // else
    //      WAOT = WAOTMin
    std::ostringstream exprStream;
#ifdef OTB_MUPARSER_HAS_CXX_LOGICAL_OPERATORS
    // The expression that will be set is
    //      "(im1bX / AOTQuantificationVal <= AOTMax) ?
    //          WAOTMin + (WAOTMax-WAOTMin) * (1-im1bX/AOTQuantificationVal/AOTMax) :
    //          WAOTMin"
    exprStream << "(im1b" << m_nBand << "< 0) ? -10000 : ((im1b" << m_nBand << "/" << m_fAotQuantificationVal << "<=" << fAotMax << ") ? "
               <<  m_fMinWeightAot + (m_fMaxWeightAot-m_fMinWeightAot) << " * (1-" << "im1b" << m_nBand << "/" <<
               m_fAotQuantificationVal << "/" << fAotMax << ") : " << m_fMinWeightAot << ")";
#else
    // The expression that will be set is
    //      "if (im1bX / AOTQuantificationVal <= AOTMax),
    //          WAOTMin + (WAOTMax-WAOTMin) * (1-im1bX/AOTQuantificationVal/AOTMax),
    //          WAOTMin)"
    exprStream << "if ((im1b" << m_nBand << "/" << m_fAotQuantificationVal << "<=" << fAotMax << "), "
               <<  m_fMinWeightAot + (m_fMaxWeightAot-m_fMinWeightAot) << " * (1-" << "im1b" << m_nBand << "/" <<
               m_fAotQuantificationVal << "/" << fAotMax << "), " << m_fMinWeightAot << ")";
#endif

    std::cout << "Expression used for AOT: " << exprStream.str() << std::endl;

    m_Filter->SetExpression(exprStream.str());
}

void WeightOnAOT::WriteToOutputFile()
{
    if(!m_outputFileName.empty())
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


