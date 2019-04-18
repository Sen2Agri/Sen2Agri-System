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
 
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbBandMathImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"

#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkCastImageFilter.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkIndent.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkBinaryFunctorImageFilter.h"
#include "itkVariableLengthVector.h"
#include <vector>
#include "DirectionalCorrectionFunctor.h"
#include "MetadataHelperFactory.h"

namespace otb
{

namespace Wrapper
{
class DirectionalCorrection : public Application
{
public:
    typedef DirectionalCorrection Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(DirectionalCorrection, otb::Application)

    typedef float                                   PixelType;
    typedef short                                   PixelShortType;
    typedef FloatVectorImageType                    InputImageType;
    typedef FloatImageType                          InternalBandImageType;
    typedef Int16VectorImageType                    OutImageType;

    typedef otb::ImageList<InternalBandImageType>  ImageListType;
    typedef ImageListToVectorImageFilter<ImageListType, InputImageType >    ListConcatenerFilterType;
    typedef MultiToMonoChannelExtractROI<InputImageType::InternalPixelType,
                                         InternalBandImageType::PixelType>         ExtractROIFilterType;
    typedef ObjectList<ExtractROIFilterType>                                ExtractROIFilterListType;

    typedef DirectionalCorrectionFunctor <InputImageType::PixelType,
                                    OutImageType::PixelType>                DirectionalCorrectionFunctorType;
    typedef itk::UnaryFunctorImageFilter< InputImageType,
                                          OutImageType,
                                          DirectionalCorrectionFunctorType >      FunctorFilterType;

    typedef otb::ImageFileReader<InputImageType> ReaderType;

private:

    void DoInit()
    {
        SetName("DirectionalCorrection");
        SetDescription("Update synthesis using the recurrent expression of the weighted average.");

        SetDocName("DirectionalCorrection");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");
        AddDocTag(Tags::Vector);

        AddParameter(ParameterType_Int, "res", "The resolution to be processed");
        SetDefaultParameterInt("res", -1);
        MandatoryOff("res");
        AddParameter(ParameterType_InputFilename, "xml", "Input general L2A XML");
        AddParameter(ParameterType_InputFilename, "scatcoef", "File containing coefficients for scattering function");
        AddParameter(ParameterType_InputImage, "csm", "Cloud-Shadow Mask");
        AddParameter(ParameterType_InputImage, "wm", "Water Mask");
        AddParameter(ParameterType_InputImage, "sm", "Snow Mask");

        AddParameter(ParameterType_InputImage, "angles", "The angles for solar and sensor");
        AddParameter(ParameterType_InputImage, "ndvi", "The computed NDVI");

        AddParameter(ParameterType_OutputImage, "out", "Out image containing reflectances with directional correction.");

        m_ImageList = ImageListType::New();
        m_Concat = ListConcatenerFilterType::New();
        m_ExtractorList = ExtractROIFilterListType::New();
    }

    void DoUpdateParameters()
    {
      // Nothing to do.
    }

    void DoExecute()
    {

        int resolution = GetParameterInt("res");
        std::string inXml = GetParameterAsString("xml");
        std::string scatCoeffsFile = GetParameterAsString("scatcoef");
        m_AnglesImg = GetParameterFloatVectorImage("angles");
        m_NdviImg = GetParameterFloatVectorImage("ndvi");

        m_CSM = GetParameterFloatVectorImage("csm");
        m_WM = GetParameterFloatVectorImage("wm");
        m_SM = GetParameterFloatVectorImage("sm");

        auto factory = MetadataHelperFactory::New();
        m_pHelper = factory->GetMetadataHelper<float>(inXml);
        float fReflQuantifValue = m_pHelper->GetReflectanceQuantificationValue();
        const std::vector<std::string> &bandNames = m_pHelper->GetBandNamesForResolution(resolution);
        // extract the bands
        m_pHelper->GetImageList(bandNames, m_ImageList, resolution);
        extractBandsFromImage(m_CSM);
        extractBandsFromImage(m_WM);
        extractBandsFromImage(m_SM);
        extractBandsFromImage(m_NdviImg);
        extractBandsFromImage(m_AnglesImg);

        m_Concat->SetInput(m_ImageList);

        // TODO: read these coefficients from the parameters (4 for each band!!!)
        std::vector<ScaterringFunctionCoefficients> scatteringCoeffs;
        scatteringCoeffs = loadScatteringFunctionCoeffs(scatCoeffsFile);

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
        SetParameterOutputImagePixelType("out", ImagePixelType_int16);
        SetParameterOutputImage("out", m_DirectionalCorrectionFunctor->GetOutput());

        return;
    }

    int extractBandsFromImage(InputImageType::Pointer & imageType) {
        int nbBands = imageType->GetNumberOfComponentsPerPixel();
        for(int j=0; j < nbBands; j++)
        {
            ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
            extractor->SetInput( imageType );
            extractor->SetChannel( j+1 );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );
            m_ImageList->PushBack( extractor->GetOutput() );
        }
        return nbBands;
    }

    std::vector<ScaterringFunctionCoefficients> loadScatteringFunctionCoeffs(std::string &strFileName) {
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

    std::string trim(std::string const& str)
    {
        if(str.empty())
            return str;

        std::size_t firstScan = str.find_first_not_of(' ');
        std::size_t first     = firstScan == std::string::npos ? str.length() : firstScan;
        std::size_t last      = str.find_last_not_of(' ');
        return str.substr(first, last-first+1);
    }

    InputImageType::Pointer             m_L2AIn;
    InputImageType::Pointer             m_AnglesImg, m_NdviImg;
    InputImageType::Pointer             m_CSM, m_WM, m_SM;
    ImageListType::Pointer              m_ImageList;
    ListConcatenerFilterType::Pointer   m_Concat;
    ExtractROIFilterListType::Pointer   m_ExtractorList;
    FunctorFilterType::Pointer          m_DirectionalCorrectionFunctor;
    DirectionalCorrectionFunctorType          m_Functor;

    std::unique_ptr<MetadataHelper<float>> m_pHelper;
};

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::DirectionalCorrection)


