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

/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbWrapperInputImageListParameter.h"

#include "otbMultiToMonoChannelExtractROI.h"
#include "otbImageToVectorImageCastFilter.h"
#include "otbWrapperTypes.h"
#include "otbObjectList.h"
#include "otbVectorImage.h"
#include "otbImageList.h"
#include "otbImageListToImageFilter.h"

namespace otb
{

template <class TInputImageType, class TOutputImageType>
class ITK_EXPORT StandardDeviationFilter
  : public ImageListToImageFilter<TInputImageType, TOutputImageType>
{
public:
  /** Standard typedefs */
  typedef StandardDeviationFilter          Self;
  typedef ImageListToImageFilter
      <TInputImageType, TOutputImageType>           Superclass;
  typedef itk::SmartPointer<Self>                   Pointer;
  typedef itk::SmartPointer<const Self>             ConstPointer;

  typedef TInputImageType                           InputImageType;
  typedef typename InputImageType::Pointer          InputImagePointerType;
  typedef typename InputImageType::PixelType        InputPixelType;
  typedef typename InputPixelType::ValueType        InputPixelValueType;
  typedef ImageList<InputImageType>                 InputImageListType;
  typedef TOutputImageType                          OutputImageType;
  typedef typename OutputImageType::PixelType       OutputPixelType;
  typedef typename OutputImageType::Pointer         OutputImagePointerType;
  typedef double                                    PrecisionType;

  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef typename InputImageType::RegionType  InputImageRegionType;


  /** Type macro */
  itkNewMacro(Self);

  /** Creation through object factory macro */
  itkTypeMacro(StandardDeviationFilter, ImageListToImageFilter);

  itkGetMacro(NoDataValue, InputPixelValueType);
  itkSetMacro(NoDataValue, InputPixelValueType);
  itkGetMacro(UseNoDataValue, bool);
  itkSetMacro(UseNoDataValue, bool);
  itkBooleanMacro(UseNoDataValue);

protected:
  StandardDeviationFilter();
  ~StandardDeviationFilter() override {}

  void GenerateInputRequestedRegion() override;
  void GenerateOutputInformation() override;
  void ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, itk::ThreadIdType threadId) override;

  /**PrintSelf method */
  void PrintSelf(std::ostream& os, itk::Indent indent) const override;


private:
  StandardDeviationFilter(const Self &) = delete;
  void operator =(const Self&) = delete;

  InputPixelValueType       m_NoDataValue;
  bool                      m_UseNoDataValue;
};

template <class TInputImageType, class TOutputImageType>
StandardDeviationFilter<TInputImageType, TOutputImageType>
::StandardDeviationFilter()
 : m_NoDataValue(),
   m_UseNoDataValue()
{
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfRequiredOutputs(1);
}

template <class TInputImageType, class TOutputImageType>
void
StandardDeviationFilter<TInputImageType, TOutputImageType>
::GenerateInputRequestedRegion(void)
{
  auto inputPtr = this->GetInput();
  for (auto inputListIt = inputPtr->Begin(); inputListIt != inputPtr->End(); ++inputListIt)
    {
    inputListIt.Get()->SetRequestedRegion(this->GetOutput()->GetRequestedRegion());
    }
}

template <class TInputImageType, class TOutputImageType>
void
StandardDeviationFilter<TInputImageType, TOutputImageType>
::GenerateOutputInformation()
{
  if (this->GetOutput())
    {
    if (this->GetInput()->Size() > 0)
      {
      this->GetOutput()->CopyInformation(this->GetInput()->GetNthElement(0));
      this->GetOutput()->SetLargestPossibleRegion(this->GetInput()->GetNthElement(0)->GetLargestPossibleRegion());
      this->GetOutput()->SetNumberOfComponentsPerPixel(1);
      }
    }
}

template <class TInputImageType, class TOutputImageType>
void
StandardDeviationFilter<TInputImageType, TOutputImageType>
::ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, itk::ThreadIdType threadId)
{
  auto inputPtr = this->GetInput();
  auto inputImages = this->GetInput()->Size();
  auto bands = this->GetOutput()->GetNumberOfComponentsPerPixel();

  OutputImagePointerType  outputPtr = this->GetOutput();

  typedef itk::ImageRegionConstIteratorWithIndex<InputImageType> InputIteratorType;
  typedef itk::ImageRegionIteratorWithIndex<OutputImageType>     OutputIteratorType;

  itk::ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  OutputIteratorType outputIt(outputPtr, outputRegionForThread);
  outputIt.GoToBegin();

  std::vector<InputIteratorType> inputIts;
  inputIts.reserve(inputImages);
  for (auto inputListIt = inputPtr->Begin(); inputListIt != inputPtr->End(); ++inputListIt)
    {
    InputIteratorType inputIt(inputListIt.Get(), outputRegionForThread);
    inputIts.emplace_back(std::move(inputIt));
    inputIts.back().GoToBegin();
    }

  InputPixelValueType zero = m_UseNoDataValue ? m_NoDataValue : 0;

  OutputPixelType outPix;
  PrecisionType sum;
  PrecisionType sqSum;
  PrecisionType dev;
  int count;

  while (!outputIt.IsAtEnd())
    {
    sum = 0;
    sqSum = 0;
    count = 0;

    for (auto &it : inputIts)
      {
      const auto &inPix = it.Get();
      for (unsigned int i = 0; i < bands; i++)
        {
        if ((!m_UseNoDataValue || inPix[i] != m_NoDataValue) && !std::isnan(inPix[i]))
          {
          sum += inPix[i];
          sqSum += inPix[i] * inPix[i];
          count++;
          }
        }
      ++it;
      }


    if (count > 1)
      {
      auto mean = sum / count;
      dev = std::sqrt((sqSum - sum * mean) / (count - 1));
      }
    else
      {
      dev = zero;
      }

    outPix = static_cast<OutputPixelType>(dev);
    outputIt.Set(outPix);
    ++outputIt;
    progress.CompletedPixel();
    }
}

/**
 * PrintSelf Method
 */
template <class TInputImageType, class TOutputImageType>
void
StandardDeviationFilter<TInputImageType, TOutputImageType>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

namespace Wrapper
{
class StandardDeviation : public Application
{
public:
    typedef StandardDeviation Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self);
    itkTypeMacro(StandardDeviation, otb::Application);

    typedef Int16VectorImageType                                          OutputImageType;
    typedef otb::ImageList<FloatVectorImageType>                          ImageListType;
    typedef otb::StandardDeviationFilter
                <FloatVectorImageType, FloatImageType>                    StandardDeviationFilterType;

private:
    void DoInit() override
    {
        SetName("StandardDeviation");
        SetDescription("Computes the standard deviation of a time series");

        SetDocName("Backscatter Temporal Features");
        SetDocLongDescription("Computes the standard deviation of a time series.");
        SetDocLimitations("None");
        SetDocAuthors("LN");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Raster);

        AddParameter(ParameterType_InputImageList, "il", "Input images");
        SetParameterDescription("il", "The list of input images");

        AddParameter(ParameterType_Float, "bv", "Background value");
        SetParameterDescription("bv", "Background value to ignore in computation.");
        SetDefaultParameterFloat("bv", 0.);
        MandatoryOff("bv");

        AddParameter(ParameterType_OutputImage, "out", "Output image");
        SetParameterDescription("out", "Output image.");

        AddRAMParameter();

        SetDocExampleParameterValue("il", "image1.tif image2.tif");
        SetDocExampleParameterValue("out", "output.tif");
    }

    void DoUpdateParameters() override
    {
    }

    void DoExecute() override
    {
        const auto inImages = GetParameterImageList("il");

        m_ImageList = ImageListType::New();

        for (unsigned int i = 0; i < inImages->Size(); i++)
        {
            const auto inImage = inImages->GetNthElement(i);
            m_ImageList->PushBack(inImage);
        }

        m_StandardDeviationFilter = StandardDeviationFilterType::New();
        m_StandardDeviationFilter->SetInput(m_ImageList);

        if (HasValue("bv"))
        {
          m_StandardDeviationFilter->UseNoDataValueOn();
          m_StandardDeviationFilter->SetNoDataValue(GetParameterFloat("bv"));
        }

        // Output Image
        SetParameterOutputImage("out", m_StandardDeviationFilter->GetOutput());
    }

    ImageListType::Pointer                       m_ImageList;
    StandardDeviationFilterType::Pointer         m_StandardDeviationFilter;
    std::string                                  m_OutputProjectionRef;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::StandardDeviation)
