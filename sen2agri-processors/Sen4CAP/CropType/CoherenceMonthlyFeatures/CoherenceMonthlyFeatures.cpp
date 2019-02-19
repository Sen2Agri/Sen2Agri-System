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

float quantile(size_t k, size_t q, std::vector<float> &values)
{
    auto n = values.size();
    auto p = static_cast<float>(k) / q;

    // https://en.wikipedia.org/wiki/Quantile#Estimating_quantiles_from_a_sample
    // R-4
    auto h = n * p;
    auto hl = static_cast<size_t>(h);
    if (p < 1.0f / n) {
        return *std::min_element(values.begin(), values.end());
    }
    if (p >= 1 || hl + 1 == n) {
        return *std::max_element(values.begin(), values.end());
    }
    std::sort(values.begin(), values.end());
    if (h == hl) {
        return values[hl - 1];
    }
    return values[hl - 1] + (h - hl) * (values[hl] - values[hl - 1]);
}

namespace otb
{

template <class TInputImageType, class TOutputImageType>
class ITK_EXPORT CoherenceMonthlyFeaturesFilter
  : public ImageListToImageFilter<TInputImageType, TOutputImageType>
{
public:
  /** Standard typedefs */
  typedef CoherenceMonthlyFeaturesFilter          Self;
  typedef ImageListToImageFilter
      <TInputImageType, TOutputImageType>           Superclass;
  typedef itk::SmartPointer<Self>                   Pointer;
  typedef itk::SmartPointer<const Self>             ConstPointer;

  typedef TInputImageType                               InputImageType;
  typedef typename InputImageType::Pointer              InputImagePointerType;
  typedef typename InputImageType::PixelType            InputPixelType;
  typedef ImageList<InputImageType>                     InputImageListType;
  typedef TOutputImageType                              OutputImageType;
  typedef typename OutputImageType::PixelType           OutputPixelType;
  typedef typename OutputImageType::InternalPixelType   OutputInternalPixelType;
  typedef typename OutputImageType::Pointer             OutputImagePointerType;
  typedef double                                        PrecisionType;

  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef typename InputImageType::RegionType  InputImageRegionType;


  /** Type macro */
  itkNewMacro(Self);

  /** Creation through object factory macro */
  itkTypeMacro(CoherenceMonthlyFeaturesFilter, ImageListToImageFilter);

  itkGetMacro(NoDataValue, InputPixelType);
  itkSetMacro(NoDataValue, InputPixelType);
  itkGetMacro(UseNoDataValue, bool);
  itkSetMacro(UseNoDataValue, bool);
  itkBooleanMacro(UseNoDataValue);

protected:
  CoherenceMonthlyFeaturesFilter();
  ~CoherenceMonthlyFeaturesFilter() override {}

  void GenerateInputRequestedRegion() override;
  void GenerateOutputInformation() override;
  void ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, itk::ThreadIdType threadId) override;

  /**PrintSelf method */
  void PrintSelf(std::ostream& os, itk::Indent indent) const override;


private:
  CoherenceMonthlyFeaturesFilter(const Self &) = delete;
  void operator =(const Self&) = delete;

  InputPixelType       m_NoDataValue;
  bool                 m_UseNoDataValue;
};

template <class TInputImageType, class TOutputImageType>
CoherenceMonthlyFeaturesFilter<TInputImageType, TOutputImageType>
::CoherenceMonthlyFeaturesFilter()
 : m_NoDataValue(),
   m_UseNoDataValue()
{
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfRequiredOutputs(1);
}

template <class TInputImageType, class TOutputImageType>
void
CoherenceMonthlyFeaturesFilter<TInputImageType, TOutputImageType>
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
CoherenceMonthlyFeaturesFilter<TInputImageType, TOutputImageType>
::GenerateOutputInformation()
{
  if (this->GetOutput())
    {
    if (this->GetInput()->Size() > 0)
      {
      this->GetOutput()->CopyInformation(this->GetInput()->GetNthElement(0));
      this->GetOutput()->SetLargestPossibleRegion(this->GetInput()->GetNthElement(0)->GetLargestPossibleRegion());
      this->GetOutput()->SetNumberOfComponentsPerPixel(2);
      }
    }
}

template <class TInputImageType, class TOutputImageType>
void
CoherenceMonthlyFeaturesFilter<TInputImageType, TOutputImageType>
::ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, itk::ThreadIdType threadId)
{
  auto inputPtr = this->GetInput();
  auto inputImages = this->GetInput()->Size();

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

  InputPixelType zero = m_UseNoDataValue ? m_NoDataValue : 0;

  OutputPixelType outPix;
  PrecisionType sum;
  PrecisionType mean;
  InputPixelType quantile10;
  std::vector<InputPixelType> values;
  int count;

  values.reserve(inputImages);
  outPix.SetSize(2);
  while (!outputIt.IsAtEnd())
    {
    sum = 0;
    count = 0;
    values.clear();

    for (auto &it : inputIts)
      {
      auto inPix = it.Get();
      if ((!m_UseNoDataValue || inPix != m_NoDataValue) && !std::isnan(inPix))
        {
        sum += inPix;
        values.push_back(inPix);
        count++;
        }
      ++it;
      }

    if (count > 0)
      {
      mean = sum / count;
      quantile10 = quantile(1, 10, values);
      }
    else
      {
      mean = zero;
      quantile10 = zero;
      }

    outPix[0] = static_cast<OutputInternalPixelType>(mean);
    outPix[1] = quantile10;
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
CoherenceMonthlyFeaturesFilter<TInputImageType, TOutputImageType>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

namespace Wrapper
{
class CoherenceMonthlyFeatures : public Application
{
public:
    typedef CoherenceMonthlyFeatures Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self);
    itkTypeMacro(CoherenceMonthlyFeatures, otb::Application);

    typedef FloatImageType                                   InputImageType;
    typedef FloatVectorImageType                             OutputImageType;
    typedef otb::ImageList<FloatImageType>                   ImageListType;
    typedef otb::ImageFileReader<InputImageType>             ReaderType;
    typedef otb::ObjectList<ReaderType>                      ReaderListType;
    typedef otb::CoherenceMonthlyFeaturesFilter
                <InputImageType, OutputImageType>            CoherenceMonthlyFeaturesFilterType;

private:
    void DoInit() override
    {
        SetName("CoherenceMonthlyFeatures");
        SetDescription("Computes the monthly coherence features");

        SetDocName("Backscatter Temporal Features");
        SetDocLongDescription("Computes the monthly coherence features.");
        SetDocLimitations("None");
        SetDocAuthors("LN");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Raster);

        AddParameter(ParameterType_InputFilenameList, "il", "Input images");
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
        const auto &inImages = GetParameterStringList("il");

        m_ImageList = ImageListType::New();
        m_Readers = ReaderListType::New();

        for (const auto &file : inImages)
        {
            auto reader = ReaderType::New();
            reader->SetFileName(file);
            reader->UpdateOutputInformation();
            m_Readers->PushBack(reader);

            m_ImageList->PushBack(reader->GetOutput());
        }

        m_CoherenceMonthlyFeaturesFilter = CoherenceMonthlyFeaturesFilterType::New();
        m_CoherenceMonthlyFeaturesFilter->SetInput(m_ImageList);

        if (HasValue("bv"))
        {
          m_CoherenceMonthlyFeaturesFilter->UseNoDataValueOn();
          m_CoherenceMonthlyFeaturesFilter->SetNoDataValue(GetParameterFloat("bv"));
        }

        // Output Image
        SetParameterOutputImage("out", m_CoherenceMonthlyFeaturesFilter->GetOutput());
    }

    ImageListType::Pointer                           m_ImageList;
    ReaderListType::Pointer                          m_Readers;
    CoherenceMonthlyFeaturesFilterType::Pointer    m_CoherenceMonthlyFeaturesFilter;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::CoherenceMonthlyFeatures)
