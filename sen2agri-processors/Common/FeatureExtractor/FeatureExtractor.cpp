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
#include "otbWrapperInputImageListParameter.h"

#include "otbStatisticsXMLFileReader.h"
#include "../MultiModelImageClassifier/otbMultiModelImageClassificationFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbImageToVectorImageCastFilter.h"
#include "otbWrapperTypes.h"
#include "otbObjectList.h"
#include "otbVectorImage.h"
#include "otbImageList.h"

#include "../Filters/CropTypePreprocessing.h"

template<class TInputImage, class TOutputImage = TInputImage>
class ITK_EXPORT MaskedScaleImageFilter
  : public itk::ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  typedef MaskedScaleImageFilter                               Self;
  typedef itk::ImageToImageFilter<TInputImage, TOutputImage>  Superclass;
  typedef itk::SmartPointer<Self>                             Pointer;
  typedef itk::SmartPointer<const Self>                       ConstPointer;

  itkNewMacro(Self)

  itkTypeMacro(MaskedScaleImageFilter, ImageToImageFilter)

  /** Template related typedefs */
  typedef TInputImage InputImageType;
  typedef TOutputImage OutputImageType;

  typedef typename InputImageType::Pointer  InputImagePointerType;
  typedef typename OutputImageType::Pointer OutputImagePointerType;

  typedef typename InputImageType::PixelType          InputPixelType;
  typedef typename OutputImageType::InternalPixelType InputInternalPixelType;

  typedef typename OutputImageType::PixelType         OutputPixelType;
  typedef typename OutputImageType::InternalPixelType OutputInternalPixelType;
  typedef typename OutputImageType::RegionType        OutputImageRegionType;

  /** ImageDimension constant */
  itkStaticConstMacro(OutputImageDimension, unsigned int, TOutputImage::ImageDimension);

  itkSetMacro(NoDataValue, InputInternalPixelType)
  itkGetMacro(NoDataValue, InputInternalPixelType)

  itkSetMacro(Scale, InputPixelType)
  itkGetMacro(Scale, InputPixelType)

protected:
  MaskedScaleImageFilter()
  {
    this->SetNumberOfRequiredInputs(1);
  }

  virtual ~MaskedScaleImageFilter()
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

    OutputPixelType outputPixel(output->GetVectorLength());

    outputIt.GoToBegin();
    while (!outputIt.IsAtEnd())
      {
      const auto &inputPixel = inputIt.Get();
      for (int i = 0; i < outputPixel.Size(); i++)
        {
        if (inputPixel[i] != m_NoDataValue)
          {
          outputPixel[i] = inputPixel[i] * m_Scale[i];
          }
        else
          {
          outputPixel[i] = m_NoDataValue;
          }
        }
      outputIt.Set(outputPixel);

      ++inputIt;
      ++outputIt;
      progress.CompletedPixel();
      }
  }

private:
  MaskedScaleImageFilter(const Self &) = delete;
  void operator =(const Self&) = delete;

  InputInternalPixelType    m_NoDataValue;
  InputPixelType            m_Scale;
};

namespace otb
{

namespace Wrapper
{
class FeatureExtractor : public Application
{
public:
    typedef FeatureExtractor Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(FeatureExtractor, otb::Application)

    typedef Int16VectorImageType                                                     OutputImageType;
    typedef MaskedScaleImageFilter<FloatVectorImageType, Int16VectorImageType>       MaskedScaleImageFilterType;

private:
    void DoInit() override
    {
        SetName("FeatureExtractor");
        SetDescription("Build the statistics from a set of tiles");

        SetDocName("FeatureExtractor");
        SetDocLongDescription("Build the statistics from a set of tiles.");
        SetDocLimitations("None");
        SetDocAuthors("LN");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Vector);

        AddParameter(ParameterType_InputFilenameList, "il", "Input descriptors");
        SetParameterDescription("il", "The list of descriptors. They must be sorted by tiles.");

        AddParameter(ParameterType_OutputImage, "out", "Output Image");
        SetParameterDescription("out", "Output image");
        SetParameterOutputImagePixelType("out", ImagePixelType_int16);

        AddParameter(ParameterType_StringList, "sp", "Temporal sampling rate");
        MandatoryOff("sp");

        AddParameter(ParameterType_Choice, "mode", "Mode");
        SetParameterDescription("mode", "Specifies the choice of output dates (default: resample)");
        AddChoice("mode.resample", "Specifies the temporal resampling mode");
        AddChoice("mode.gapfill", "Specifies the gapfilling mode");
        AddChoice("mode.gapfillmain", "Specifies the gapfilling mode, but only use non-main series "
                                      "products to fill in the main one");
        SetParameterString("mode", "resample");

        AddParameter(ParameterType_Float, "pixsize", "The size of a pixel, in meters");
        SetDefaultParameterFloat("pixsize", 10.0); // The default value is 10 meters
        SetMinimumParameterFloatValue("pixsize", 1.0);
        MandatoryOff("pixsize");

        AddParameter(ParameterType_String,
                     "mission",
                     "The main raster series that will be used. By default, SENTINEL is used");
        MandatoryOff("mission");

        AddParameter(
            ParameterType_Empty, "rededge", "Include Sentinel-2 vegetation red edge bands");
        MandatoryOff("rededge");

        AddRAMParameter();

        SetDocExampleParameterValue("il", "image1.xml image2.xml");
        SetDocExampleParameterValue("out", "features.tif");
    }

    void DoUpdateParameters() override
    {
    }

    void DoExecute() override
    {
        // Get the list of input files
        const std::vector<std::string> &descriptors = this->GetParameterStringList("il");
        if (descriptors.size() == 0) {
            itkExceptionMacro("No input file set...");
        }

        // get the required pixel size
        auto pixSize = this->GetParameterFloat("pixsize");
        // get the main mission
        std::string mission = SENTINEL;
        if (HasValue("mission")) {
            mission = this->GetParameterString("mission");
        }

        TileData td;
        m_Preprocessor = CropTypePreprocessing::New();
        m_Preprocessor->SetPixelSize(pixSize);
        m_Preprocessor->SetMission(mission);
        if (GetParameterEmpty("rededge")) {
            m_Preprocessor->SetIncludeRedEdge(true);
        }

        // compute the desired size of the processed rasters
        m_Preprocessor->updateRequiredImageSize(descriptors, 0, descriptors.size(), td);
        m_Preprocessor->Build(descriptors.begin(), descriptors.end(), td);

        TemporalResamplingMode resamplingMode = TemporalResamplingMode::Resample;
        const auto &modeStr = GetParameterString("mode");
        if (modeStr == "gapfill") {
            resamplingMode = TemporalResamplingMode::GapFill;
        } else if (modeStr == "gapfillmain") {
            resamplingMode = TemporalResamplingMode::GapFillMainMission;
        }

        std::vector<SensorPreferences> sp;
        if (HasValue("sp")) {
            const auto &spValues = GetParameterStringList("sp");
            sp = parseSensorPreferences(spValues);
        } else {
            sp.emplace_back(SensorPreferences{ "SENTINEL", 0, 10 });
            sp.emplace_back(SensorPreferences{ "SPOT", 1, 5 });
            sp.emplace_back(SensorPreferences{ "LANDSAT", 2, 16 });
        }

        auto preprocessors = CropTypePreprocessingList::New();
        preprocessors->PushBack(m_Preprocessor);

        const auto &sensorOutDays = getOutputDays(preprocessors, resamplingMode, mission, sp);
        auto output = m_Preprocessor->GetOutput(sensorOutDays);
        output->UpdateOutputInformation();

        itk::VariableLengthVector<float> scaleValues(output->GetNumberOfComponentsPerPixel());

        scaleValues.Fill(1.0f);

        auto images = 0;
        for (const auto &sensor :  sensorOutDays) {
            images += sensor.days.size();
        }
        auto bands = output->GetNumberOfComponentsPerPixel() / images;
        for (size_t i = 0; i < images; i++) {
            scaleValues[i * bands + bands - 3] = 3000;
            scaleValues[i * bands + bands - 2] = 3000;
        }

        m_MaskedScaleFilter = MaskedScaleImageFilterType::New();
        m_MaskedScaleFilter->SetScale(scaleValues);
        m_MaskedScaleFilter->SetInput(output);
        m_MaskedScaleFilter->SetNoDataValue(-10000);

        SetParameterOutputImage<OutputImageType>("out", m_MaskedScaleFilter->GetOutput());
    }

    CropTypePreprocessing::Pointer          m_Preprocessor;
    MaskedScaleImageFilterType::Pointer     m_MaskedScaleFilter;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::FeatureExtractor)
