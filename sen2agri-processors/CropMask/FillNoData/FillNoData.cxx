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

#include "otbVectorImageToImageListFilter.h"
#include "otbImageList.h"
#include "otbStreamingStatisticsVectorImageFilterEx.h"

template<class TInputImage, class TOutputImage = TInputImage>
class ITK_EXPORT FillNoDataImageFilter
  : public itk::ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  typedef FillNoDataImageFilter                               Self;
  typedef itk::ImageToImageFilter<TInputImage, TOutputImage>  Superclass;
  typedef itk::SmartPointer<Self>                             Pointer;
  typedef itk::SmartPointer<const Self>                       ConstPointer;

  itkNewMacro(Self)

  itkTypeMacro(FillNoDataImageFilter, ImageToImageFilter)

  /** Template related typedefs */
  typedef TInputImage InputImageType;
  typedef TOutputImage OutputImageType;

  typedef typename InputImageType::Pointer  InputImagePointerType;
  typedef typename OutputImageType::Pointer OutputImagePointerType;

  typedef typename InputImageType::PixelType         InputPixelType;

  typedef typename OutputImageType::PixelType         OutputPixelType;
  typedef typename OutputImageType::InternalPixelType OutputInternalPixelType;
  typedef typename OutputImageType::RegionType        OutputImageRegionType;

  /** ImageDimension constant */
  itkStaticConstMacro(OutputImageDimension, unsigned int, TOutputImage::ImageDimension);

  itkSetMacro(NoDataValue, int)
  itkGetMacro(NoDataValue, int)

  itkSetMacro(ReplacementValues, InputPixelType)
  itkGetMacro(ReplacementValues, InputPixelType)

protected:
  FillNoDataImageFilter()
  {
    this->SetNumberOfRequiredInputs(1);
  }

  virtual ~FillNoDataImageFilter()
  {
  }

  virtual void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType threadId)
  {
    OutputImagePointerType output = this->GetOutput();

    itk::ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

    typename InputImageType::RegionType inputRegionForThread;
    this->CallCopyOutputRegionToInputRegion(inputRegionForThread, outputRegionForThread);

    typedef itk::ImageRegionIterator<InputImageType>  InputIteratorType;
    typedef itk::ImageRegionIterator<OutputImageType> OutputIteratorType;

    InputIteratorType inputIt(const_cast<InputImageType *>(this->GetInput()), inputRegionForThread);
    OutputIteratorType outputIt(output, outputRegionForThread);

    typename OutputImageType::PixelType outputPixel(output->GetVectorLength());

    outputIt.GoToBegin();
    while (!outputIt.IsAtEnd())
      {
      outputPixel = inputIt.Get();
      for (int i = 0; i < outputPixel.Size(); i++)
        {
        if (outputPixel[i] == m_NoDataValue)
          {
          outputPixel[i] = m_ReplacementValues[i];
          }
        }
      outputIt.Set(outputPixel);

      ++inputIt;
      ++outputIt;
      progress.CompletedPixel();
      }
  }

private:
  FillNoDataImageFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  int                       m_NoDataValue;
  InputPixelType            m_ReplacementValues;
};

typedef float                                PixelType;
typedef otb::VectorImage<PixelType>          ImageType;
typedef ImageType::PixelType                 PixelValueType;
typedef otb::Image<PixelType>                InternalImageType;

typedef otb::ImageList<InternalImageType>                               ImageListType;
typedef otb::VectorImageToImageListFilter<ImageType, ImageListType>     VectorImageToImageListFilterType;
typedef otb::StreamingStatisticsVectorImageFilterEx<ImageType>          StreamingStatisticsVectorImageFilterType;

typedef FillNoDataImageFilter<ImageType>                                FillNoDataImageFilterType;

typedef InternalImageType::SizeType          ReferenceSizeType;
typedef InternalImageType::SizeValueType     ReferenceSizeValueType;

namespace otb
{
namespace Wrapper
{
class FillNoData : public Application
{
public:
  typedef FillNoData Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  itkNewMacro(Self)

  itkTypeMacro(FillNoData, otb::Application)

private:
  void DoInit()
  {
    SetName("FillNoData");
    SetDescription("Fills \"no data\" band values with the mean of the corresponding bands.");

    SetDocName("FillNoData");
    SetDocLongDescription("This application computes the average for each of the bands and replaces \"no data\" values with that average.");
    SetDocLimitations("None");
    SetDocAuthors("LN");
    SetDocSeeAlso(" ");

    AddDocTag(Tags::Filter);
    AddDocTag(Tags::Raster);

    AddParameter(ParameterType_InputImage, "in", "The input image");
    AddParameter(ParameterType_Int, "bv", "The background value");

    AddParameter(ParameterType_OutputImage, "out", "The output image");

    SetDefaultParameterInt("bv", 0);

    SetDocExampleParameterValue("in", "ndvi.tif");
    SetDocExampleParameterValue("bv", "-10000");
    SetDocExampleParameterValue("out", "ndvi_filled.tif");
  }

  void DoUpdateParameters()
  {
  }

  void DoExecute()
  {
    ImageType::Pointer inputImage = GetParameterFloatVectorImage("in");
    inputImage->UpdateOutputInformation();

    int noDataValue = GetParameterInt("bv");

    StreamingStatisticsVectorImageFilterType::Pointer streamingStatisticsVectorImageFilter = StreamingStatisticsVectorImageFilterType::New();

    unsigned int pixelSize = inputImage->GetVectorLength();
    PixelValueType means(pixelSize);

    streamingStatisticsVectorImageFilter->SetInput(inputImage);
    streamingStatisticsVectorImageFilter->SetIgnoreUserDefinedValue(true);
    streamingStatisticsVectorImageFilter->SetUserIgnoredValue(noDataValue);
    streamingStatisticsVectorImageFilter->SetEnableSecondOrderStats(false);
    streamingStatisticsVectorImageFilter->SetEnableMinMax(false);
    streamingStatisticsVectorImageFilter->Update();

    means = streamingStatisticsVectorImageFilter->GetMean();
    for (unsigned int i = 0; i < pixelSize; i++)
      {
      if (std::isnan(means[i]))
        {
        otbAppLogCRITICAL("Found NaN band mean value for band" << i);
        means[i] = 0;
        }
      }
    otbAppLogINFO("Band means: " << means);

    m_FillNoDataImageFilter = FillNoDataImageFilterType::New();
    m_FillNoDataImageFilter->SetInput(inputImage);
    m_FillNoDataImageFilter->SetNoDataValue(noDataValue);
    m_FillNoDataImageFilter->SetReplacementValues(means);

    SetParameterOutputImage("out", m_FillNoDataImageFilter->GetOutput());
  }

  FillNoDataImageFilterType::Pointer                        m_FillNoDataImageFilter;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::FillNoData)
