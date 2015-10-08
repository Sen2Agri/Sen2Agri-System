#include "ComputeNDVI.h"


ComputeNDVI::ComputeNDVI()
{
}

void ComputeNDVI::DoInit(std::string &xml)
{
    m_inXml = xml;
}

// The algorithm consists in a applying a formula for computing the NDVI for each pixel,
// using BandMathFilter
ComputeNDVI::OutputImageType::Pointer ComputeNDVI::DoExecute()
{
   MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    std::vector<char> buf(m_inXml.begin(), m_inXml.end());
    m_DirName = std::string(dirname(buf.data()));
    m_DirName += '/';
    auto meta = maccsMetadataReader->ReadMetadata(m_inXml);
    // check if it is a sentinel 2 product, otherwise -> exception
    if (meta != nullptr) {
        if (meta->Header.FixedHeader.Mission.find("SENTINEL") == std::string::npos) {
            itkExceptionMacro("Mission is not a SENTINEL !");
        }
    }
    else
        itkExceptionMacro("Mission is not a SENTINEL !");

    std::string imageFile1 = getMACCSRasterFileName(m_DirName, (*meta).ProductOrganization.ImageFiles, "_FRE_R1");
    if(imageFile1.length() <= 0)
        itkExceptionMacro("Couldn't get the FRE_R1 file name !");
    m_InImage = ImageReaderType::New();
    m_InImage->SetFileName(imageFile1);
    m_InImage->UpdateOutputInformation();

    //inImage->UpdateOutputInformation();
    m_ChannelExtractorList = ExtractROIFilterListType::New();
    m_Filter               = BMFilterType::New();

    m_ImageList = VectorImageToImageListType::New();

    m_ImageList->SetInput(m_InImage->GetOutput());
    m_ImageList->UpdateOutputInformation();
    if(m_InImage->GetOutput()->GetNumberOfComponentsPerPixel() < 4)
        itkExceptionMacro("The image has less than 4 bands, which is not acceptable for a SENTINEL-S2 product with resolution 10 meters !");

    unsigned int j = 0;
    for (j = 0; j < m_InImage->GetOutput()->GetNumberOfComponentsPerPixel(); j++)
        m_Filter->SetNthInput(j, m_ImageList->GetOutput()->GetNthElement(j));

    // The significance of the bands is:
    // b1 - G
    // b2 - R
    // b3 - NIR
    // b4 - SWIR
    std::string ndviExpr;
#ifdef OTB_MUPARSER_HAS_CXX_LOGICAL_OPERATORS
    ndviExpr = "(b3==-10000 || b2==-10000) ? -10000 : (abs(b3+b2)<0.000001) ? 0 : 10000 * (b3-b2)/(b3+b2)";
#else        
    ndviExpr = "if(b3==-10000 or b2==-10000,-10000,if(abs(b3+b2)<0.000001,0,(b3-b2)/(b3+b2)";
#endif

    m_Filter->SetExpression(ndviExpr);

    return m_Filter->GetOutput();
}

std::string ComputeNDVI::getMACCSRasterFileName(const std::string& rootFolder,
                                                const std::vector<MACCSFileInformation>& imageFiles,
                                                const std::string& ending) {

    for (const MACCSFileInformation& fileInfo : imageFiles) {
        if (fileInfo.LogicalName.length() >= ending.length() &&
                0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
            return rootFolder + fileInfo.FileLocation.substr(0, fileInfo.FileLocation.find_last_of('.')) + ".DBL.TIF";
        }

    }
    return "";
}


