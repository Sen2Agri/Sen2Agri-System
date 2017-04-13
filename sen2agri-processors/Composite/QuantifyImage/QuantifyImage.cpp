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
#include "otbImage.h"
#include "otbVectorImage.h"

#include "itkUnaryFunctorImageFilter.h"

#include "GlobalDefs.h"

namespace otb
{

namespace Wrapper
{

class QuantifyImage : public Application
{
public:
    typedef QuantifyImage Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(QuantifyImage, otb::Application)


    typedef otb::VectorImage<float, 2>                                          InputImageType;
    typedef otb::VectorImage<short, 2>                                          OutputImageType;

    typedef itk::UnaryFunctorImageFilter<InputImageType,OutputImageType,
                    FloatToShortTranslationFunctor<
                        InputImageType::PixelType,
                        OutputImageType::PixelType> > FloatToShortTransFilterType;

private:

    void DoInit()
    {
        SetName("QuantifyImage");
        SetDescription("Converts an image from float to short image using the given quantification value");

        SetDocName("QuantifyImage");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");
        AddDocTag(Tags::Vector);
        AddParameter(ParameterType_InputImage, "in", "Input image");
        AddParameter(ParameterType_OutputImage, "out", "Output image");

        AddParameter(ParameterType_Int, "qval", "The quantification value to be used");
        SetDefaultParameterInt("qval", DEFAULT_QUANTIFICATION_VALUE);
        MandatoryOff("qval");

        AddParameter(ParameterType_Int, "nodata", "The no data value to be used");
        SetDefaultParameterInt("nodata", NO_DATA_VALUE);
        MandatoryOff("nodata");
    }

    void DoUpdateParameters()
    {
      // Nothing to do.
    }

    void DoExecute()
    {
        m_inputImg = GetParameterFloatVectorImage("in");
        m_inputImg->UpdateOutputInformation();
        int quantificationValue = GetParameterInt("qval");
        int noDataValue = GetParameterInt("nodata");

        m_floatToShortFunctor = FloatToShortTransFilterType::New();
        m_floatToShortFunctor->GetFunctor().Initialize(quantificationValue, noDataValue);
        m_floatToShortFunctor->SetInput(m_inputImg);
        m_floatToShortFunctor->GetOutput()->SetNumberOfComponentsPerPixel(m_inputImg->GetNumberOfComponentsPerPixel());

        SetParameterOutputImage("out", m_floatToShortFunctor->GetOutput());
        SetParameterOutputImagePixelType("out", ImagePixelType_int16);
    }

    InputImageType::Pointer             m_inputImg;
    FloatToShortTransFilterType::Pointer  m_floatToShortFunctor;
};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::QuantifyImage)



