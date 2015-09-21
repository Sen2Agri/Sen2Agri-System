#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbVectorImage.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbStreamingResampleImageFilter.h"
#include "otbBandMathImageFilter.h"
#include "MetadataHelperFactory.h"

//Transform
#include "itkScalableAffineTransform.h"

typedef float                                                             PixelType;
typedef otb::VectorImage<PixelType, 2>                                    ImageType;
typedef otb::Image<PixelType, 2>                                          InternalImageType;
typedef otb::ImageFileReader<ImageType>                                   ReaderType;
typedef otb::ImageList<InternalImageType>                                 ImageListType;

typedef otb::VectorImageToImageListFilter<ImageType, ImageListType>       VectorImageToImageListType;
typedef otb::ImageListToVectorImageFilter<ImageListType, ImageType>       ImageListToVectorImageFilterType;
typedef otb::BandMathImageFilter<InternalImageType>                       BandMathImageFilterType;
typedef otb::ObjectList<BandMathImageFilterType>                          BandMathImageFilterListType;

typedef otb::StreamingResampleImageFilter<InternalImageType, InternalImageType, double>     ResampleFilterType;
typedef otb::ObjectList<ResampleFilterType>                                                 ResampleFilterListType;

typedef itk::NearestNeighborInterpolateImageFunction<InternalImageType, double>             NearestNeighborInterpolationType;
typedef itk::LinearInterpolateImageFunction<InternalImageType, double>                      LinearInterpolationType;
typedef otb::BCOInterpolateImageFunction<InternalImageType>                                 BCOInterpolationType;
typedef itk::IdentityTransform<double, InternalImageType::ImageDimension>                   IdentityTransformType;

typedef itk::ScalableAffineTransform<double, InternalImageType::ImageDimension>             ScalableTransformType;
typedef ScalableTransformType::OutputVectorType     OutputVectorType;

namespace otb
{
namespace Wrapper
{
class NdviRviExtractionApp : public Application
{
public:
  typedef NdviRviExtractionApp Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  itkNewMacro(Self)
  itkTypeMacro(NdviRviExtractionApp, otb::Application)

private:

  void DoInit()
  {
        SetName("NdviRviExtraction");
        SetDescription("The feature extraction step produces the NDVI and RVI for a product.");

        SetDocName("NdviRviExtraction");
        SetDocLongDescription("The feature extraction step produces the NDVI and RVI from a product.");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");

        AddParameter(ParameterType_String, "xml", "Product Metadata XML File");
        AddParameter(ParameterType_Int, "outres", "Output resolution. If not specified, is the same as the input resolution.");
        MandatoryOff("outres");

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
        m_imgReader = ReaderType::New();
        m_imgSplit = VectorImageToImageListType::New();
        m_ResamplersList = ResampleFilterListType::New();

        // NDVI = (NIR-RED) / (NIR + RED)
        // RVI = NIR / RED
        // The BandMath expression for NDVI
        std::ostringstream ndviExprStream;
        // The BandMath expression for RVI
        std::ostringstream rviExprStream;

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

        auto factory = MetadataHelperFactory::New();
        // we are interested only in the 10m resolution as here we have the RED and NIR
        auto pHelper = factory->GetMetadataHelper(inMetadataXml, 10);

        int nNirBand = pHelper->GetNirBandIndex();
        int nRedBandIdx = pHelper->GetRedBandIndex();
        std::string RED = getBandStr(nRedBandIdx);
        std::string NIR = getBandStr(nNirBand);

        // NO_DATA will be in the output also -10000 but the NDVI and RVI values will not be multiplied with the quantification value
        ndviExprStream << "(abs(" << NIR << "+10000)<0.000001 || abs(" << RED << "+10000)<0.000001) ? -10000.0 : (abs(" << NIR << "+" << RED << ") < 0.000001 ? 0 : (" <<
                       NIR << "-" << RED << ")/(" << NIR <<"+" << RED << "))";
        rviExprStream << "(abs(" << NIR << "+10000)<0.000001 || abs(" << RED << "+10000)<0.000001) ? -10000.0 : ((" << RED << "<0.000001) ? 0 : (" << NIR << " / " << RED << "))";

        //Read all input parameters
        m_imgReader->SetFileName(pHelper->GetImageFileName());
        m_imgReader->UpdateOutputInformation();

        int curRes = m_imgReader->GetOutput()->GetSpacing()[0];
        int nOutRes = curRes;
        if(HasValue("outres")) {
            nOutRes = GetParameterInt("outres");
            if(nOutRes != 10 && nOutRes != 20) {
                itkExceptionMacro("Invalid output resolution specified (only 10 and 20 accepted)" << nOutRes);
            }
        }

        m_imgSplit->SetInput(m_imgReader->GetOutput());
        // Use a filter to split the input into a list of one band images
        m_imgSplit->UpdateOutputInformation();
        int nbBands = m_imgReader->GetOutput()->GetNumberOfComponentsPerPixel();

        // Create the ndvi bandmath filter
        m_ndviFilter = BandMathImageFilterType::New();
        // Create the rvi bandmath filter
        m_rviFilter = BandMathImageFilterType::New();

        for (int j = 0; j < nbBands; j++) {
            // Set all bands of the current image as inputs
            m_ndviFilter->SetNthInput(j, getResampledImage(curRes, nOutRes, m_imgSplit->GetOutput()->GetNthElement(j)));
            m_rviFilter->SetNthInput(j, getResampledImage(curRes, nOutRes, m_imgSplit->GetOutput()->GetNthElement(j)));
        }

        // set the expressions
        m_ndviFilter->SetExpression(ndviExprStream.str());
        m_rviFilter->SetExpression(rviExprStream.str());

        // export the NDVI in a distinct raster if we have this option set
        if(bOutNdvi) {
            SetParameterOutputImage("ndvi", m_ndviFilter->GetOutput());
        }

        // export the RVI in a distinct raster if we have this option set
        if(bOutRvi) {
            SetParameterOutputImage("rvi", m_rviFilter->GetOutput());
        }
        // export the features bands if we have this option set
        if(bOutFts)
        {
            ImageListType::Pointer ftsList = ImageListType::New();
            ftsList->PushBack(m_ndviFilter->GetOutput());
            ftsList->PushBack(m_rviFilter->GetOutput());
            m_ftsConcat = ImageListToVectorImageFilterType::New();
            m_ftsConcat->SetInput(ftsList);
            SetParameterOutputImage("fts", m_ftsConcat->GetOutput());
        }
  }

  std::string getBandStr(int nBand) {
      std::ostringstream oss;
      oss << "b" << nBand;
      return oss.str();
  }

  ResampleFilterType::Pointer getResampler(const InternalImageType::Pointer& image, const float& ratio) {
       ResampleFilterType::Pointer resampler = ResampleFilterType::New();
       resampler->SetInput(image);

      LinearInterpolationType::Pointer interpolator = LinearInterpolationType::New();
      resampler->SetInterpolator(interpolator);

      //BCOInterpolationType::Pointer interpolator = BCOInterpolationType::New();
      //interpolator->SetRadius(2);
      //resampler->SetInterpolator(interpolator);
       IdentityTransformType::Pointer transform = IdentityTransformType::New();

       resampler->SetOutputParametersFromImage( image );
       // Scale Transform
       OutputVectorType scale;
       scale[0] = 1.0 / ratio;
       scale[1] = 1.0 / ratio;

       // Evaluate spacing
       InternalImageType::SpacingType spacing = image->GetSpacing();
       InternalImageType::SpacingType OutputSpacing;
       OutputSpacing[0] = spacing[0] * scale[0];
       OutputSpacing[1] = spacing[1] * scale[1];

       resampler->SetOutputSpacing(OutputSpacing);

       FloatVectorImageType::PointType origin = image->GetOrigin();
       FloatVectorImageType::PointType outputOrigin;
       outputOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale[0] - 1.0);
       outputOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale[1] - 1.0);

       resampler->SetOutputOrigin(outputOrigin);

       resampler->SetTransform(transform);

       // Evaluate size
       ResampleFilterType::SizeType recomputedSize;
       recomputedSize[0] = image->GetLargestPossibleRegion().GetSize()[0] / scale[0];
       recomputedSize[1] = image->GetLargestPossibleRegion().GetSize()[1] / scale[1];

       resampler->SetOutputSize(recomputedSize);

       m_ResamplersList->PushBack(resampler);
       return resampler;
  }

  InternalImageType::Pointer getResampledImage(int nCurRes, int nDesiredRes,
                                               InternalImageType::Pointer inImg) {
      if(nCurRes == nDesiredRes)
          return inImg;
      float fMultiplicationFactor = ((float)nCurRes)/nDesiredRes;
      ResampleFilterType::Pointer resampler = getResampler(inImg, fMultiplicationFactor);
      return resampler->GetOutput();
  }



  ReaderType::Pointer                       m_imgReader;
  VectorImageToImageListType::Pointer       m_imgSplit;

  BandMathImageFilterType::Pointer m_ndviFilter;
  BandMathImageFilterType::Pointer m_rviFilter;

  ImageListToVectorImageFilterType::Pointer m_ftsConcat;
  ResampleFilterListType::Pointer           m_ResamplersList;

};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::NdviRviExtractionApp)


