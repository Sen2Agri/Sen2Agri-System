#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbVectorImage.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbBandMathImageFilter.h"
#include "MetadataHelperFactory.h"
#include "GlobalDefs.h"

namespace otb
{
namespace Wrapper
{
class TimeSeriesBuilder : public Application
{
public:
  typedef TimeSeriesBuilder Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

    typedef otb::VectorImage<float, 2>              ImageType;
    typedef otb::Image<float, 2>                    InternalImageType;
    typedef otb::ImageFileReader<ImageType>         ReaderType;
    typedef otb::ImageFileWriter<ImageType>         WriterType;
    typedef otb::ImageList<InternalImageType>       ImageListType;
    typedef otb::VectorImageToImageListFilter<ImageType, ImageListType>       VectorImageToImageListType;
    typedef otb::ImageListToVectorImageFilter<ImageListType, ImageType>       ImageListToVectorImageFilterType;

    typedef otb::ImageFileReader<ImageType>         ImageReaderType;
    typedef otb::ObjectList<ImageReaderType>        ImageReaderListType;
    typedef otb::ObjectList<ImageType>              ImagesListType;

    typedef otb::ObjectList<VectorImageToImageListType>    SplitFilterListType;



    typedef itk::UnaryFunctorImageFilter<ImageType,ImageType,
                    ShortToFloatTranslationFunctor<
                        ImageType::PixelType,
                        ImageType::PixelType> > DequantifyFilterType;


  itkNewMacro(Self)
  itkTypeMacro(TimeSeriesBuilder, otb::Application)

private:

  void DoInit()
  {
        SetName("TimeSeriesBuilder");
        SetDescription("Creates one image from all input images.");

        SetDocName("TimeSeriesBuilder");
        SetDocLongDescription("Creates one image from all input images.");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");

        AddParameter(ParameterType_InputFilenameList, "il", "The image files list");
        AddParameter(ParameterType_OutputImage, "out", "Image containing all bands from the image files list");

        AddParameter(ParameterType_Float, "deqval", "The de-quantification value to be used");
        SetDefaultParameterFloat("deqval", -1);
        MandatoryOff("deqval");

        SetDocExampleParameterValue("il", "image1.tif image2.tif");
        SetDocExampleParameterValue("out", "result.tif");
  }

  void DoUpdateParameters()
  {
  }
  void DoExecute()
  {
        m_ImageReaderList = ImageReaderListType::New();
        m_ImageSplitList = SplitFilterListType::New();
        float deqValue = GetParameterFloat("deqval");

        std::vector<std::string> imgsList = this->GetParameterStringList("il");

        if( imgsList.size()== 0 )
        {
            itkExceptionMacro("No input file set...");
        }

        ImageListType::Pointer allBandsList = ImageListType::New();
        for (const std::string& strImg : imgsList)
        {
            ImageReaderType::Pointer reader = getReader(strImg);
            ImageType::Pointer img = reader->GetOutput();
            img->UpdateOutputInformation();
            m_imagesList->PushBack(img);
            if(deqValue > 0) {
                m_deqFunctor = DequantifyFilterType::New();
                m_deqFunctor->GetFunctor().Initialize(deqValue, 0);
                m_deqFunctor->SetInput(img);
                m_deqFunctor->GetOutput()->SetNumberOfComponentsPerPixel(img->GetNumberOfComponentsPerPixel());
                img = m_deqFunctor->GetOutput();
                m_imagesList->PushBack(img);
            }

            VectorImageToImageListType::Pointer splitter = getSplitter(img);
            int nBands = img->GetNumberOfComponentsPerPixel();
            for(int i = 0; i<nBands; i++)
            {
                allBandsList->PushBack(splitter->GetOutput()->GetNthElement(i));
            }

        }
        m_bandsConcat = ImageListToVectorImageFilterType::New();
        m_bandsConcat->SetInput(allBandsList);
        SetParameterOutputImage("out", m_bandsConcat->GetOutput());
  }

  // get a reader from the file path
  ImageReaderType::Pointer getReader(const std::string& filePath) {
        ImageReaderType::Pointer reader = ImageReaderType::New();

        // set the file name
        reader->SetFileName(filePath);
        reader->UpdateOutputInformation();

        // add it to the list and return
        m_ImageReaderList->PushBack(reader);
        return reader;
  }
  VectorImageToImageListType::Pointer getSplitter(const ImageType::Pointer& image) {
      VectorImageToImageListType::Pointer imgSplit = VectorImageToImageListType::New();
      imgSplit->SetInput(image);
      imgSplit->UpdateOutputInformation();
      m_ImageSplitList->PushBack( imgSplit );
      return imgSplit;
  }


    ImageReaderListType::Pointer                m_ImageReaderList;
    SplitFilterListType::Pointer                m_ImageSplitList;
    ImageListToVectorImageFilterType::Pointer   m_bandsConcat;
    DequantifyFilterType::Pointer               m_deqFunctor;
    ImagesListType::Pointer                     m_imagesList;
};

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::TimeSeriesBuilder)


