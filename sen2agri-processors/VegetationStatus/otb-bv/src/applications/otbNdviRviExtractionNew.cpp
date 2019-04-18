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
#include "otbVectorImage.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbStreamingResampleImageFilter.h"
#include "MetadataHelperFactory.h"
#include "ImageResampler.h"

//Transform
#include "itkScalableAffineTransform.h"
#include "GlobalDefs.h"
#include "itkBinaryFunctorImageFilter.h"
#include "CommonFunctions.h"

typedef float                                                             PixelType;
typedef otb::VectorImage<PixelType, 2>                                    ImageType;
typedef short                                                             ShortPixelType;
typedef otb::Image<ShortPixelType, 2>                                     ShortImageType;
typedef otb::VectorImage<PixelType, 2>                                    OutImageType;
typedef otb::Image<PixelType, 2>                                          InternalImageType;
typedef otb::ImageFileReader<ImageType>                                   ReaderType;
typedef otb::ImageList<InternalImageType>                                 ImageListType;

typedef otb::VectorImageToImageListFilter<ImageType, ImageListType>       VectorImageToImageListType;
typedef otb::ImageListToVectorImageFilter<ImageListType, ImageType>       ImageListToVectorImageFilterType;

typedef otb::StreamingResampleImageFilter<InternalImageType, InternalImageType, double>     ResampleFilterType;

namespace otb
{
namespace Wrapper
{
class NdviRviExtractionNew : public Application
{
    typedef itk::UnaryFunctorImageFilter<ImageType,ImageType,
                    NdviRviFunctor<
                        ImageType::PixelType,
                        ImageType::PixelType> > UnmaskedNDVIRVIFilterType;

    typedef itk::BinaryFunctorImageFilter<ImageType,ImageType,ImageType,
                    NdviRviFunctor<
                        ImageType::PixelType, ImageType::PixelType,
                        ImageType::PixelType> > MaskedNDVIRVIFilterType;


    typedef itk::UnaryFunctorImageFilter<ImageType,ImageType,
                    ShortToFloatTranslationFunctor<
                        ImageType::PixelType,
                        ImageType::PixelType> > ReflTransFilterType;
    typedef itk::UnaryFunctorImageFilter<InternalImageType,ShortImageType,
                    FloatToShortTranslationFunctor<
                        InternalImageType::PixelType,
                        ShortImageType::PixelType> > FloatToShortTransFilterType;

public:
    typedef NdviRviExtractionNew Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)
    itkTypeMacro(NdviRviExtractionApp2, otb::Application)

private:

    void DoInit()
    {
        SetName("NdviRviExtractionNew");
        SetDescription("The feature extraction step produces the NDVI and RVI for a product.");

        SetDocName("NdviRviExtractionNew");
        SetDocLongDescription("The feature extraction step produces the NDVI and RVI from a product.");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");

        AddParameter(ParameterType_String, "xml", "Product Metadata XML File");
        AddParameter(ParameterType_Int, "outres", "Output resolution. If not specified, is the same as the input resolution.");
        MandatoryOff("outres");

        AddParameter(ParameterType_InputImage, "msks", "Masks flags used for masking final NDVI/RVI values");
        MandatoryOff("msks");

        AddParameter(ParameterType_InputFilename, "laicfgs",
                     "Master file containing the LAI configuration files for each mission. Used when fts parameter is set.");
        SetParameterDescription( "laicfgs", "Master file containing the LAI configuration files for each mission. Used when fts parameter is set." );
        MandatoryOff("laicfgs");

        AddParameter(ParameterType_InputFilename, "laicfg",
                     "File containing the bands indexes to be used for the LAI. Used when fts parameter is set.");
        SetParameterDescription( "laicfg", "File containing the bands indexes to be used for the LAI. Used when fts parameter is set." );
        MandatoryOff("laicfg");

        AddParameter(ParameterType_OutputImage, "ndvi", "NDVI image");
        MandatoryOff("ndvi");
        AddParameter(ParameterType_OutputImage, "rvi", "RVI image");
        MandatoryOff("rvi");
        AddParameter(ParameterType_OutputImage, "fts", "Features image containing NDVI and RVI bands");
        MandatoryOff("fts");

        SetDocExampleParameterValue("xml", "data.xml");
        SetDocExampleParameterValue("ndvi", "ndvi.tif");
        SetDocExampleParameterValue("rvi", "rvi.tif");
        SetDocExampleParameterValue("fts", "ndv_rvi.tif");
    }

    void DoUpdateParameters()
    {
    }
    void DoExecute()
    {
        const std::string &inMetadataXml = GetParameterString("xml");
        if (inMetadataXml.empty())
        {
            itkExceptionMacro("No input metadata XML set...; please set the input image");
        }

        auto factory = MetadataHelperFactory::New();
        // we load first the default resolution where we have the RED and NIR
        m_pHelper = factory->GetMetadataHelper<float>(inMetadataXml);
        const std::vector<std::string> &bandsToExtract = {m_pHelper->GetRedBandName(), m_pHelper->GetNirBandName()};
        m_img = m_pHelper->GetImage(bandsToExtract, &m_bandIndexes);
        m_img->UpdateOutputInformation();

        // Read the spacing for the default image
        int curRes = m_img->GetSpacing()[0];

        int nOutRes = curRes;
        if(HasValue("outres")) {
            nOutRes = GetParameterInt("outres");
            if(nOutRes != 10 && nOutRes != 20) {
                itkExceptionMacro("Invalid output resolution specified (only 10 and 20 accepted)" << nOutRes);
            }
        }
        // Compute NDVI and RVI
        computeNdviRvi();

        // Handle, if needed, the creation of the NDVI or RVI export rasters
        handleNdviRviCreation(curRes, nOutRes);

        // Handle, if needed, the features creation (all bands + optionally the NDVI and RVI bands)
        handleFeaturesCreation(m_pHelper, curRes, nOutRes);
    }

    /**
     * Resamples an image according to the given current and desired resolution
     */
    InternalImageType::Pointer getResampledImage(int nCurRes, int nDesiredRes,
                                                    InternalImageType::Pointer inImg) {
        if(nCurRes == nDesiredRes)
            return inImg;
        float fMultiplicationFactor = ((float)nCurRes)/nDesiredRes;
        ResampleFilterType::Pointer resampler = m_Resampler.getResampler(inImg, fMultiplicationFactor);
        return resampler->GetOutput();
    }

    /**
     * Performs the computation of the NDVI and RVI that will be accessible after that
     * using the m_ndviRviImgSplit member
     */
    void computeNdviRvi() {
        // the bands are 1 based
        bool bHasMsks = HasValue("msks");
        if(bHasMsks) {
            m_msksImg = GetParameterFloatVectorImage("msks");
            m_MaskedNdviRviFunctor = MaskedNDVIRVIFilterType::New();
            m_MaskedNdviRviFunctor->GetFunctor().Initialize(m_bandIndexes[0], m_bandIndexes[1]);
            m_MaskedNdviRviFunctor->SetInput1(m_img);
            m_MaskedNdviRviFunctor->SetInput2(m_msksImg);
            m_ndviRviFunctorOutput = m_MaskedNdviRviFunctor->GetOutput();
        } else {
            m_UnmaskedNdviRviFunctor = UnmaskedNDVIRVIFilterType::New();
            m_UnmaskedNdviRviFunctor->GetFunctor().Initialize(m_bandIndexes[0], m_bandIndexes[1]);
            m_UnmaskedNdviRviFunctor->SetInput(m_img);
            m_ndviRviFunctorOutput = m_UnmaskedNdviRviFunctor->GetOutput();
        }

        m_ndviRviImgSplit = VectorImageToImageListType::New();
        m_ndviRviFunctorOutput->UpdateOutputInformation();
        m_ndviRviImgSplit->SetInput(m_ndviRviFunctorOutput);
        m_ndviRviImgSplit->UpdateOutputInformation();
        m_ndviRviImgSplit->GetOutput()->UpdateOutputInformation();
    }

    /**
     * Handles the creation of the NDVI and RVI outputs (if configured) by extracting these
     * indices in their own rasters
     */
    void handleNdviRviCreation(int nCurRes, int nOutRes) {
        bool bOutNdvi = HasValue("ndvi");
        bool bOutRvi = HasValue("rvi");

        // export the NDVI in a distinct raster if we have this option set
        if(bOutNdvi) {
            m_floatToShortNdviFunctor = FloatToShortTransFilterType::New();
            // quantify the image using the default factor and considering 0 as NO_DATA but
            // also setting all values less than 0 to 0
            m_floatToShortNdviFunctor->GetFunctor().Initialize(DEFAULT_QUANTIFICATION_VALUE, NO_DATA_VALUE, true);
            m_floatToShortNdviFunctor->SetInput(getResampledImage(nCurRes, nOutRes,
                                m_ndviRviImgSplit->GetOutput()->GetNthElement(0)).GetPointer());
            m_floatToShortNdviFunctor->GetOutput()->SetNumberOfComponentsPerPixel(1);
            SetParameterOutputImage("ndvi", m_floatToShortNdviFunctor->GetOutput());
            SetParameterOutputImagePixelType("ndvi", ImagePixelType_int16);
        }

        // export the RVI in a distinct raster if we have this option set
        if(bOutRvi) {
            //SetParameterOutputImagePixelType("rvi", ImagePixelType_int16);
            SetParameterOutputImage("rvi", getResampledImage(nCurRes, nOutRes, m_ndviRviImgSplit->GetOutput()->GetNthElement(1)).GetPointer());
        }
    }

    /**
     * Handles the creation of the features output (if configured) by adding the configured
     * bands and the NDVI and RVI if configured in LAI bands configuration file
     */
    void handleFeaturesCreation(const std::unique_ptr<MetadataHelper<float>> &pHelper, int nCurRes, int nOutRes) {
        bool bOutFts = HasValue("fts");
        if(bOutFts) {
            // Load the LAI bands configuration file
            bool bHasLaiCfgs = HasValue("laicfgs");
            std::string laiCfgFile;
            if(bHasLaiCfgs) {
                const std::string &laiCfgsFile = GetParameterString("laicfgs");
                laiCfgFile = getValueFromMissionsCfgFile(laiCfgsFile, pHelper->GetMissionName(), pHelper->GetInstrumentName());
            } else {
                bool bHasLaiCfg = HasValue("laicfg");
                if (!bHasLaiCfg) {
                    itkExceptionMacro("Either laicfgs or laicfg should be configured when fts is present");
                }
                laiCfgFile = GetParameterString("laicfg");
            }
            LAIBandsConfigInfos laiCfg = loadLaiBandsConfigFile(laiCfgFile);

            std::cout << "=================================" << std::endl;
            std::cout << "addndvi : " << laiCfg.bUseNdvi           << std::endl;
            std::cout << "addrvi : " << laiCfg.bUseRvi             << std::endl;
            std::cout << "Bands used : ";
            for (const auto &bandName: laiCfg.bandsNames) {
                std::cout << bandName << ' ';
            }
            std::cout << std::endl;
            std::cout << "=================================" << std::endl;

            double fQuantifVal = pHelper->GetReflectanceQuantificationValue();
            allList = ImageListType::New();

            std::vector<int> relBandIdxs;
            MetadataHelper<float>::VectorImageType::Pointer img = pHelper->GetImage(laiCfg.bandsNames, &relBandIdxs, nOutRes);
            img->UpdateOutputInformation();
            int curRes = img->GetSpacing()[0];
            int nInputBands = img->GetNumberOfComponentsPerPixel();

            ReflTransFilterType::Pointer reflTransFunctor = ReflTransFilterType::New();
            reflTransFunctor->GetFunctor().Initialize((float)fQuantifVal);
            reflTransFunctor->SetInput(img);
            reflTransFunctor->GetOutput()->SetNumberOfComponentsPerPixel(nInputBands);

            VectorImageToImageListType::Pointer imgInputSplit = VectorImageToImageListType::New();
            imgInputSplit->SetInput(reflTransFunctor->GetOutput());
            imgInputSplit->UpdateOutputInformation();
            imgInputSplit->GetOutput()->UpdateOutputInformation();

            // add the new translator functor and the immage splitter into their maps
            m_reflTransFunctors.push_back(reflTransFunctor);
            m_imgInputSplitters.push_back(imgInputSplit);

            for (size_t i = 0; i<laiCfg.bandsNames.size(); i++) {
                allList->PushBack(getResampledImage(curRes, nOutRes, imgInputSplit->GetOutput()->GetNthElement(relBandIdxs[i])));
            }

            // add the bands for NDVI and RVI
            if(laiCfg.bUseNdvi) {
                std::cout << "Adding NDVI band ..." << std::endl;
                allList->PushBack(getResampledImage(nCurRes, nOutRes, m_ndviRviImgSplit->GetOutput()->GetNthElement(0)));
            }
            if(laiCfg.bUseRvi) {
                std::cout << "Adding RVI band ..." << std::endl;
                allList->PushBack(getResampledImage(nCurRes, nOutRes, m_ndviRviImgSplit->GetOutput()->GetNthElement(1)));
            }
            m_ftsConcat = ImageListToVectorImageFilterType::New();
            m_ftsConcat->SetInput(allList);
            SetParameterOutputImage("fts", m_ftsConcat->GetOutput());
        }
    }

    VectorImageToImageListType::Pointer       m_ndviRviImgSplit;
    ImageListToVectorImageFilterType::Pointer m_ftsConcat;

    ImageListType::Pointer allList;

    OutImageType::Pointer m_ndviRviFunctorOutput;
    UnmaskedNDVIRVIFilterType::Pointer m_UnmaskedNdviRviFunctor;
    MaskedNDVIRVIFilterType::Pointer m_MaskedNdviRviFunctor;

    FloatToShortTransFilterType::Pointer  m_floatToShortNdviFunctor;

    std::unique_ptr<MetadataHelper<float>>             m_pHelper;
    MetadataHelper<float>::VectorImageType::Pointer    m_img;
    std::vector<int>                            m_bandIndexes;

    //ReaderType::Pointer                       m_defImgReader;
    ImageResampler<InternalImageType, InternalImageType> m_Resampler;

    ImageType::Pointer m_msksImg;
    std::vector<ReflTransFilterType::Pointer> m_reflTransFunctors;
    std::vector<VectorImageToImageListType::Pointer> m_imgInputSplitters;

    //std::map<int, ReaderType::Pointer> m_imgInputReadersMap;
//    std::map<int, std::unique_ptr<MetadataHelper<float>>> m_metadataHelpersMap;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::NdviRviExtractionNew)


