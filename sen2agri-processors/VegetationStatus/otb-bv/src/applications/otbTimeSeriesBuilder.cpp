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
#include "otbBandMathImageFilter.h"
#include "MetadataHelperFactory.h"
#include "GlobalDefs.h"
#include "ImageResampler.h"

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
    typedef otb::ObjectList<DequantifyFilterType>              DeqFunctorListType;


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

        AddParameter(ParameterType_Int, "isflg", "Specifies if the time series is built of flags");
        SetDefaultParameterInt("isflg", 0);
        MandatoryOff("isflg");

        AddParameter(ParameterType_String, "main", "The image from the il that is used for the cutting other images");
        MandatoryOff("main");


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
        m_deqFunctorList = DeqFunctorListType::New();
        m_imagesList = ImagesListType::New();

        m_deqValue = GetParameterFloat("deqval");
        m_isFlagsTimeSeries = GetParameterInt("isflg");

        std::vector<std::string> imgsList = this->GetParameterStringList("il");
        if( imgsList.size()== 0 )
        {
            itkExceptionMacro("No input file set...");
        }

        // update the width, hight and origin if we have a main image
        updateRequiredImageSize();
        // keep the first image one that has the origin and dimmension of the main one (if it is the case)
        //imgsList = trimLeftInvalidPrds(imgsList);

        ImageListType::Pointer allBandsList = ImageListType::New();
        for (const std::string& strImg : imgsList)
        {
            ImageReaderType::Pointer reader = getReader(strImg);
            ImageType::Pointer img = reader->GetOutput();
            img->UpdateOutputInformation();

            // cut the image if we need to
            img = cutImage(img);

            // dequantify image if we need to
            img = dequantifyImage(img);

            m_imagesList->PushBack(img);

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

  ImageType::Pointer dequantifyImage(const ImageType::Pointer &img) {
      if(m_deqValue > 0) {
          DequantifyFilterType::Pointer deqFunctor = DequantifyFilterType::New();
          m_deqFunctorList->PushBack(deqFunctor);
          deqFunctor->GetFunctor().Initialize(m_deqValue, 0);
          deqFunctor->SetInput(img);
          int nComponents = img->GetNumberOfComponentsPerPixel();
          ImageType::Pointer newImg = deqFunctor->GetOutput();
          newImg->SetNumberOfComponentsPerPixel(nComponents);
          newImg->UpdateOutputInformation();
          return newImg;
      }
      return img;
  }

  ImageType::Pointer cutImage(const ImageType::Pointer &img) {
      if(m_bCutImages) {
          float imageWidth = img->GetLargestPossibleRegion().GetSize()[0] / m_scale;
          float imageHeight = img->GetLargestPossibleRegion().GetSize()[1] / m_scale;

          ImageType::PointType origin = img->GetOrigin();
          ImageType::PointType imageOrigin;
          //ImageType::SpacingType spacing = img->GetSpacing();
          imageOrigin[0] = origin[0];// + 0.5 * spacing[0] * (m_scale - 1.0);
          imageOrigin[1] = origin[1];// + 0.5 * spacing[1] * (m_scale - 1.0);

          if((imageWidth != m_imageWidth) || (imageHeight != m_imageHeight) ||
                  (m_imageOrigin[0] != imageOrigin[0]) || (m_imageOrigin[1] != imageOrigin[1])) {
              ImageResampler<ImageType, ImageType>::ResamplerPtr resampler = m_ImageResampler.getResampler(
                          img, m_scale, m_imageWidth, m_imageHeight,
                          m_isFlagsTimeSeries == 0 ? Interpolator_NNeighbor : Interpolator_Linear);
              ImageType::Pointer newImg = resampler->GetOutput();
              newImg->UpdateOutputInformation();
              return newImg;
          }
      }

      return img;

  }

  void updateRequiredImageSize() {
      m_bCutImages = false;
      m_imageWidth = 0;
      m_imageHeight = 0;
      m_scale = 1.0; //(float)m_pixSize / curRes;

      std::string mainImg;
      if (HasValue("main")) {
          mainImg = this->GetParameterString("main");
          m_bCutImages = true;
      } else {
          return;
      }

      ImageReaderType::Pointer reader = getReader(mainImg);
      reader->GetOutput()->UpdateOutputInformation();
      //float curRes = reader->GetOutput()->GetSpacing()[0];

      m_imageWidth = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[0] / m_scale;
      m_imageHeight = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[1] / m_scale;

      ImageType::PointType origin = reader->GetOutput()->GetOrigin();
      //ImageType::SpacingType spacing = reader->GetOutput()->GetSpacing();
      m_imageOrigin[0] = origin[0]; // + 0.5 * spacing[0] * (m_scale - 1.0);
      m_imageOrigin[1] = origin[1]; // + 0.5 * spacing[1] * (m_scale - 1.0);

  }

  std::vector<std::string> trimLeftInvalidPrds(std::vector<std::string> imgsList) {
      if(!m_bCutImages) {
          return imgsList;
      }
      std::vector<std::string> retImgsList;

      bool bFirstValidFound = false;
      for (const std::string& strImg : imgsList)
      {
          if(!bFirstValidFound) {
              ImageReaderType::Pointer reader = getReader(strImg);
              ImageType::Pointer img = reader->GetOutput();
              img->UpdateOutputInformation();
              float imageWidth = img->GetLargestPossibleRegion().GetSize()[0] / m_scale;
              float imageHeight = img->GetLargestPossibleRegion().GetSize()[1] / m_scale;

              ImageType::PointType origin = img->GetOrigin();
              ImageType::PointType imageOrigin;
              //ImageType::SpacingType spacing = img->GetSpacing();
              imageOrigin[0] = origin[0];// + 0.5 * spacing[0] * (m_scale - 1.0);
              imageOrigin[1] = origin[1];// + 0.5 * spacing[1] * (m_scale - 1.0);

              if((imageWidth == m_imageWidth) && (imageHeight == m_imageHeight) &&
                      (m_imageOrigin[0] == imageOrigin[0]) && (m_imageOrigin[1] == imageOrigin[1])) {
                  bFirstValidFound = true;
                  retImgsList.push_back(strImg);
              }
          } else {
                retImgsList.push_back(strImg);
          }
      }
      return retImgsList;
  }


    ImageReaderListType::Pointer                m_ImageReaderList;
    SplitFilterListType::Pointer                m_ImageSplitList;
    ImageListToVectorImageFilterType::Pointer   m_bandsConcat;
    DeqFunctorListType::Pointer                 m_deqFunctorList;
    ImagesListType::Pointer                     m_imagesList;

    float                                 m_pixSize;
    double                                m_imageWidth;
    double                                m_imageHeight;
    ImageType::PointType                  m_imageOrigin;
    bool m_bCutImages;
    ImageResampler<ImageType, ImageType>  m_ImageResampler;
    float                                 m_scale;
    float                                 m_deqValue;
    int                                   m_isFlagsTimeSeries;
};

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::TimeSeriesBuilder)


