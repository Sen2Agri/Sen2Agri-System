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

template< class TInput, class TOutput, class TInput2=TInput>
class NdviRviFunctor
{
public:
    NdviRviFunctor() {}
    ~NdviRviFunctor() {}
    void Initialize(int nRedBandIdx, int nNirBandIdx) {
        m_nRedBandIdx = nRedBandIdx;
        m_nNirBandIdx = nNirBandIdx;
  }

  bool operator!=( const NdviRviFunctor &a) const
  {
      return (this->m_nRedBandIdx != a.m_nRedBandIdx) || (this->m_nNirBandIdx != a.m_nNirBandIdx);
  }
  bool operator==( const NdviRviFunctor & other ) const
  {
    return !(*this != other);
  }
  inline TOutput operator()( const TInput & A ) const
  {
      TOutput ret(2);
      double redVal = A[m_nRedBandIdx];
      double nirVal = A[m_nNirBandIdx];
      if((fabs(redVal - NO_DATA_VALUE) < 0.000001) || (fabs(nirVal - NO_DATA_VALUE) < 0.000001)) {
          // if one of the values is no data, then we set the NDVI and RVI to 0
          ret[0] = 0;
          ret[1] = 0;
      } else {
        if(fabs(redVal + nirVal) < 0.000001) {
            ret[0] = 0;
        } else {
            ret[0] = (nirVal - redVal)/(nirVal+redVal);
        }
        ret[1] = nirVal/redVal;
        // we limit the RVI to a maximum value of 30
        if(ret[1] < 0.000001 || std::isnan(ret[1])) {
            ret[1] = 0;
        } else {
            if(ret[1] > 30 || std::isinf(ret[1])) {
                ret[1] = 30;
            }
        }
      }

      return ret;
  }

  inline TOutput operator()( const TInput & A, const TInput2 & B ) const
  {
        TOutput ret = (*this)(A);
        if(B[0] != IMG_FLG_LAND) {
            ret[0] = 0;
            ret[1] = 0;
        }
        return ret;
  }

private:
  int m_nRedBandIdx;
  int m_nNirBandIdx;
};

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
        auto pHelper = factory->GetMetadataHelper(inMetadataXml);

        // Read the spacing for the default image
        m_defImgReader = ReaderType::New();
        m_defImgReader->SetFileName(pHelper->GetImageFileName());
        m_defImgReader->UpdateOutputInformation();
        int curRes = m_defImgReader->GetOutput()->GetSpacing()[0];

        int nOutRes = curRes;
        if(HasValue("outres")) {
            nOutRes = GetParameterInt("outres");
            if(nOutRes != 10 && nOutRes != 20) {
                itkExceptionMacro("Invalid output resolution specified (only 10 and 20 accepted)" << nOutRes);
            }
        }
        // Compute NDVI and RVI
        computeNdviRvi(pHelper);

        // Handle, if needed, the creation of the NDVI or RVI export rasters
        handleNdviRviCreation(curRes, nOutRes);

        // Handle, if needed, the features creation (all bands + optionally the NDVI and RVI bands)
        handleFeaturesCreation(pHelper, curRes, nOutRes);
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
     * Extracts the (sub)set of resolutions for the bands configured in the LAI configuration file
     */
    std::vector<int> getResolutionsForConfiguredBands(const LAIBandsConfigInfos &infos, const std::unique_ptr<MetadataHelper> &pHelper) {
        std::vector<int> retRes;
        for(int bandIdx: infos.bandsIdxs) {
            int res = pHelper->GetResolutionForAbsoluteBandIndex(bandIdx);
            if(std::find(retRes.begin(), retRes.end(), res) == retRes.end()) {
                retRes.push_back(res);
            }
        }
        return retRes;
    }

    /**
     * Performs the computation of the NDVI and RVI that will be accessible after that
     * using the m_ndviRviImgSplit member
     */
    void computeNdviRvi(const std::unique_ptr<MetadataHelper> &pHelper) {
        // the bands are 1 based
        int nNirBandIdx = pHelper->GetRelNirBandIndex()-1;
        int nRedBandIdx = pHelper->GetRelRedBandIndex()-1;

        bool bHasMsks = HasValue("msks");
        if(bHasMsks) {
            m_msksImg = GetParameterFloatVectorImage("msks");
            m_MaskedNdviRviFunctor = MaskedNDVIRVIFilterType::New();
            m_MaskedNdviRviFunctor->GetFunctor().Initialize(nRedBandIdx, nNirBandIdx);
            m_MaskedNdviRviFunctor->SetInput1(m_defImgReader->GetOutput());
            m_MaskedNdviRviFunctor->SetInput2(m_msksImg);
            m_ndviRviFunctorOutput = m_MaskedNdviRviFunctor->GetOutput();
        } else {
            m_UnmaskedNdviRviFunctor = UnmaskedNDVIRVIFilterType::New();
            m_UnmaskedNdviRviFunctor->GetFunctor().Initialize(nRedBandIdx, nNirBandIdx);
            m_UnmaskedNdviRviFunctor->SetInput(m_defImgReader->GetOutput());
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
            m_floatToShortNdviFunctor->GetFunctor().Initialize(DEFAULT_QUANTIFICATION_VALUE, 0, true);
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
    void handleFeaturesCreation(const std::unique_ptr<MetadataHelper> &pHelper, int nCurRes, int nOutRes) {
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
            for (std::vector<int>::const_iterator i = laiCfg.bandsIdxs.begin(); i != laiCfg.bandsIdxs.end(); ++i) {
                std::cout << *i << ' ';
            }
            std::cout << std::endl;
            std::cout << "=================================" << std::endl;

            std::vector<int> resolutionsVect = getResolutionsForConfiguredBands(laiCfg, pHelper);
            for (int cfgRes: resolutionsVect) {
                createInputImgSplitForResolution(cfgRes);
            }

            allList = ImageListType::New();
            // Iterate all configured bands taking into account the raster for the resolution the band belongs
            for (int bandIdx: laiCfg.bandsIdxs) {
                int curBandRes = pHelper->GetResolutionForAbsoluteBandIndex(bandIdx);
                // Get the helper for the current resolution
                const auto &pResHelper = m_metadataHelpersMap.at(curBandRes);
                // get the relative band index in the product raster for the current band resolution
                // The relative band index is 1 based value so we must extract the 0 based one
                int relBandIdx = pResHelper->GetRelativeBandIndex(bandIdx) - 1;
                VectorImageToImageListType::Pointer imgInputSplit = m_imgInputSplittersMap.at(curBandRes);
                allList->PushBack(getResampledImage(curBandRes, nOutRes, imgInputSplit->GetOutput()->GetNthElement(relBandIdx)));
            }

            // add the bands for NDVI and RVI
            if(laiCfg.bUseNdvi) {
                allList->PushBack(getResampledImage(nCurRes, nOutRes, m_ndviRviImgSplit->GetOutput()->GetNthElement(0)));
            }
            if(laiCfg.bUseRvi) {
                allList->PushBack(getResampledImage(nCurRes, nOutRes, m_ndviRviImgSplit->GetOutput()->GetNthElement(1)));
            }
            m_ftsConcat = ImageListToVectorImageFilterType::New();
            m_ftsConcat->SetInput(allList);
            SetParameterOutputImage("fts", m_ftsConcat->GetOutput());
        }
    }

    void createInputImgSplitForResolution(int res) {
        std::map<int,VectorImageToImageListType::Pointer>::iterator it;
        it = m_imgInputSplittersMap.find(res);
        if (it == m_imgInputSplittersMap.end()) {
            // get the XML parameter
            const std::string &inMetadataXml = GetParameterString("xml");
            // Create a new product helper for the current resolution
            auto factory = MetadataHelperFactory::New();
            // create the helper directly into the map
            m_metadataHelpersMap[res] = factory->GetMetadataHelper(inMetadataXml, res);
            const auto &pHelper = m_metadataHelpersMap[res];

            // create the reader for the image at this resolution
            ReaderType::Pointer imgReader = ReaderType::New();
            imgReader->SetFileName(pHelper->GetImageFileName());
            imgReader->UpdateOutputInformation();

            // Translate the input image from short reflectance values to float (subunitaire) values
            // translated using the quantification value
            double fQuantifVal = pHelper->GetReflectanceQuantificationValue();

            ReflTransFilterType::Pointer reflTransFunctor = ReflTransFilterType::New();
            reflTransFunctor->GetFunctor().Initialize((float)fQuantifVal);
            reflTransFunctor->SetInput(imgReader->GetOutput());
            int nInputBands = imgReader->GetOutput()->GetNumberOfComponentsPerPixel();
            reflTransFunctor->GetOutput()->SetNumberOfComponentsPerPixel(nInputBands);

            VectorImageToImageListType::Pointer imgInputSplit = VectorImageToImageListType::New();
            imgInputSplit->SetInput(reflTransFunctor->GetOutput());
            imgInputSplit->UpdateOutputInformation();
            imgInputSplit->GetOutput()->UpdateOutputInformation();

            // add the new translator functor and the immage splitter into their maps
            m_imgInputReadersMap[res] = imgReader;
            m_reflTransFunctorsMap[res] = reflTransFunctor;
            m_imgInputSplittersMap[res] = imgInputSplit;
        }
    }

    VectorImageToImageListType::Pointer       m_ndviRviImgSplit;
    ImageListToVectorImageFilterType::Pointer m_ftsConcat;

    ImageListType::Pointer allList;

    OutImageType::Pointer m_ndviRviFunctorOutput;
    UnmaskedNDVIRVIFilterType::Pointer m_UnmaskedNdviRviFunctor;
    MaskedNDVIRVIFilterType::Pointer m_MaskedNdviRviFunctor;

    FloatToShortTransFilterType::Pointer  m_floatToShortNdviFunctor;

    ReaderType::Pointer                       m_defImgReader;
    ImageResampler<InternalImageType, InternalImageType> m_Resampler;

    ImageType::Pointer m_msksImg;

    std::map<int, ReaderType::Pointer> m_imgInputReadersMap;
    std::map<int, std::unique_ptr<MetadataHelper>> m_metadataHelpersMap;
    std::map<int, ReflTransFilterType::Pointer> m_reflTransFunctorsMap;
    std::map<int, VectorImageToImageListType::Pointer> m_imgInputSplittersMap;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::NdviRviExtractionNew)


