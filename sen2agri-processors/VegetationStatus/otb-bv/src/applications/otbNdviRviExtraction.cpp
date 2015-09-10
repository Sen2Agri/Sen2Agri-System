#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbVectorImage.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbBandMathImageFilter.h"
#include "MetadataHelperFactory.h"

typedef otb::VectorImage<float, 2>  ImageType;
typedef otb::Image<float, 2>        InternalImageType;
typedef otb::ImageFileReader<ImageType>                                   ReaderType;
typedef otb::ImageFileWriter<ImageType>                                   WriterType;
typedef otb::ImageList<InternalImageType>                                 ImageListType;
typedef otb::VectorImageToImageListFilter<ImageType, ImageListType>       VectorImageToImageListType;
typedef otb::ImageListToVectorImageFilter<ImageListType, ImageType>       ImageListToVectorImageFilterType;
typedef otb::BandMathImageFilter<InternalImageType>                       BandMathImageFilterType;
typedef otb::ObjectList<BandMathImageFilterType>                          BandMathImageFilterListType;

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
        AddParameter(ParameterType_OutputImage, "ndvi", "NDVI image");
        MandatoryOff("ndvi");
        AddParameter(ParameterType_OutputImage, "rvi", "RVI image");
        MandatoryOff("rvi");
        AddParameter(ParameterType_OutputImage, "fts", "Features image containing NDVI and RVI bands");
        MandatoryOff("fts");

        SetDocExampleParameterValue("xml", "data.xml");
        SetDocExampleParameterValue("ndvi", "ndvi.tif");
        SetDocExampleParameterValue("rvi", "rvi.tif");
  }

  void DoUpdateParameters()
  {
  }
  void DoExecute()
  {
        // define all needed types
        m_imgReader = ReaderType::New();
        m_imgSplit = VectorImageToImageListType::New();

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

        ndviExprStream << "(" << NIR << "!= -10000) ? (" << NIR << "-" << RED << ") / " <<
                        "(" << NIR << "+" << RED << ") : -10000";
        rviExprStream << "(" << NIR << "!= -10000 && " << RED << "!= 0) ? (" << NIR << " / " << RED << ") : -10000";

        //Read all input parameters
        m_imgReader->SetFileName(pHelper->GetImageFileName());
        m_imgReader->UpdateOutputInformation();

        // Use a filter to split the input into a list of one band images
        m_imgSplit->SetInput(m_imgReader->GetOutput());
        m_imgSplit->UpdateOutputInformation();
        int nbBands = m_imgReader->GetOutput()->GetNumberOfComponentsPerPixel();

        // Create the ndvi bandmath filter
        m_ndviFilter = BandMathImageFilterType::New();
        // Create the rvi bandmath filter
        m_rviFilter = BandMathImageFilterType::New();

        for (int j = 0; j < nbBands; j++) {
            // Set all bands of the current image as inputs
            m_ndviFilter->SetNthInput(j, m_imgSplit->GetOutput()->GetNthElement(j));
            m_rviFilter->SetNthInput(j, m_imgSplit->GetOutput()->GetNthElement(j));
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

  ReaderType::Pointer                       m_imgReader;
  VectorImageToImageListType::Pointer       m_imgSplit;

  BandMathImageFilterType::Pointer m_ndviFilter;
  BandMathImageFilterType::Pointer m_rviFilter;

  ImageListToVectorImageFilterType::Pointer m_ftsConcat;

};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::NdviRviExtractionApp)


