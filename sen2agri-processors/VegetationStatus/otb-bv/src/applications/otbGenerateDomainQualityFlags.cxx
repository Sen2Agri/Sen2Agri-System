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
#include <boost/algorithm/string.hpp>

//Transform
#include "itkScalableAffineTransform.h"
#include "GlobalDefs.h"
#include "itkBinaryFunctorImageFilter.h"
#include "itkTernaryFunctorImageFilter.h"
#include "CommonFunctions.h"


namespace otb
{
namespace Wrapper
{
class GenerateDomainQualityFlags : public Application
{
    typedef short                                                             ShortPixelType;
    typedef float                                                             FloatPixelType;
    typedef otb::VectorImage<FloatPixelType, 2>                               FloatVectorImageType;
    typedef otb::Image<ShortPixelType, 2>                                     ShortImageType;
    typedef otb::ImageFileReader<FloatVectorImageType>                        FloatVectorImageReaderType;

    typedef otb::Image<FloatPixelType, 2>                                     FloatImageType;
    typedef otb::ImageList<FloatImageType>                                    FloatImageListType;

    typedef otb::StreamingResampleImageFilter<ShortImageType, ShortImageType, double>         ResampleShortImageFilterType;
    typedef otb::StreamingResampleImageFilter<FloatImageType, FloatImageType, double>         ResampleFloatImageFilterType;
    typedef otb::StreamingResampleImageFilter<FloatVectorImageType, FloatVectorImageType, double> ResampleFloatVectorImageFilterType;

    typedef otb::VectorImageToImageListFilter<FloatVectorImageType, FloatImageListType>       VectorImageToImageListType;

    typedef struct {
        double minVal;
        double maxVal;
        double tolerance;
        bool   isEnabled;
    } DomainLimits_Type;

    std::map<std::string, DomainLimits_Type> S2_VALIDATION_LIMITS = {
                                                                 {"B1", {0.0, 1.0, 0.0}},      //B1 - not used
                                                                 {"B2", {0.0, 1.0, 0.0}},      //B2 - not used
                                                                 {"B3", {0.0, 0.242, 0.0}},    //B3
                                                                 {"B4", {0.0, 0.292, 0.0}},    //B4
                                                                 {"B5", {0.0, 0.335, 0.0}},    //B5
                                                                 {"B6", {0.0, 0.608, 0.0}},    //B6
                                                                 {"B7", {0.0, 0.759, 0.0}},    //B7
                                                                 {"B8A", {0.0, 0.776, 0.0}},   //B8a
                                                                 {"B8", {0.0, 0.790, 0.0}},    //B8
                                                                 {"B9", {0.0, 1.0, 0.0}},      //B9 - not used
                                                                 {"B10", {0.0, 1.0, 0.0}},     //B10 - not used
                                                                 {"B11", {0.0, 0.495, 0.0}},   //B11
                                                                 {"B12", {0.0, 0.494, 0.0}}};  //B12

    std::map<std::string, DomainLimits_Type> L8_VALIDATION_LIMITS = {
                                                                 {"B3", {0.0, 0.3, 0.0}},      //B3
                                                                 {"B4", {0.0, 0.266, 0.0}},    //B4
                                                                 {"B5", {0.0, 0.788, 0.0}},   //B8a
                                                                 {"B6", {0.0, 0.476, 0.0}},   //B11
                                                                 {"B7", {0.0, 0.504, 0.0}}};  //B12

    std::vector<DomainLimits_Type> LAI_VALIDATION_LIMITS = {{0.0, 8.0, 0.2, true}};
    std::vector<DomainLimits_Type> FAPAR_VALIDATION_LIMITS = {{0.0, 0.94, 0.1, true}};
    std::vector<DomainLimits_Type> FCOVER_VALIDATION_LIMITS = {{0.0, 1.0, 0.1, true}};

    template< class TInput, class TInput2, class TOutput>
    class MaskingFunctor
    {
    public:
        MaskingFunctor() {}
        ~MaskingFunctor() {}

        void SetValidationLimits(const std::vector<DomainLimits_Type> &limits)
        {
            domainLimits = limits;
            // precompute these values to avoid computing them for every pixel
            minToleranceVal = limits[0].minVal - limits[0].tolerance;
            maxToleranceVal = limits[0].maxVal + limits[0].tolerance;
        }

        MaskingFunctor& operator =(const MaskingFunctor& copy)
        {
            domainLimits = copy.domainLimits;
            minToleranceVal = copy.minToleranceVal;
            maxToleranceVal = copy.maxToleranceVal;
            return *this;
        }

        bool operator!=( const MaskingFunctor &a) const
        {
            UNUSED(a);
            return true;
        }
        bool operator==( const MaskingFunctor & other ) const
        {
            return !(*this != other);
        }

        inline TOutput operator()( const TInput & A, const TInput2 & B ) const
        {
            TOutput ret(1);
            ret[0] = A[0];

            if (ret[0] < minToleranceVal) {
                ret[0] = domainLimits[0].minVal;
            } else if (ret[0] > maxToleranceVal) {
                ret[0] = domainLimits[0].maxVal;
            }
            switch (B) {
                case IMG_FLG_NO_DATA:
                case IMG_FLG_CLOUD:
                case IMG_FLG_SNOW:
                    ret[0] = NO_DATA_VALUE;
            default:
                break;
            }
            return ret;
        }

    private:
        std::vector<DomainLimits_Type> domainLimits;
        double minToleranceVal;
        double maxToleranceVal;
    };

    template< class TInput, class TInputMsk, class TOutput>
    class DomainMaskExtractorFunctor
    {
    public:
        DomainMaskExtractorFunctor() {}
        ~DomainMaskExtractorFunctor() {}

        void SetValidationLimits(const std::vector<DomainLimits_Type> &limits)
        {
            domainLimits = limits;
        }

        DomainMaskExtractorFunctor& operator =(const DomainMaskExtractorFunctor& copy)
        {
            domainLimits = copy.domainLimits;
            return *this;
        }

        bool operator!=( const DomainMaskExtractorFunctor &a) const
        {
            UNUSED(a);
            return true;
        }
        bool operator==( const DomainMaskExtractorFunctor & other ) const
        {
            return !(*this != other);
        }

        inline TOutput operator()( const TInput & A, const TInputMsk &msk) const
        {
            TOutput ret(1);
            ret[0] = 0;
            for (int i = 0; i<domainLimits.size(); i++) {
                if (domainLimits[i].isEnabled) {
                    if ((msk == IMG_FLG_NO_DATA) || (A[i] < domainLimits[i].minVal) || (A[i] > domainLimits[i].maxVal)) {
                        ret[0] = 1;
                        //printValue(A, i);
                        break;
                    }
                }
            }

            return ret;
        }

        inline void printValue(const TInput & A, int band) const {
            if (A[band] != NO_DATA_VALUE) {
                std::ostringstream stringStream;
                if (A[band] < domainLimits[band].minVal) {
                    stringStream << "Band " << band << " value " << A[band] << " less than " << domainLimits[band].minVal << std::endl;
                    std::cout << stringStream.str();
                } else if(A[band] > domainLimits[band].maxVal) {
                    stringStream << "Band " << band << " value " << A[band] << " greater than " << domainLimits[band].maxVal << std::endl;
                    std::cout << stringStream.str();
                }
            }
        }

    private:
        std::vector<DomainLimits_Type>  domainLimits;
        int                             quantifValue;
    };

    template< class TInput, class TInputMsk, class TInput2, class TOutput>
    class DomainMaskExtractorBinaryFunctor : public DomainMaskExtractorFunctor<TInput, TInputMsk, TOutput>
    {
    public:
        DomainMaskExtractorBinaryFunctor() {}
        ~DomainMaskExtractorBinaryFunctor() {}
        void SetValidationLimits2(const std::vector<DomainLimits_Type> &limits)
        {
            domainLimits2 = limits;
        }

        DomainMaskExtractorBinaryFunctor& operator =(const DomainMaskExtractorBinaryFunctor& copy)
        {
            DomainMaskExtractorFunctor<TInput, TInputMsk, TOutput>::operator = (copy);
            domainLimits2 = copy.domainLimits2;
            return *this;
        }
        inline TOutput operator()( const TInput & A, const TInputMsk &msk, const TInput2 & B ) const
        {
              TOutput ret = DomainMaskExtractorFunctor<TInput, TInputMsk, TOutput>::operator ()(A, msk);
              if(ret[0] == 0) {
                  for (int i = 0; i<domainLimits2.size(); i++) {
                      if (domainLimits2[i].isEnabled) {
                          if ((msk == IMG_FLG_NO_DATA) || (B[i] < domainLimits2[i].minVal) || (B[i] > domainLimits2[i].maxVal)) {
                              ret[0] = 1;
                              break;
                          }
                      }
                  }
              }
              return ret;
        }
    private:
        std::vector<DomainLimits_Type> domainLimits2;

    };

    typedef enum {LAI, FAPAR, FCOVER} IndexType;

    typedef MaskingFunctor< FloatVectorImageType::PixelType,
                    MetadataHelper<float>::SingleBandShortImageType::PixelType,
                    FloatVectorImageType::PixelType>                                MaskingFunctorType;

    typedef DomainMaskExtractorFunctor <FloatVectorImageType::PixelType,
                                    MetadataHelper<float>::SingleBandShortImageType::PixelType,
                                    FloatVectorImageType::PixelType>                DomainMaskExtractorFunctorType;

    typedef DomainMaskExtractorBinaryFunctor<FloatVectorImageType::PixelType,
                                             MetadataHelper<float>::SingleBandShortImageType::PixelType,
                                             FloatVectorImageType::PixelType,
                                             FloatVectorImageType::PixelType>       DomainMaskExtractorBinaryFunctorType;

    typedef itk::BinaryFunctorImageFilter<  FloatVectorImageType,
                                            MetadataHelper<float>::SingleBandShortImageType,
                                            FloatVectorImageType,
                                            MaskingFunctorType >                    MaskedOutputFilterType;

    typedef itk::BinaryFunctorImageFilter<FloatVectorImageType,
                                        MetadataHelper<float>::SingleBandShortImageType,
                                        FloatVectorImageType,
                                        DomainMaskExtractorFunctorType >            DomainMaskExtractorFilterType;

    typedef itk::TernaryFunctorImageFilter<FloatVectorImageType,
                                        MetadataHelper<float>::SingleBandShortImageType,
                                        FloatVectorImageType,
                                        FloatVectorImageType,
                                        DomainMaskExtractorBinaryFunctorType >      DomainMaskExtractorBinaryFilterType;

public:
    typedef GenerateDomainQualityFlags Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)
    itkTypeMacro(GenerateDomainQualityFlags, otb::Application)

private:

    void DoInit()
    {
        SetName("GenerateDomainQualityFlags");
        SetDescription("Extracts domain quality flags from an input product or from a LAI/FAPAR/FCOVER raster.");

        SetDocName("GenerateDomainQualityFlags");
        SetDocLongDescription("Extracts domain quality flags from an input product or from a LAI/FAPAR/FCOVER raster.");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");

        AddParameter(ParameterType_String, "xml", "Product Metadata XML File");
        AddParameter(ParameterType_String, "in", "LAI/FAPAR/FCOVER unmasked raster. If not provided, input domains "
                                                 "are extracted otherwise the output domain flags are extracted");
        MandatoryOff("in");

        AddParameter(ParameterType_InputFilename, "laicfgs",
                     "Master file containing the LAI configuration files for each mission.");
        SetParameterDescription( "laicfgs", "Master file containing the LAI configuration files for each mission." );
        MandatoryOff("laicfgs");

        AddParameter(ParameterType_InputFilename, "laicfg",
                     "File containing the bands indexes to be used for the LAI.");
        SetParameterDescription( "laicfg", "File containing the bands indexes and models to be used for the LAI." );
        MandatoryOff("laicfg");

        AddParameter(ParameterType_String, "indextype", "The type of the in raster (lai, fapar or fcover)");
        MandatoryOff("indextype");

        AddParameter(ParameterType_String,  "outf",   "The extracted domain flags");
        MandatoryOff("outf");

        AddParameter(ParameterType_String,  "out",   "The updated masked output");
        MandatoryOff("out");

        AddParameter(ParameterType_Int,  "outres",   "The output resolution");
        MandatoryOff("outres");

//        AddParameter(ParameterType_String,  "fts",   "The input bands");
//        MandatoryOff("fts");

    }

    void DoUpdateParameters()
    {
    }
    void DoExecute()
    {
        const std::string &inMetadataXml = GetParameterString("xml");
        auto factory = MetadataHelperFactory::New();
        m_pHelper = factory->GetMetadataHelper<float>(inMetadataXml);
        if (HasValue("in")) {
            HandleOutputDomainFlags(m_pHelper);
        } else {
            ExtractInputDomainFlags(m_pHelper);
        }
    }

    void ExtractInputDomainFlags(const std::unique_ptr<MetadataHelper<float>> &pHelper)
    {
        // load the LAI config file
        loadLaiConfiguration(pHelper);

        // compute the domain limits for each band
        const std::string &missionName = pHelper->GetMissionName();
        std::map<std::string, DomainLimits_Type> domLimits = (missionName.find("SENTINEL2") != std::string::npos) ?
                    S2_VALIDATION_LIMITS : L8_VALIDATION_LIMITS;
        std::vector<DomainLimits_Type> resDomLimits(m_laiCfg.bandsNames.size());
        MetadataHelper<float>::VectorImageType::Pointer img = pHelper->GetImage(m_laiCfg.bandsNames);
        img->UpdateOutputInformation();

        int i = 0;
        for (const std::string &bandName: m_laiCfg.bandsNames) {
            std::map<std::string, DomainLimits_Type>::const_iterator mapIt = domLimits.find(bandName);
            if (mapIt != domLimits.end() ) {
                resDomLimits[i] = domLimits[bandName];
                resDomLimits[i].minVal *= pHelper->GetReflectanceQuantificationValue();
                resDomLimits[i].maxVal *= pHelper->GetReflectanceQuantificationValue();
                resDomLimits[i].tolerance *= pHelper->GetReflectanceQuantificationValue();
                resDomLimits[i].isEnabled = true;
            } else {
                resDomLimits[i].isEnabled = false;
            }
            i++;
        }

        //int baseRes = resolutionsVect[0];
        MetadataHelper<float>::SingleBandMasksImageType::Pointer imgMsk = pHelper->GetMasksImage(ALL, false);
        FloatVectorImageType::Pointer maskedImg;
        m_domainMaskExtractorFunctor.SetValidationLimits(resDomLimits);

        m_domainMaskExtractorFilter = DomainMaskExtractorFilterType::New();
        m_domainMaskExtractorFilter->SetFunctor(m_domainMaskExtractorFunctor);
        m_domainMaskExtractorFilter->SetInput1(img);
        m_domainMaskExtractorFilter->SetInput2(imgMsk);
        m_domainMaskExtractorFilter->UpdateOutputInformation();
        // Mask the output according to the flags
        maskedImg = m_domainMaskExtractorFilter->GetOutput();

        WriteImageToOutput(maskedImg, "outf", ImagePixelType_uint8);
    }

    void HandleOutputDomainFlags(const std::unique_ptr<MetadataHelper<float>> &pHelper)
    {
        if (!HasValue("indextype")) {
            itkExceptionMacro("Please provide the parameter <indextype> along with the in parameter!");
        }
        if (!HasValue("out")) {
            itkExceptionMacro("Please provide the parameter <out> along with the in parameter!");
        }

        // extract the flags
        ExtractOutputDomainFlags(pHelper);
        // update the in raster in order to have values inside the right domain
        UpdateOutputDomainValues(pHelper);
    }

    void ExtractOutputDomainFlags(const std::unique_ptr<MetadataHelper<float>> &pHelper) {
        const std::string &rasterPath = GetParameterString("in");
        m_defImgReader = FloatVectorImageReaderType::New();
        m_defImgReader->SetFileName(rasterPath);
        m_defImgReader->UpdateOutputInformation();
        int inRes = m_defImgReader->GetOutput()->GetSpacing()[0];

        MetadataHelper<float>::SingleBandMasksImageType::Pointer imgMsk = pHelper->GetMasksImage(ALL, false);
        imgMsk->UpdateOutputInformation();
        int curMaskRes = imgMsk->GetSpacing()[0];

        const std::vector<DomainLimits_Type> &resDomLimits = GetOutDomainLimits();

        m_domainMaskExtractorFilter = DomainMaskExtractorFilterType::New();
        m_domainMaskExtractorFilter->GetFunctor().SetValidationLimits(resDomLimits);
        m_domainMaskExtractorFilter->SetInput1(m_defImgReader->GetOutput());
        m_domainMaskExtractorFilter->SetInput2(getResampledImage(curMaskRes, inRes, imgMsk));
        m_domainMaskExtractorFilter->UpdateOutputInformation();
        //m_domainMaskExtractorFilter->GetOutput()->SetNumberOfComponentsPerPixel(1);

        WriteImageToOutput(m_domainMaskExtractorFilter->GetOutput(), "outf", ImagePixelType_uint8);
    }

    void UpdateOutputDomainValues(const std::unique_ptr<MetadataHelper<float>> &pHelper) {
        const std::string &rasterPath = GetParameterString("in");
        m_defImgReader2 = FloatVectorImageReaderType::New();
        m_defImgReader2->SetFileName(rasterPath);
        m_defImgReader2->UpdateOutputInformation();
        int inRes = m_defImgReader2->GetOutput()->GetSpacing()[0];

        std::vector<DomainLimits_Type> resDomLimits = GetOutDomainLimits();
        MetadataHelper<float>::SingleBandMasksImageType::Pointer imgMsk = pHelper->GetMasksImage(ALL, false);
        imgMsk->UpdateOutputInformation();
        int curMaskRes = imgMsk->GetSpacing()[0];

        // now mask also the input raster and apply the domain limits
        m_maskedOutputFunctor = MaskedOutputFilterType::New();
        m_maskedOutputFunctor->GetFunctor().SetValidationLimits(resDomLimits);

        m_maskedOutputFunctor->SetInput1(m_defImgReader2->GetOutput());
        m_maskedOutputFunctor->SetInput2(getResampledImage(curMaskRes, inRes, imgMsk));

        WriteImageToOutput(m_maskedOutputFunctor->GetOutput(), "out", ImagePixelType_double);
    }

    void WriteImageToOutput(const FloatVectorImageType::Pointer &img, const std::string &outName, const ImagePixelType &imgPixelType) {
        img->UpdateOutputInformation();

        int curRes = img->GetSpacing()[0];
        int outRes = curRes;
        if (HasValue("outres")) {
            outRes = GetParameterInt("outres");
        }

        m_imgSplit = VectorImageToImageListType::New();
        m_imgSplit->SetInput(img);
        m_imgSplit->UpdateOutputInformation();
        m_imgSplit->GetOutput()->UpdateOutputInformation();


        FloatImageType::Pointer resImg = getResampledImage3(curRes, outRes,
                                    m_imgSplit->GetOutput()->GetNthElement(0));

        const std::string &outImgFileName = GetParameterAsString(outName);
        // write the output
        WriteOutput<FloatImageType>(resImg, outImgFileName, imgPixelType);
    }

    std::vector<DomainLimits_Type> GetOutDomainLimits() {
        const std::string &indexType = GetParameterString("indextype");
        std::vector<DomainLimits_Type> resDomLimits;
        if (boost::iequals(indexType, "lai")) {
            resDomLimits = LAI_VALIDATION_LIMITS;
        } else if (boost::iequals(indexType, "fapar")) {
            resDomLimits = FAPAR_VALIDATION_LIMITS;
        } else if (boost::iequals(indexType, "fcover")) {
            resDomLimits = FCOVER_VALIDATION_LIMITS;
        } else {
            itkExceptionMacro("Please provide a valid value for the indextype parameter (lai/fapar/fcover)");
        }

        return resDomLimits;
    }

    FloatImageType::Pointer getResampledImage3(int nCurRes, int nDesiredRes,
                                                 FloatImageType::Pointer inImg) {
        if(nCurRes == nDesiredRes)
            return inImg;
        float fMultiplicationFactor = ((float)nCurRes)/nDesiredRes;
        ResampleFloatImageFilterType::Pointer resampler = m_FloatImageResampler.getResampler(inImg, fMultiplicationFactor);
        return resampler->GetOutput();
    }

    ShortImageType::Pointer getResampledImage(int nCurRes, int nDesiredRes,
                                                    ShortImageType::Pointer inImg) {
        if(nCurRes == nDesiredRes)
            return inImg;
        float fMultiplicationFactor = ((float)nCurRes)/nDesiredRes;
        ResampleShortImageFilterType::Pointer resampler = m_ShortImageResampler.getResampler(inImg, fMultiplicationFactor,
                                                                         Interpolator_NNeighbor);
        return resampler->GetOutput();
    }

    FloatVectorImageType::Pointer getResampledImage2(int nCurRes, int nDesiredRes,
                                                    FloatVectorImageType::Pointer inImg) {
        if(nCurRes == nDesiredRes)
            return inImg;
        float fMultiplicationFactor = ((float)nCurRes)/nDesiredRes;
        ResampleFloatVectorImageFilterType::Pointer resampler = m_FloatVectorImageResampler.getResampler(inImg, fMultiplicationFactor,
                                                                         Interpolator_NNeighbor );
        return resampler->GetOutput();
    }

    /**
     * Extracts the (sub)set of resolutions for the bands configured in the LAI configuration file
     */
    std::vector<int> getResolutionsForConfiguredBands(const LAIBandsConfigInfos &infos, const std::unique_ptr<MetadataHelper<float>> &pHelper) {
        std::vector<int> retRes;
        for(const std::string &bandName: infos.bandsNames) {
            int res = pHelper->GetResolutionForBand(bandName);
            if(std::find(retRes.begin(), retRes.end(), res) == retRes.end()) {
                retRes.push_back(res);
            }
        }
        return retRes;
    }

    template<typename ImageType>
    void WriteOutput(typename ImageType::Pointer inImg, const std::string &outImg, ImagePixelType imgPixelType) {
        inImg->UpdateOutputInformation();

        // Create an output parameter to write the current output image
        OutputImageParameter::Pointer paramOut = OutputImageParameter::New();
        // Set the filename of the current output image
        paramOut->SetFileName(outImg);
        paramOut->SetValue(inImg);
        paramOut->SetPixelType(imgPixelType);
        // Add the current level to be written
        paramOut->InitializeWriters();
        std::ostringstream osswriter;
        osswriter<< "Writing flags "<< outImg;
        AddProcess(paramOut->GetWriter(), osswriter.str());
        paramOut->Write();
    }

    void loadLaiConfiguration(const std::unique_ptr<MetadataHelper<float>> &pHelper) {
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
        m_laiCfg = loadLaiBandsConfigFile(laiCfgFile);

        std::cout << "=================================" << std::endl;
        std::cout << "Bands used : ";
        for (std::vector<std::string>::const_iterator i = m_laiCfg.bandsNames.begin(); i != m_laiCfg.bandsNames.end(); ++i) {
            std::cout << *i << ' ';
        }
        std::cout << std::endl;
        std::cout << "=================================" << std::endl;
    }


    FloatVectorImageReaderType::Pointer                         m_defImgReader;
    FloatVectorImageReaderType::Pointer                         m_defImgReader2;
    ImageResampler<ShortImageType, ShortImageType>              m_ShortImageResampler;
    ImageResampler<FloatVectorImageType, FloatVectorImageType>  m_FloatVectorImageResampler;
    ImageResampler<FloatImageType, FloatImageType>              m_FloatImageResampler;

    std::map<int, FloatVectorImageReaderType::Pointer>          m_imgInputReadersMap;
    std::map<int, std::unique_ptr<MetadataHelper<float>>>              m_metadataHelpersMap;

    MaskedOutputFilterType::Pointer                             m_maskedOutputFunctor;

    DomainMaskExtractorFunctorType                              m_domainMaskExtractorFunctor;
    DomainMaskExtractorBinaryFunctorType                        m_domainMaskExtractorBinaryFunctor;

    DomainMaskExtractorFilterType::Pointer                      m_domainMaskExtractorFilter;
    DomainMaskExtractorBinaryFilterType::Pointer                m_domainMaskExtractorBinaryFilter;

    LAIBandsConfigInfos m_laiCfg;

    VectorImageToImageListType::Pointer                         m_imgSplit;

    std::unique_ptr<MetadataHelper<float>> m_pHelper;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::GenerateDomainQualityFlags)


