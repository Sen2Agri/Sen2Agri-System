#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbVectorImage.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbStreamingResampleImageFilter.h"
#include "MetadataHelperFactory.h"

//Transform
#include "itkScalableAffineTransform.h"
#include "GlobalDefs.h"

typedef float                                                             PixelType;
typedef otb::VectorImage<PixelType, 2>                                    ImageType;
typedef otb::VectorImage<PixelType, 2>                                    OutImageType;
typedef otb::Image<PixelType, 2>                                          InternalImageType;
typedef otb::ImageFileReader<ImageType>                                   ReaderType;
typedef otb::ImageList<InternalImageType>                                 ImageListType;

typedef otb::VectorImageToImageListFilter<ImageType, ImageListType>       VectorImageToImageListType;
typedef otb::ImageListToVectorImageFilter<ImageListType, ImageType>       ImageListToVectorImageFilterType;

typedef otb::StreamingResampleImageFilter<InternalImageType, InternalImageType, double>     ResampleFilterType;
typedef otb::ObjectList<ResampleFilterType>                                                 ResampleFilterListType;

typedef itk::NearestNeighborInterpolateImageFunction<InternalImageType, double>             NearestNeighborInterpolationType;
typedef itk::LinearInterpolateImageFunction<InternalImageType, double>                      LinearInterpolationType;
typedef otb::BCOInterpolateImageFunction<InternalImageType>                                 BCOInterpolationType;
typedef itk::IdentityTransform<double, InternalImageType::ImageDimension>                   IdentityTransformType;

typedef itk::ScalableAffineTransform<double, InternalImageType::ImageDimension>             ScalableTransformType;
typedef ScalableTransformType::OutputVectorType     OutputVectorType;


template< class TInput, class TOutput>
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
private:
  int m_nRedBandIdx;
  int m_nNirBandIdx;
};

// Translates the INT reflectances in the input image into floats, taking into account
// the quantification value
template< class TInput, class TOutput>
class ReflectanceTranslationFunctor
{
public:
    ReflectanceTranslationFunctor() {}
    ~ReflectanceTranslationFunctor() {}
    void Initialize(float fQuantificationVal) {
        m_fQuantifVal = fQuantificationVal;
  }

  bool operator!=( const ReflectanceTranslationFunctor &a) const
  {
      return (this->m_fQuantifVal != a.m_fQuantifVal);
  }
  bool operator==( const ReflectanceTranslationFunctor & other ) const
  {
    return !(*this != other);
  }
  inline TOutput operator()( const TInput & A ) const
  {
      TOutput ret(A.Size());
      for(int i = 0; i<A.Size(); i++) {
           if(fabs(A[i] - NO_DATA_VALUE) < 0.00001) {
               ret[i] = NO_DATA_VALUE;
           } else {
                ret[i] = static_cast< float >(static_cast< float >(A[i]))/m_fQuantifVal;
           }
      }

      return ret;
  }
private:
  int m_fQuantifVal;
};

namespace otb
{
namespace Wrapper
{
class NdviRviExtraction2 : public Application
{
    typedef itk::UnaryFunctorImageFilter<ImageType,ImageType,
                    NdviRviFunctor<
                        ImageType::PixelType,
                        ImageType::PixelType> > FilterType;
    typedef itk::UnaryFunctorImageFilter<ImageType,ImageType,
                    ReflectanceTranslationFunctor<
                        ImageType::PixelType,
                        ImageType::PixelType> > ReflTransFilterType;

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

        AddParameter(ParameterType_OutputImage, "ndvi", "NDVI image");
        MandatoryOff("ndvi");
        AddParameter(ParameterType_OutputImage, "rvi", "RVI image");
        MandatoryOff("rvi");
        AddParameter(ParameterType_OutputImage, "fts", "Features image containing NDVI and RVI bands");
        MandatoryOff("fts");
        AddParameter(ParameterType_Int, "addallrefls", "Add all reflectance bands fo the features output.");
        MandatoryOff("addallrefls");
        SetDefaultParameterInt("addallrefls", 1);

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
        m_imgReader = ReaderType::New();
        m_ResamplersList = ResampleFilterListType::New();

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

        auto factory = MetadataHelperFactory::New();
        // we are interested only in the 10m resolution as here we have the RED and NIR
        auto pHelper = factory->GetMetadataHelper(inMetadataXml, 10);

        // the bands are 1 based
        int nNirBandIdx = pHelper->GetRelNirBandIndex()-1;
        int nRedBandIdx = pHelper->GetRelRedBandIndex()-1;

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

        m_Functor = FilterType::New();
        m_Functor->GetFunctor().Initialize(nRedBandIdx, nNirBandIdx);
        m_Functor->SetInput(m_imgReader->GetOutput());

        allList = ImageListType::New();

        m_imgSplit = VectorImageToImageListType::New();
        m_functorOutput = m_Functor->GetOutput();
        m_functorOutput->UpdateOutputInformation();
        m_imgSplit->SetInput(m_functorOutput);
        m_imgSplit->UpdateOutputInformation();
        m_imgSplit->GetOutput()->UpdateOutputInformation();

        // export the NDVI in a distinct raster if we have this option set
        if(bOutNdvi) {
            //SetParameterOutputImagePixelType("ndvi", ImagePixelType_int16);
            SetParameterOutputImage("ndvi", getResampledImage(curRes, nOutRes, m_imgSplit->GetOutput()->GetNthElement(0)).GetPointer());
        }

        // export the RVI in a distinct raster if we have this option set
        if(bOutRvi) {
            //SetParameterOutputImagePixelType("rvi", ImagePixelType_int16);
            SetParameterOutputImage("rvi", getResampledImage(curRes, nOutRes, m_imgSplit->GetOutput()->GetNthElement(1)).GetPointer());
        }
        if(bOutFts) {
            // Translate the input image from short reflectance values to float (subunitaire) values
            // translated using the quantification value
            double fQuantifVal = pHelper->GetReflectanceQuantificationValue();
            m_ReflTransFunctor = ReflTransFilterType::New();
            m_ReflTransFunctor->GetFunctor().Initialize((float)fQuantifVal);
            m_ReflTransFunctor->SetInput(m_imgReader->GetOutput());
            int nInputBands = m_imgReader->GetOutput()->GetNumberOfComponentsPerPixel();
            m_ReflTransFunctor->GetOutput()->SetNumberOfComponentsPerPixel(nInputBands);

            m_imgInputSplit = VectorImageToImageListType::New();
            m_imgInputSplit->SetInput(m_ReflTransFunctor->GetOutput());
            m_imgInputSplit->UpdateOutputInformation();
            m_imgInputSplit->GetOutput()->UpdateOutputInformation();

            if(bUseAllBands) {
                // add all bands from the input image
                int nBandsNo = m_imgReader->GetOutput()->GetNumberOfComponentsPerPixel();
                for(int i = 0; i<nBandsNo; i++) {
                    allList->PushBack(getResampledImage(curRes, nOutRes, m_imgInputSplit->GetOutput()->GetNthElement(i)));
                }
            } else {
                // add the RED and NIR bands from the input image
                allList->PushBack(getResampledImage(curRes, nOutRes, m_imgInputSplit->GetOutput()->GetNthElement(nRedBandIdx)));
                allList->PushBack(getResampledImage(curRes, nOutRes, m_imgInputSplit->GetOutput()->GetNthElement(nNirBandIdx)));
            }
            // add the bands for NDVI and RVI
            allList->PushBack(getResampledImage(curRes, nOutRes, m_imgSplit->GetOutput()->GetNthElement(0)));
            allList->PushBack(getResampledImage(curRes, nOutRes, m_imgSplit->GetOutput()->GetNthElement(1)));
            m_ftsConcat = ImageListToVectorImageFilterType::New();
            m_ftsConcat->SetInput(allList);
            //SetParameterOutputImagePixelType("fts", ImagePixelType_int16);
            SetParameterOutputImage("fts", m_ftsConcat->GetOutput());
        }
  }

  ResampleFilterType::Pointer getResampler(const InternalImageType::Pointer& image, const float& ratio) {
       ResampleFilterType::Pointer resampler = ResampleFilterType::New();
       resampler->SetInput(image);

       LinearInterpolationType::Pointer interpolator = LinearInterpolationType::New();
       resampler->SetInterpolator(interpolator);

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

    VectorImageToImageListType::Pointer       m_imgSplit;
    VectorImageToImageListType::Pointer       m_imgInputSplit;
    ImageListToVectorImageFilterType::Pointer m_ftsConcat;

    ImageListType::Pointer allList;

    OutImageType::Pointer m_functorOutput;
    FilterType::Pointer m_Functor;
    ReflTransFilterType::Pointer m_ReflTransFunctor;

    ReaderType::Pointer                       m_imgReader;
    ResampleFilterListType::Pointer           m_ResamplersList;

};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::NdviRviExtraction2)


