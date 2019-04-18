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
class NdviRviExtraction2 : public Application
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
  typedef NdviRviExtraction2 Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  itkNewMacro(Self)
  itkTypeMacro(NdviRviExtractionApp2, otb::Application)

private:

  void DoInit()
  {
        SetName("NdviRviExtraction2");
        SetDescription("The feature extraction step produces the NDVI and RVI for a product.");

        SetDocName("NdviRviExtraction2");
        SetDocLongDescription("The feature extraction step produces the NDVI and RVI from a product.");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");

        AddParameter(ParameterType_String, "xml", "Product Metadata XML File");
        AddParameter(ParameterType_Int, "outres", "Output resolution. If not specified, is the same as the input resolution.");
        MandatoryOff("outres");

        AddParameter(ParameterType_InputImage, "msks", "Masks flags used for masking final NDVI/RVI values");
        MandatoryOff("msks");

        AddParameter(ParameterType_OutputImage, "ndvi", "NDVI image");
        MandatoryOff("ndvi");
        AddParameter(ParameterType_OutputImage, "rvi", "RVI image");
        MandatoryOff("rvi");
        AddParameter(ParameterType_OutputImage, "fts", "Features image containing NDVI and RVI bands");
        MandatoryOff("fts");
        AddParameter(ParameterType_Int, "addallrefls", "Add all reflectance bands fo the features output.");
        MandatoryOff("addallrefls");
        SetDefaultParameterInt("addallrefls", 1);

        AddParameter(ParameterType_Int, "addndvi", "Add NDVI band for the features output.");
        MandatoryOff("addndvi");
        SetDefaultParameterInt("addndvi", 0);

        AddParameter(ParameterType_Int, "addrvi", "Add RVI band for the features output.");
        MandatoryOff("addrvi");
        SetDefaultParameterInt("addrvi", 0);

        AddParameter(ParameterType_Int, "addblue", "Add blue band for the features output.");
        MandatoryOff("addblue");
        SetDefaultParameterInt("addblue", 0);

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
        bool bUseAllBands = true;
        bool bUseNdvi = false;
        bool bUseRvi = false;
        bool bUseBlueBand = false;
        m_imgReader = ReaderType::New();

        std::string inMetadataXml = GetParameterString("xml");
        if (inMetadataXml.empty())
        {
            itkExceptionMacro("No input metadata XML set...; please set the input image");
        }
        bool bOutNdvi = HasValue("ndvi");
        bool bOutRvi = HasValue("rvi");
        bool bOutFts = HasValue("fts");
        if(!bOutNdvi && !bOutRvi && !bOutFts) {
            itkExceptionMacro("No output specified. Please specify at least one output (ndvi or rvi)");
        }
        if(HasValue("addallrefls")) {
            bUseAllBands = (GetParameterInt("addallrefls") != 0);
        }
        if(HasValue("addndvi")) {
            bUseNdvi = (GetParameterInt("addndvi") != 0);
        }
        if(HasValue("addrvi")) {
            bUseRvi = (GetParameterInt("addrvi") != 0);
        }
        if(HasValue("addblue")) {
            bUseBlueBand = (GetParameterInt("addblue") != 0);
        }
        std::cout << "=================================" << std::endl;
        std::cout << "addallrefls : " << bUseAllBands   << std::endl;
        std::cout << "addndvi : " << bUseNdvi           << std::endl;
        std::cout << "addrvi : " << bUseRvi             << std::endl;
        std::cout << "addblue : " << bUseBlueBand       << std::endl;
        std::cout << "=================================" << std::endl;

        auto factory = MetadataHelperFactory::New();
        // we are interested only in the 10m resolution as here we have the RED and NIR
        m_pHelper = factory->GetMetadataHelper<float>(inMetadataXml);

        // the bands are 1 based
        const std::string &nirBandName = m_pHelper->GetNirBandName();
        const std::string &redBandName = m_pHelper->GetRedBandName();
        const std::string &blueBandName = m_pHelper->GetBlueBandName();

        std::vector<int> relBandIdxs;
        MetadataHelper<float>::VectorImageType::Pointer img = m_pHelper->GetImage({redBandName, nirBandName}, &relBandIdxs);
        img->UpdateOutputInformation();

        int curRes = img->GetSpacing()[0];
        int nOutRes = curRes;
        if(HasValue("outres")) {
            nOutRes = GetParameterInt("outres");
            if(nOutRes != 10 && nOutRes != 20) {
                itkExceptionMacro("Invalid output resolution specified (only 10 and 20 accepted)" << nOutRes);
            }
        }

        bool bHasMsks = HasValue("msks");
        if(bHasMsks) {
            m_msksImg = GetParameterFloatVectorImage("msks");
            m_MaskedFunctor = MaskedNDVIRVIFilterType::New();
            m_MaskedFunctor->GetFunctor().Initialize(relBandIdxs[0], relBandIdxs[1]);
            m_MaskedFunctor->SetInput1(img);
            m_MaskedFunctor->SetInput2(m_msksImg);
            m_functorOutput = m_MaskedFunctor->GetOutput();
        } else {
            m_UnmaskedFunctor = UnmaskedNDVIRVIFilterType::New();
            m_UnmaskedFunctor->GetFunctor().Initialize(relBandIdxs[0], relBandIdxs[1]);
            m_UnmaskedFunctor->SetInput(img);
            m_functorOutput = m_UnmaskedFunctor->GetOutput();
        }

        allList = ImageListType::New();

        m_imgSplit = VectorImageToImageListType::New();
        m_functorOutput->UpdateOutputInformation();
        m_imgSplit->SetInput(m_functorOutput);
        m_imgSplit->UpdateOutputInformation();
        m_imgSplit->GetOutput()->UpdateOutputInformation();

        // export the NDVI in a distinct raster if we have this option set
        if(bOutNdvi) {
            m_floatToShortFunctor = FloatToShortTransFilterType::New();
            // quantify the image using the default factor and considering 0 as NO_DATA but
            // also setting all values less than 0 to 0
            m_floatToShortFunctor->GetFunctor().Initialize(DEFAULT_QUANTIFICATION_VALUE, NO_DATA_VALUE, true);
            m_floatToShortFunctor->SetInput(getResampledImage(curRes, nOutRes,
                                  m_imgSplit->GetOutput()->GetNthElement(0)).GetPointer());
            m_floatToShortFunctor->GetOutput()->SetNumberOfComponentsPerPixel(1);
            SetParameterOutputImage("ndvi", m_floatToShortFunctor->GetOutput());
            SetParameterOutputImagePixelType("ndvi", ImagePixelType_int16);
        }

        // export the RVI in a distinct raster if we have this option set
        if(bOutRvi) {
            //SetParameterOutputImagePixelType("rvi", ImagePixelType_int16);
            SetParameterOutputImage("rvi", getResampledImage(curRes, nOutRes, m_imgSplit->GetOutput()->GetNthElement(1)).GetPointer());
        }

        if(bOutFts) {
            const std::vector<std::string> &bandNames = m_pHelper->GetBandNamesForResolution(m_pHelper->GetProductResolutions()[0]);
            relBandIdxs.clear();
            MetadataHelper<float>::VectorImageType::Pointer imgFts = m_pHelper->GetImage(bandNames, &relBandIdxs);
            imgFts->UpdateOutputInformation();
            // Translate the input image from short reflectance values to float (subunitaire) values
            // translated using the quantification value
            double fQuantifVal = m_pHelper->GetReflectanceQuantificationValue();
            m_ReflTransFunctor = ReflTransFilterType::New();
            m_ReflTransFunctor->GetFunctor().Initialize((float)fQuantifVal);
            m_ReflTransFunctor->SetInput(imgFts);
            int nInputBands = m_imgReader->GetOutput()->GetNumberOfComponentsPerPixel();
            m_ReflTransFunctor->GetOutput()->SetNumberOfComponentsPerPixel(nInputBands);

            m_imgInputSplit = VectorImageToImageListType::New();
            m_imgInputSplit->SetInput(m_ReflTransFunctor->GetOutput());
            m_imgInputSplit->UpdateOutputInformation();
            m_imgInputSplit->GetOutput()->UpdateOutputInformation();

            if(bUseAllBands) {
                // add all bands from the input image
                for(size_t i = 0; i<bandNames.size(); i++) {
                    if (bUseBlueBand || (bandNames[i] != blueBandName)) {
                        allList->PushBack(getResampledImage(curRes, nOutRes, m_imgInputSplit->GetOutput()->GetNthElement(i)));
                    }
                }
            } else {
                // add the RED and NIR bands from the input image
                std::ptrdiff_t nRedBandIdx = relBandIdxs[std::distance(bandNames.begin(), find(bandNames.begin(), bandNames.end(), redBandName))];
                std::ptrdiff_t nNirBandIdx = relBandIdxs[std::distance(bandNames.begin(), find(bandNames.begin(), bandNames.end(), redBandName))];

                allList->PushBack(getResampledImage(curRes, nOutRes, m_imgInputSplit->GetOutput()->GetNthElement(nRedBandIdx)));
                allList->PushBack(getResampledImage(curRes, nOutRes, m_imgInputSplit->GetOutput()->GetNthElement(nNirBandIdx)));
            }
            // add the bands for NDVI and RVI
            if(bUseNdvi) {
                allList->PushBack(getResampledImage(curRes, nOutRes, m_imgSplit->GetOutput()->GetNthElement(0)));
            }
            if(bUseRvi) {
                allList->PushBack(getResampledImage(curRes, nOutRes, m_imgSplit->GetOutput()->GetNthElement(1)));
            }
            m_ftsConcat = ImageListToVectorImageFilterType::New();
            m_ftsConcat->SetInput(allList);
            //SetParameterOutputImagePixelType("fts", ImagePixelType_int16);
            SetParameterOutputImage("fts", m_ftsConcat->GetOutput());
        }
  }

  InternalImageType::Pointer getResampledImage(int nCurRes, int nDesiredRes,
                                               InternalImageType::Pointer inImg) {
      if(nCurRes == nDesiredRes)
          return inImg;
      float fMultiplicationFactor = ((float)nCurRes)/nDesiredRes;
      ResampleFilterType::Pointer resampler = m_Resampler.getResampler(inImg, fMultiplicationFactor);
      return resampler->GetOutput();
  }

    VectorImageToImageListType::Pointer       m_imgSplit;
    VectorImageToImageListType::Pointer       m_imgInputSplit;
    ImageListToVectorImageFilterType::Pointer m_ftsConcat;

    ImageListType::Pointer allList;

    OutImageType::Pointer m_functorOutput;
    UnmaskedNDVIRVIFilterType::Pointer m_UnmaskedFunctor;
    MaskedNDVIRVIFilterType::Pointer m_MaskedFunctor;

    ReflTransFilterType::Pointer m_ReflTransFunctor;
    FloatToShortTransFilterType::Pointer  m_floatToShortFunctor;

    ReaderType::Pointer                       m_imgReader;
    ImageResampler<InternalImageType, InternalImageType> m_Resampler;

    ImageType::Pointer m_msksImg;

    std::unique_ptr<MetadataHelper<float>> m_pHelper;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::NdviRviExtraction2)


