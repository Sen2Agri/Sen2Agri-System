#include "DirectionalCorrection.h"
#include <vector>


DirectionalCorrection::DirectionalCorrection()
{
    m_inputImageReader = ReaderType::New();
}

void DirectionalCorrection::Init(int res, std::string &xml, std::string &scatcoef, std::string &strMaskFileName,
                                 InputImageType1::Pointer &angles, InputImageType2::Pointer &ndvi)
{
    m_nRes = res;
    m_strXml = xml;
    m_strScatCoeffs = scatcoef;
    m_inputImageReader->SetFileName(strMaskFileName);

    m_AnglesImg = angles;
    m_NdviImg = ndvi;

    m_ImageList = ImageListType::New();
    m_Concat = ListConcatenerFilterType::New();
    m_ExtractorList = ExtractROIFilterListType::New();
}

void DirectionalCorrection::DoExecute()
{
    auto factory = MetadataHelperFactory::New();
    auto pHelper = factory->GetMetadataHelper(m_strXml, m_nRes);
    float fReflQuantifValue = pHelper->GetReflectanceQuantificationValue();
    std::string inputImageFile = pHelper->GetImageFileName();
    m_inputImageReader = ReaderType::New();
    m_inputImageReader->SetFileName(inputImageFile);
    m_inputImageReader->UpdateOutputInformation();

    InputImageType1::Pointer inputImg = m_inputImageReader->GetOutput();
    extractBandsFromImage(inputImg);

    // extract the cloud, water and snow masks from the masks file
    m_CSM = m_ResampledBandsExtractor.ExtractResampledBand(m_inputImageReader->GetOutput(), 1);
    m_WM = m_ResampledBandsExtractor.ExtractResampledBand(m_inputImageReader->GetOutput(), 2);
    m_SM = m_ResampledBandsExtractor.ExtractResampledBand(m_inputImageReader->GetOutput(), 3);
    m_ImageList->PushBack(m_CSM);
    m_ImageList->PushBack(m_WM);
    m_ImageList->PushBack(m_SM);
    m_ImageList->PushBack(m_NdviImg);
    extractBandsFromImage(m_AnglesImg);

    m_Concat->SetInput(m_ImageList);

    // TODO: read these coefficients from the parameters (4 for each band!!!)
    std::vector<ScaterringFunctionCoefficients> scatteringCoeffs;
    scatteringCoeffs = loadScatteringFunctionCoeffs(m_strScatCoeffs);

//        otbAppLogINFO( << "Mission name: " << missionName << std::endl );
//        otbAppLogINFO( << "Resolution: " << resolution << std::endl );
//        otbAppLogINFO( << "Exists L3A flag: " << l3aExist << std::endl );
//        otbAppLogINFO( << "All in one flag: " << allInOne << std::endl );
//        otbAppLogINFO( << "Missing L3A bands flag: " << bMissingL3ABands << std::endl );

    m_Functor.Initialize(scatteringCoeffs, fReflQuantifValue);
    m_DirectionalCorrectionFunctor = FunctorFilterType::New();
    m_DirectionalCorrectionFunctor->SetFunctor(m_Functor);
    m_DirectionalCorrectionFunctor->SetInput(m_Concat->GetOutput());
    m_DirectionalCorrectionFunctor->UpdateOutputInformation();
    m_DirectionalCorrectionFunctor->GetOutput()->SetNumberOfComponentsPerPixel(scatteringCoeffs.size());

    // additionally, we export also the AOT
    std::string aotFileName = pHelper->GetAotImageFileName();
    m_aotImageReader = ReaderType::New();
    m_aotImageReader->SetFileName(aotFileName);
    m_aotImageReader->UpdateOutputInformation();
    m_AOT = m_ResampledBandsExtractor.ExtractResampledBand(m_aotImageReader->GetOutput(), 1);
}

int DirectionalCorrection::extractBandsFromImage(InputImageType1::Pointer & imageType) {
    int nbBands = imageType->GetNumberOfComponentsPerPixel();
    for(int j=0; j < nbBands; j++)
    {
        // extract the band without resampling
        m_ImageList->PushBack(m_ResampledBandsExtractor.ExtractResampledBand(imageType, j+1));
    }
    return nbBands;
}

std::vector<ScaterringFunctionCoefficients> DirectionalCorrection::loadScatteringFunctionCoeffs(std::string &strFileName) {
    std::vector<ScaterringFunctionCoefficients> scatteringCoeffs;

    std::ifstream coeffsFile;
    coeffsFile.open(strFileName);
    if (!coeffsFile.is_open()) {
        itkExceptionMacro("Can't open dates file for reading!");
    }

    std::string line;
    int curLine = 0;
    while (std::getline(coeffsFile, line))
    {
        trim(line);
        if ((line.size() > 0) && (line[0] != '#') && (line[0] != '\r'))
        {
            ScaterringFunctionCoefficients coeffs;
            std::istringstream iss(line);
            float num; // The number in the line

            //while the iss is a number
            int i = 0;
            while ((iss >> num))
            {
                //look at the number
                switch(i) {
                case 0:
                    coeffs.V0 = num;
                    break;
                case 1:
                    coeffs.V1 = num;
                    break;
                case 2:
                    coeffs.R0 = num;
                    break;
                case 3:
                    coeffs.R1 = num;
                    break;
                default:
                    break;
                }
                i++;
            }
            if(i >= 4) {
                scatteringCoeffs.push_back(coeffs);
            } else {
                itkExceptionMacro("Invalid values line found at position " << curLine);
            }
        }
        curLine++;
    }

    return scatteringCoeffs;
}

std::string DirectionalCorrection::trim(std::string const& str)
{
    if(str.empty())
        return str;

    std::size_t firstScan = str.find_first_not_of(' ');
    std::size_t first     = firstScan == std::string::npos ? str.length() : firstScan;
    std::size_t last      = str.find_last_not_of(' ');
    return str.substr(first, last-first+1);
}


DirectionalCorrection::OutImageType::Pointer DirectionalCorrection::GetResampledMainImg()
{
    return m_DirectionalCorrectionFunctor->GetOutput();
}

DirectionalCorrection::InputImageType2::Pointer DirectionalCorrection::GetResampledCloudMaskImg()
{
    return m_CSM;
}

DirectionalCorrection::InputImageType2::Pointer DirectionalCorrection::GetResampledWaterMaskImg()
{
    return m_WM;
}

DirectionalCorrection::InputImageType2::Pointer DirectionalCorrection::GetResampledSnowMaskImg()
{
    return m_SM;
}

DirectionalCorrection::InputImageType2::Pointer DirectionalCorrection::GetResampledAotImg()
{
    return m_AOT;
}
