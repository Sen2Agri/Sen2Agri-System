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

#include "otbPCAImageFilter.h"
#include "otbVectorImage.h"
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

typedef float                                PixelValueType;
typedef otb::VectorImage<PixelValueType, 2>  ImageType;

typedef otb::PCAImageFilter<ImageType, ImageType, otb::Transform::FORWARD>  PCAFilterType;
typedef otb::StreamingStatisticsVectorImageFilterEx<ImageType>              StreamingStatisticsVectorImageFilterType;
typedef FillNoDataImageFilter<ImageType>                                    FillNoDataImageFilterType;

namespace otb
{

namespace Wrapper
{

class PrincipalComponentAnalysis : public Application
{
public:
    typedef PrincipalComponentAnalysis Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)
    itkTypeMacro(PrincipalComponentAnalysis, otb::Application)

    private:
        void DoInit()
    {
        SetName("PrincipalComponentAnalysis");
        SetDescription("Performs Principal Component Analysis on the input image.");

        SetDocName("PrincipalComponentAnalysis");
        SetDocLongDescription("Performs PCA on an image. Has support for ignoring a background value.");
        SetDocLimitations("None");
        SetDocAuthors("LN");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Vector);

        AddParameter(ParameterType_InputImage, "in", "Input Image");
        AddParameter(ParameterType_Int, "bv", "Background Value");
        MandatoryOff("bv");
        AddParameter(ParameterType_Int, "nbcomp", "Number of Components");
        AddParameter(ParameterType_OutputImage, "out", "Output Image");

        AddRAMParameter();

        SetDefaultParameterInt("nbcomp", 0);

        SetDocExampleParameterValue("in", "cupriteSubHsi.tif");
        SetDocExampleParameterValue("out", "pca.tif");
    }

    void DoUpdateParameters()
    {
        m_pcaFilter = PCAFilterType::New();
        m_fillNoDataImageFilter = FillNoDataImageFilterType::New();
    }

    void DoExecute()
    {
        int nbcomp = GetParameterInt("nbcomp");
        int noDataValue = GetParameterInt("bv");

        ImageType::Pointer inputImage = GetParameterFloatVectorImage("in");

        m_pcaFilter->SetNumberOfPrincipalComponentsRequired(nbcomp);

        if (HasValue("bv")) {
            StreamingStatisticsVectorImageFilterType::Pointer streamingStatisticsVectorImageFilter = StreamingStatisticsVectorImageFilterType::New();
            streamingStatisticsVectorImageFilter->SetInput(inputImage);
            streamingStatisticsVectorImageFilter->SetIgnoreUserDefinedValue(true);
            streamingStatisticsVectorImageFilter->SetUserIgnoredValue(noDataValue);
            streamingStatisticsVectorImageFilter->SetEnableSecondOrderStats(false);
            streamingStatisticsVectorImageFilter->SetEnableMinMax(false);
            streamingStatisticsVectorImageFilter->Update();

            StreamingStatisticsVectorImageFilterType::RealPixelType means = streamingStatisticsVectorImageFilter->GetMean();
            for (unsigned int i = 0; i < means.Size(); i++)
            {
                if (std::isnan(means[i]))
                {
                    otbAppLogCRITICAL("Found NaN band mean value for band" << i);
                    means[i] = 0;
                }
            }
            otbAppLogINFO("Band means: " << means);

            m_fillNoDataImageFilter = FillNoDataImageFilterType::New();
            m_fillNoDataImageFilter->SetInput(inputImage);
            m_fillNoDataImageFilter->SetNoDataValue(noDataValue);
            m_fillNoDataImageFilter->SetReplacementValues(means);

            m_pcaFilter->SetInput(m_fillNoDataImageFilter->GetOutput());
        } else {
            m_pcaFilter->SetInput(inputImage);
        }

        SetParameterOutputImage("out", m_pcaFilter->GetOutput());
    }

    PCAFilterType::Pointer                  m_pcaFilter;
    FillNoDataImageFilterType::Pointer      m_fillNoDataImageFilter;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::PrincipalComponentAnalysis)
