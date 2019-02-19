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
#include "otbVectorImageToImageListFilter.h"

#include "../../../Common/Filters/RatioFilterVector.hxx"

namespace otb
{
template <class TInputImageType, class TOutputImageType>
class ITK_EXPORT BackscatterTemporalFeaturesFilter
  : public ImageListToImageFilter<TInputImageType, TOutputImageType>
{
public:
  /** Standard typedefs */
  typedef BackscatterTemporalFeaturesFilter          Self;
  typedef ImageListToImageFilter
      <TInputImageType, TOutputImageType>           Superclass;
  typedef itk::SmartPointer<Self>                   Pointer;
  typedef itk::SmartPointer<const Self>             ConstPointer;

  typedef TInputImageType                               InputImageType;
  typedef typename InputImageType::Pointer              InputImagePointerType;
  typedef typename InputImageType::PixelType            InputPixelType;
  typedef typename InputImageType::ValueType            InputPixelValueType;
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
  itkTypeMacro(BackscatterTemporalFeaturesFilter, ImageListToImageFilter);

  itkGetMacro(NoDataValue, InputPixelType);
  itkSetMacro(NoDataValue, InputPixelType);
  itkGetMacro(UseNoDataValue, bool);
  itkSetMacro(UseNoDataValue, bool);
  itkBooleanMacro(UseNoDataValue);

protected:
  BackscatterTemporalFeaturesFilter();
  ~BackscatterTemporalFeaturesFilter() override {}

  void GenerateInputRequestedRegion() override;
  void GenerateOutputInformation() override;
  void ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, itk::ThreadIdType threadId) override;

  /**PrintSelf method */
  void PrintSelf(std::ostream& os, itk::Indent indent) const override;


private:
  BackscatterTemporalFeaturesFilter(const Self &) = delete;
  void operator =(const Self&) = delete;

  InputPixelType       m_NoDataValue;
  bool                 m_UseNoDataValue;
};

template <class TInputImageType, class TOutputImageType>
BackscatterTemporalFeaturesFilter<TInputImageType, TOutputImageType>
::BackscatterTemporalFeaturesFilter()
 : m_NoDataValue(),
   m_UseNoDataValue()
{
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfRequiredOutputs(1);
}

template <class TInputImageType, class TOutputImageType>
void
BackscatterTemporalFeaturesFilter<TInputImageType, TOutputImageType>
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
BackscatterTemporalFeaturesFilter<TInputImageType, TOutputImageType>
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
BackscatterTemporalFeaturesFilter<TInputImageType, TOutputImageType>
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
  PrecisionType sqSum;
  PrecisionType mean;
  PrecisionType cvar;
  int count;

  outPix.SetSize(2);
  while (!outputIt.IsAtEnd())
    {
    sum = 0;
    sqSum = 0;
    count = 0;

    for (auto &it : inputIts)
      {
      auto inPix = it.Get();
      if ((!m_UseNoDataValue || inPix != m_NoDataValue) && !std::isnan(inPix))
        {
        sum += inPix;
        sqSum += inPix * inPix;
        count++;
        }
      ++it;
      }

    if (count > 0 && sum > 0)
      {
      mean = sum / count;
      if (count > 1)
        {
        auto dev = std::sqrt((sqSum - sum * mean) / (count - 1));
        cvar = dev / mean;
        }
      else
        {
        cvar = zero;
        }
      }
    else
      {
      mean = zero;
      cvar = zero;
      }

    outPix[0] = static_cast<OutputInternalPixelType>(mean);
    outPix[1] = static_cast<OutputInternalPixelType>(cvar);
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
BackscatterTemporalFeaturesFilter<TInputImageType, TOutputImageType>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

namespace Wrapper
{
class BackscatterTemporalFeatures : public Application
{
public:
    typedef BackscatterTemporalFeatures Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self);
    itkTypeMacro(BackscatterTemporalFeatures, otb::Application);

    typedef FloatImageType                                   InputImageType;
    typedef FloatVectorImageType                             OutputImageType;
    typedef otb::ImageList<FloatImageType>                   ImageListType;
    typedef otb::ImageFileReader<InputImageType>             ReaderType;
    typedef otb::ObjectList<ReaderType>                      ReaderListType;
    typedef otb::BackscatterTemporalFeaturesFilter
                <InputImageType, OutputImageType>            BackscatterTemporalFeaturesFilterType;
    typedef otb::RatioFilterVector
                <InputImageType, OutputImageType>            RatioFilterType;
    typedef otb::VectorImageToImageListFilter
                <OutputImageType, ImageListType>             VectorImageToImageListFilterType;

private:
    void DoInit() override
    {
        SetName("BackscatterTemporalFeatures");
        SetDescription("Computes the temporal features for a backscatter series");

        SetDocName("Backscatter Temporal Features");
        SetDocLongDescription("Computes the temporal features for a backscatter series.");
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

        AddParameter(ParameterType_Choice, "mode", "Execution mode");
        AddChoice("mode.simple", "Simple execution, VV or VH time series");
        AddChoice("mode.ratio", "Ratio computation, VV/VH ratio");

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

        m_BackscatterTemporalFeaturesFilter = BackscatterTemporalFeaturesFilterType::New();
        if (GetParameterString("mode") == "simple")
          {
          m_BackscatterTemporalFeaturesFilter->SetInput(m_ImageList);
          }
        else
          {
          m_RatioFilter = RatioFilterType::New();
          m_RatioFilter->SetInput(m_ImageList);
          m_RatioFilter->SetNoDataValue(0);
          m_RatioFilter->SetUseNoDataValue(true);

          m_VectorImageToImageListFilter = VectorImageToImageListFilterType::New();
          m_VectorImageToImageListFilter->SetInput(m_RatioFilter->GetOutput());
          m_BackscatterTemporalFeaturesFilter->SetInput(m_VectorImageToImageListFilter->GetOutput());
          }

        if (HasValue("bv"))
          {
          m_BackscatterTemporalFeaturesFilter->UseNoDataValueOn();
          m_BackscatterTemporalFeaturesFilter->SetNoDataValue(GetParameterFloat("bv"));
          }

        SetParameterOutputImage("out", m_BackscatterTemporalFeaturesFilter->GetOutput());
    }

    ImageListType::Pointer                               m_ImageList;
    ReaderListType::Pointer                              m_Readers;
    BackscatterTemporalFeaturesFilterType::Pointer   m_BackscatterTemporalFeaturesFilter;
    RatioFilterType::Pointer                             m_RatioFilter;
    VectorImageToImageListFilterType::Pointer            m_VectorImageToImageListFilter;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::BackscatterTemporalFeatures)
