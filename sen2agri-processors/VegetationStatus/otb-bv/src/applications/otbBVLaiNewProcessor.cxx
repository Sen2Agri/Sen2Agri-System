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

  bool operator!=( const MaskingFunctor &a) const
  {
      return false;
  }
  bool operator==( const MaskingFunctor & other ) const
  {
    return !(*this != other);
  }
  inline TOutput operator()( const TInput & A, const TInput2 & B ) const
  {
        TOutput ret(1);
        ret[0] = A[0];

        if (ret[0] < 0) {
            ret[0] = 0;
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

    typedef itk::BinaryFunctorImageFilter<OutImageType,MetadataHelper::SingleBandShortImageType,OutImageType,
                    MaskingFunctor<
                        OutImageType::PixelType, MetadataHelper::SingleBandShortImageType::PixelType,
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

        // load the LAI config file
        loadLaiConfiguration(pHelper);

        // Extract the needed bands
        handleFeaturesCreation(pHelper, nOutRes);

        if (HasValue("fts")) {
            SetParameterOutputImage("fts", m_ftsConcat->GetOutput());
        }

        if (HasValue("outlai")) {
            ProduceOutput(pHelper, LAI);
        }
        if (HasValue("outfapar")) {
            ProduceOutput(pHelper, FAPAR);
        }
        if (HasValue("outfcover")) {
            ProduceOutput(pHelper, FCOVER);
        }
    }

    void ProduceOutput(const std::unique_ptr<MetadataHelper> &pHelper, IndexType indexType) {
        std::string modelTextFile;
        std::string modelParamName;
        std::string outparamName;
        std::string laiCfgEntry;
        BelcamNeuronFilter::Pointer filter;
        MaskedOutputFilterType::Pointer maskingFunctor;
        switch (indexType) {
            case LAI:
                modelParamName = "laimodel";
                outparamName = "outlai";
                laiCfgEntry = m_laiCfg.laiModelFilePath;
                m_laiNeuronFilter = BelcamNeuronFilter::New();
                filter = m_laiNeuronFilter;
                m_LaiMaskedOutputFunctor = MaskedOutputFilterType::New();
                maskingFunctor = m_LaiMaskedOutputFunctor;
                break;
            case FAPAR:
                modelParamName = "faparmodel";
                outparamName = "outfapar";
                laiCfgEntry = m_laiCfg.faparModelFilePath;
                m_faparNeuronFilter = BelcamNeuronFilter::New();
                filter = m_faparNeuronFilter;
                m_FaparMaskedOutputFunctor = MaskedOutputFilterType::New();
                maskingFunctor = m_FaparMaskedOutputFunctor;
                break;
            case FCOVER:
                modelParamName = "fcovermodel";
                outparamName = "outfcover";
                laiCfgEntry = m_laiCfg.fcoverModelFilePath;
                m_fcoverNeuronFilter = BelcamNeuronFilter::New();
                filter = m_fcoverNeuronFilter;
                m_FcoverMaskedOutputFunctor = MaskedOutputFilterType::New();
                maskingFunctor = m_FcoverMaskedOutputFunctor;
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

        // Mask the output according to the flags
        OutImageType::Pointer maskedImg = MaskModelOutput(modelOutput, maskingFunctor, pHelper);

        const std::string &outImgFileName = GetParameterAsString(outparamName);
        // write the output
        WriteOutput(maskedImg, outImgFileName);
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
     * Handles the creation of the features output (if configured) by adding the configured
     * bands and the NDVI and RVI if configured in LAI bands configuration file
     */
    void handleFeaturesCreation(const std::unique_ptr<MetadataHelper> &pHelper, int nOutRes) {
        std::vector<int> resolutionsVect = getResolutionsForConfiguredBands(m_laiCfg, pHelper);
        for (int cfgRes: resolutionsVect) {
            createInputImgSplitForResolution(cfgRes);
        }

        allList = ImageListType::New();
        // Iterate all configured bands taking into account the raster for the resolution the band belongs
        for (int bandIdx: m_laiCfg.bandsIdxs) {
            int curBandRes = pHelper->GetResolutionForAbsoluteBandIndex(bandIdx);
            // Get the helper for the current resolution
            const auto &pResHelper = m_metadataHelpersMap.at(curBandRes);
            // get the relative band index in the product raster for the current band resolution
            // The relative band index is 1 based value so we must extract the 0 based one
            int relBandIdx = pResHelper->GetRelativeBandIndex(bandIdx) - 1;
            VectorImageToImageListType::Pointer imgInputSplit = m_imgInputSplittersMap.at(curBandRes);
            std::cout << "Adding band with id: " << bandIdx << ", res: " <<  curBandRes
                      << ", relative band idx: " << relBandIdx  << std::endl;
            allList->PushBack(getResampledImage(curBandRes, nOutRes, imgInputSplit->GetOutput()->GetNthElement(relBandIdx)));
        }

        AnglesImageType::Pointer anglesImg = createAnglesBands(pHelper);
        m_anglesSplit = VectorImageToImageListType::New();
        m_anglesSplit->SetInput(anglesImg);
        m_anglesSplit->UpdateOutputInformation();
        m_anglesSplit->GetOutput()->UpdateOutputInformation();
        for (int i = 0; i < 3; i++) {
            allList->PushBack(m_anglesSplit->GetOutput()->GetNthElement(i));
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

            VectorImageToImageListType::Pointer imgInputSplit = VectorImageToImageListType::New();
            imgInputSplit->SetInput(imgReader->GetOutput());
            imgInputSplit->UpdateOutputInformation();
            imgInputSplit->GetOutput()->UpdateOutputInformation();

            // add the new translator functor and the immage splitter into their maps
            m_imgInputReadersMap[res] = imgReader;
            m_imgInputSplittersMap[res] = imgInputSplit;
        }
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

    AnglesImageType::Pointer createAnglesBands(const std::unique_ptr<MetadataHelper> &pHelper) {
        auto sz = m_defImgReader->GetOutput()->GetLargestPossibleRegion().GetSize();
        auto spacing = m_defImgReader->GetOutput()->GetSpacing();
        double quantifValue = pHelper->GetReflectanceQuantificationValue();
        int width = sz[0];
        int height = sz[1];

        m_AnglesRaster = AnglesImageType::New();

        AnglesImageType::IndexType start;
        start[0] =   0;  // first index on X
        start[1] =   0;  // first index on Y

        AnglesImageType::SizeType size;
        size[0]  = ANGLES_GRID_SIZE;  // size along X
        size[1]  = ANGLES_GRID_SIZE;  // size along Y

        AnglesImageType::RegionType region;
        region.SetSize(size);
        region.SetIndex(start);

        m_AnglesRaster->SetRegions(region);
        m_AnglesRaster->SetNumberOfComponentsPerPixel(3);

        AnglesImageType::SpacingType anglesRasterSpacing;
        anglesRasterSpacing[0] = (((float)width) * spacing[0]) / ANGLES_GRID_SIZE; // spacing along X
        anglesRasterSpacing[1] = (((float)height) * spacing[1]) / ANGLES_GRID_SIZE; // spacing along Y
        m_AnglesRaster->SetSpacing(anglesRasterSpacing);

        m_AnglesRaster->Allocate();
        double cosSensorZenith = round(cos(pHelper->GetSensorMeanAngles().zenith * M_PI / 180) * quantifValue);
        double cosSunZenith = round(cos(pHelper->GetSolarMeanAngles().zenith * M_PI / 180) * quantifValue);
        double cosRelAzimuth = round(cos(pHelper->GetRelativeAzimuthAngle() * M_PI / 180) * quantifValue);

        for(unsigned int i = 0; i < ANGLES_GRID_SIZE; i++) {
            for(unsigned int j = 0; j < ANGLES_GRID_SIZE; j++) {
                itk::VariableLengthVector<float> vct(3);
                vct[0] = cosSensorZenith;
                vct[1] = cosSunZenith;
                vct[2] = cosRelAzimuth;

                AnglesImageType::IndexType idx;
                idx[0] = j;
                idx[1] = i;
                m_AnglesRaster->SetPixel(idx, vct);
            }
        }
        m_AnglesRaster->UpdateOutputInformation();
        AnglesImageType::Pointer retImg = m_AnglesResampler.getResampler(m_AnglesRaster, width, height)->GetOutput();
        retImg->UpdateOutputInformation();

        return retImg;
    }

    OutImageType::Pointer MaskModelOutput(OutImageType::Pointer img, MaskedOutputFilterType::Pointer maskingFunctor,
                                          const std::unique_ptr<MetadataHelper> &pHelper) {
        MetadataHelper::SingleBandShortImageType::Pointer imgMsk = pHelper->GetMasksImage(ALL, false);

        maskingFunctor->SetInput1(img);
        maskingFunctor->SetInput2(imgMsk);
        return maskingFunctor->GetOutput();
    }

    void loadLaiConfiguration(const std::unique_ptr<MetadataHelper> &pHelper) {
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
        for (std::vector<int>::const_iterator i = m_laiCfg.bandsIdxs.begin(); i != m_laiCfg.bandsIdxs.end(); ++i) {
            std::cout << *i << ' ';
        }
        std::cout << std::endl;
        std::cout << "=================================" << std::endl;
    }

    ImageListToVectorImageFilterType::Pointer m_ftsConcat;

    ImageListType::Pointer allList;

    ReaderType::Pointer                       m_defImgReader;
    ImageResampler<InternalImageType, InternalImageType> m_Resampler;

    ImageType::Pointer m_msksImg;

    std::map<int, ReaderType::Pointer> m_imgInputReadersMap;
    std::map<int, std::unique_ptr<MetadataHelper>> m_metadataHelpersMap;
    std::map<int, VectorImageToImageListType::Pointer> m_imgInputSplittersMap;

    BelcamNeuronFilter::Pointer m_laiNeuronFilter;
    BelcamNeuronFilter::Pointer m_faparNeuronFilter;
    BelcamNeuronFilter::Pointer m_fcoverNeuronFilter;

    AnglesImageType::Pointer            m_AnglesRaster;
    ImageResampler<AnglesImageType, AnglesImageType> m_AnglesResampler;
    VectorImageToImageListType::Pointer m_anglesSplit;

    MaskedOutputFilterType::Pointer m_LaiMaskedOutputFunctor;
    MaskedOutputFilterType::Pointer m_FaparMaskedOutputFunctor;
    MaskedOutputFilterType::Pointer m_FcoverMaskedOutputFunctor;

    LAIBandsConfigInfos m_laiCfg;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::BVLaiNewProcessor)


