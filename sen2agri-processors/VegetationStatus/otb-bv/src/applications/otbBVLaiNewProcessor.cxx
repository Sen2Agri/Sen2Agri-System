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
#include "belcamApplyTrainedNeuralNetworkFilter.h"

#define ANGLES_GRID_SIZE    23

typedef short                                                             PixelType;
typedef otb::VectorImage<PixelType, 2>                                    ImageType;
typedef float                                                             FloatPixelType;
typedef otb::VectorImage<FloatPixelType, 2>                               OutImageType;
typedef otb::Image<PixelType, 2>                                          InternalImageType;
typedef otb::ImageFileReader<ImageType>                                   ReaderType;
typedef otb::ImageList<InternalImageType>                                 ImageListType;

typedef otb::VectorImageToImageListFilter<ImageType, ImageListType>       VectorImageToImageListType;
typedef otb::ImageListToVectorImageFilter<ImageListType, ImageType>       ImageListToVectorImageFilterType;

typedef otb::StreamingResampleImageFilter<InternalImageType, InternalImageType, double>     ResampleFilterType;


template< class TInput, class TInput2, class TOutput>
class MaskingFunctor
{
public:
    MaskingFunctor() {}
    ~MaskingFunctor() {}

    void SetValidationLimits(double min, double max, double tolerance)
    {
        minVal = min;
        maxVal = max;
        toleranceVal = tolerance;
        minToleranceVal = min-tolerance;
        maxToleranceVal = max + tolerance;
    }

    MaskingFunctor& operator =(const MaskingFunctor& copy)
    {
        minVal = copy.minVal;
        maxVal = copy.maxVal;
        toleranceVal = copy.toleranceVal;
        minToleranceVal = copy.minToleranceVal;
        maxToleranceVal = copy.maxToleranceVal;
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
            ret[0] = minVal;
        } else if (ret[0] > maxToleranceVal) {
            ret[0] = maxVal;
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
    double minVal;
    double maxVal;
    double toleranceVal;
    double minToleranceVal;
    double maxToleranceVal;
};

namespace otb
{
namespace Wrapper
{
class BVLaiNewProcessor : public Application
{
    typedef enum {LAI, FAPAR, FCOVER} IndexType;

    typedef belcamApplyTrainedNeuralNetworkFilter<ImageType, OutImageType> BelcamNeuronFilter;
    typedef Int16VectorImageType                            AnglesImageType;

    typedef itk::BinaryFunctorImageFilter<OutImageType,MetadataHelper<short>::SingleBandImageType,OutImageType,
                    MaskingFunctor<
                        OutImageType::PixelType, MetadataHelper<short>::SingleBandImageType::PixelType,
                        OutImageType::PixelType> > MaskedOutputFilterType;

public:
    typedef BVLaiNewProcessor Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)
    itkTypeMacro(BVLaiNewProcessor, otb::Application)

private:

    void DoInit()
    {
        SetName("BVLaiNewProcessor");
        SetDescription("The feature extraction step produces the NDVI and RVI for a product.");

        SetDocName("BVLaiNewProcessor");
        SetDocLongDescription("The feature extraction step produces the NDVI and RVI from a product.");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");

        AddParameter(ParameterType_String, "xml", "Product Metadata XML File");

        AddParameter(ParameterType_InputImage, "angles", "Angles raster");

        AddParameter(ParameterType_String, "laimodel", "LAI Model file");
        SetParameterDescription( "laimodel", "The LAI model file. If not specified, the one from the lai config file is used." );
        MandatoryOff("laimodel");

        AddParameter(ParameterType_String, "faparmodel", "FAPAR Model file");
        SetParameterDescription( "faparmodel", "The FAPAR model file. If not specified, the one from the lai config file is used." );
        MandatoryOff("faparmodel");

        AddParameter(ParameterType_String, "fcovermodel", "FCOVER Model file");
        SetParameterDescription( "fcovermodel", "The FCOVER model file. If not specified, the one from the lai config file is used." );
        MandatoryOff("fcovermodel");

        AddParameter(ParameterType_String,  "outlai",   "The out image corresponding to the LAI mono date");
        MandatoryOff("outlai");
        AddParameter(ParameterType_String,  "outfapar",   "The out image corresponding to the FAPAR mono date");
        MandatoryOff("outfapar");
        AddParameter(ParameterType_String,  "outfcover",   "The out image corresponding to the FCOVER mono date");
        MandatoryOff("outfcover");

        AddParameter(ParameterType_OutputImage, "fts", "Output features enterring in the neural filter");
        MandatoryOff("fts");

        AddParameter(ParameterType_InputFilename, "laicfgs",
                     "Master file containing the LAI configuration files for each mission.");
        SetParameterDescription( "laicfgs", "Master file containing the LAI configuration files for each mission." );
        MandatoryOff("laicfgs");

        AddParameter(ParameterType_InputFilename, "laicfg",
                     "File containing the bands indexes to be used for the LAI.");
        SetParameterDescription( "laicfg", "File containing the bands indexes and models to be used for the LAI." );
        MandatoryOff("laicfg");

        AddParameter(ParameterType_Int, "outres", "Output resolution. If not specified, is the same as the input resolution.");
        MandatoryOff("outres");

        SetDocExampleParameterValue("xml", "data.xml");
        SetDocExampleParameterValue("laimodel", "laimodel.txt");
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
        m_pHelper = factory->GetMetadataHelper<short>(inMetadataXml);

        int nOutRes = m_pHelper->GetProductResolutions()[0];
        if(HasValue("outres")) {
            nOutRes = GetParameterInt("outres");
            if(nOutRes != 10 && nOutRes != 20) {
                itkExceptionMacro("Invalid output resolution specified (only 10 and 20 accepted)" << nOutRes);
            }
        }

        // load the LAI config file
        loadLaiConfiguration(m_pHelper);

        // Extract the needed bands
        handleFeaturesCreation(m_pHelper, nOutRes);

        if (HasValue("fts")) {
            SetParameterOutputImage("fts", m_ftsConcat->GetOutput());
        }

        if (HasValue("outlai")) {
            ProduceOutput(m_pHelper, LAI);
        }
        if (HasValue("outfapar")) {
            ProduceOutput(m_pHelper, FAPAR);
        }
        if (HasValue("outfcover")) {
            ProduceOutput(m_pHelper, FCOVER);
        }
    }

    void ProduceOutput(const std::unique_ptr<MetadataHelper<short>> &pHelper, IndexType indexType) {
        std::string modelTextFile;
        std::string modelParamName;
        std::string outparamName;
        std::string laiCfgEntry;
        BelcamNeuronFilter::Pointer filter;
        switch (indexType) {
            case LAI:
                modelParamName = "laimodel";
                outparamName = "outlai";
                laiCfgEntry = m_laiCfg.laiModelFilePath;
                m_laiNeuronFilter = BelcamNeuronFilter::New();
                filter = m_laiNeuronFilter;
                break;
            case FAPAR:
                modelParamName = "faparmodel";
                outparamName = "outfapar";
                laiCfgEntry = m_laiCfg.faparModelFilePath;
                m_faparNeuronFilter = BelcamNeuronFilter::New();
                filter = m_faparNeuronFilter;
                break;
            case FCOVER:
                modelParamName = "fcovermodel";
                outparamName = "outfcover";
                laiCfgEntry = m_laiCfg.fcoverModelFilePath;
                m_fcoverNeuronFilter = BelcamNeuronFilter::New();
                filter = m_fcoverNeuronFilter;
                break;
        }
        if (HasValue(modelParamName)) {
            modelTextFile = GetParameterString(modelParamName);
        } else {
            modelTextFile = laiCfgEntry;
        }
        if(modelTextFile == "") {
            itkExceptionMacro("No model file specified neither by parameter model nor in the lai configuration file");
        }

        // execute the model
        OutImageType::Pointer modelOutput = ExecuteModel(m_ftsConcat->GetOutput(), pHelper->GetReflectanceQuantificationValue(),
                                                         filter, modelTextFile);

        const std::string &outImgFileName = GetParameterAsString(outparamName);
        // write the output
        WriteOutput(modelOutput, outImgFileName);
    }

    /**
     * Resamples an image according to the given current and desired resolution
     */
    InternalImageType::Pointer getResampledImage(int nCurRes, int nDesiredRes,
                                                    InternalImageType::Pointer inImg) {
        if(nCurRes == nDesiredRes)
            return inImg;
        float fMultiplicationFactor = ((float)nCurRes)/nDesiredRes;
        ResampleFilterType::Pointer resampler = m_Resampler.getResampler(inImg, fMultiplicationFactor,
                                                                         Interpolator_NNeighbor /* Interpolator_BCO */);
        return resampler->GetOutput();
    }

    MetadataHelper<short>::SingleBandImageType::Pointer getResampledImage2(int nCurRes, int nDesiredRes,
                                                 MetadataHelper<short>::SingleBandImageType::Pointer inImg) {
        if(nCurRes == nDesiredRes)
            return inImg;
        float fMultiplicationFactor = ((float)nCurRes)/nDesiredRes;
        ResampleFilterType::Pointer resampler = m_Resampler2.getResampler(inImg, fMultiplicationFactor, Interpolator_NNeighbor);
        return resampler->GetOutput();
    }


    /**
     * Extracts the (sub)set of resolutions for the bands configured in the LAI configuration file
     */
    std::vector<int> getResolutionsForConfiguredBands(const LAIBandsConfigInfos &infos,
                                                      const std::unique_ptr<MetadataHelper<short>> &pHelper) {
        std::vector<int> retRes;
        for(const std::string &bandName: infos.bandsNames) {
            int res = pHelper->GetResolutionForBand(bandName);
            if(std::find(retRes.begin(), retRes.end(), res) == retRes.end()) {
                retRes.push_back(res);
            }
        }
        return retRes;
    }

    /**
     * Handles the creation of the features output (if configured) by adding the configured
     * bands and the NDVI and RVI if configured in LAI bands configuration file
     */
    void handleFeaturesCreation(const std::unique_ptr<MetadataHelper<short>> &pHelper, int nOutRes) {
        allList = ImageListType::New();
        pHelper->GetImageList(m_laiCfg.bandsNames, allList, nOutRes);
        AnglesImageType::Pointer anglesImg = GetParameterInt16VectorImage("angles");

        anglesImg->UpdateOutputInformation();
        m_anglesSplit = VectorImageToImageListType::New();
        m_anglesSplit->SetInput(anglesImg);
        m_anglesSplit->UpdateOutputInformation();
        m_anglesSplit->GetOutput()->UpdateOutputInformation();
        for (int i = 0; i < 3; i++) {
            allList->PushBack(getResampledImage(anglesImg->GetSpacing()[0],
                                                nOutRes, m_anglesSplit->GetOutput()->GetNthElement(i)));
        }

        m_ftsConcat = ImageListToVectorImageFilterType::New();
        m_ftsConcat->SetInput(allList);
    }

    OutImageType::Pointer ExecuteModel(ImageType::Pointer inImg, double quantifValue, BelcamNeuronFilter::Pointer neuronFilter,
                                       const std::string &modelTextFile)
    {
        // extract params from formated text file (mustache template)
        trained_network params = bvnet_fill_trained_network(modelTextFile);

        // define the pipeline
        neuronFilter->SetInput(inImg );
        neuronFilter->setParams( params );
        neuronFilter->setScale( quantifValue );
        neuronFilter->UpdateOutputInformation();

        return neuronFilter->GetOutput();
    }

    void WriteOutput(OutImageType::Pointer inImg, const std::string &outImg) {
        inImg->UpdateOutputInformation();

        // Create an output parameter to write the current output image
        OutputImageParameter::Pointer paramOut = OutputImageParameter::New();
        // Set the filename of the current output image
        paramOut->SetFileName(outImg);
        paramOut->SetValue(inImg);
        paramOut->SetPixelType(ImagePixelType_double);
        // Add the current level to be written
        paramOut->InitializeWriters();
        std::ostringstream osswriter;
        osswriter<< "Wrinting flags "<< outImg;
        AddProcess(paramOut->GetWriter(), osswriter.str());
        paramOut->Write();
    }

    OutImageType::Pointer MaskModelOutput(OutImageType::Pointer img, MaskedOutputFilterType::Pointer maskingFunctor,
                                          const std::unique_ptr<MetadataHelper<short>> &pHelper, int nOutRes) {
        MetadataHelper<short>::SingleBandMasksImageType::Pointer imgMsk = pHelper->GetMasksImage(ALL, false);

        imgMsk->UpdateOutputInformation();

        maskingFunctor->SetInput1(img);
        maskingFunctor->SetInput2(getResampledImage2(imgMsk->GetSpacing()[0], nOutRes, imgMsk));
        return maskingFunctor->GetOutput();
    }

    void loadLaiConfiguration(const std::unique_ptr<MetadataHelper<short>> &pHelper) {
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
        std::cout << "addndvi : " << m_laiCfg.bUseNdvi           << std::endl;
        std::cout << "addrvi : " << m_laiCfg.bUseRvi             << std::endl;
        std::cout << "Bands used : ";
        for (std::vector<std::string>::const_iterator i = m_laiCfg.bandsNames.begin(); i != m_laiCfg.bandsNames.end(); ++i) {
            std::cout << *i << ' ';
        }
        std::cout << std::endl;
        std::cout << "=================================" << std::endl;
    }

    ImageListToVectorImageFilterType::Pointer m_ftsConcat;

    ImageListType::Pointer allList;

    ImageResampler<InternalImageType, InternalImageType> m_Resampler;
    ImageResampler<MetadataHelper<short>::SingleBandImageType, MetadataHelper<short>::SingleBandImageType> m_Resampler2;

    ImageType::Pointer m_msksImg;

    std::unique_ptr<MetadataHelper<short>> m_pHelper;

    BelcamNeuronFilter::Pointer m_laiNeuronFilter;
    BelcamNeuronFilter::Pointer m_faparNeuronFilter;
    BelcamNeuronFilter::Pointer m_fcoverNeuronFilter;

    VectorImageToImageListType::Pointer m_anglesSplit;

    LAIBandsConfigInfos m_laiCfg;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::BVLaiNewProcessor)


