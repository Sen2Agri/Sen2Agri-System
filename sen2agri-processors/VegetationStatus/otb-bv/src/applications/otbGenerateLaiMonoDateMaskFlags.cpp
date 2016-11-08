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

#include <vector>
#include "MetadataHelperFactory.h"
#include "otbStreamingResampleImageFilter.h"
#include "ImageResampler.h"

namespace otb
{

namespace Wrapper
{

class GenerateLaiMonoDateMaskFlags : public Application
{
public:    

    typedef GenerateLaiMonoDateMaskFlags Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    itkNewMacro(Self)

    itkTypeMacro(GenerateLaiMonoDateMaskFlags, otb::Application)

    typedef otb::ImageFileWriter<MetadataHelper::SingleBandShortImageType> WriterType;
    typedef otb::StreamingResampleImageFilter<MetadataHelper::SingleBandShortImageType,
                    MetadataHelper::SingleBandShortImageType, double>     ResampleFilterType;

private:
    void DoInit()
    {
        SetName("GenerateLaiMonoDateMaskFlags");
        SetDescription("Extracts the flag masks for the given product.");

        SetDocName("GenerateLaiMonoDateMaskFlags");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");        
        AddDocTag(Tags::Vector);
        AddParameter(ParameterType_String,  "inxml",   "Input XML corresponding to the LAI mono date");
        AddParameter(ParameterType_String,  "out",   "The out mask flags image corresponding to the LAI mono date");
        AddParameter(ParameterType_Int, "compress", "Specifies if output files should be compressed or not.");
        MandatoryOff("compress");
        SetDefaultParameterInt("compress", 0);

        AddParameter(ParameterType_Int, "outres", "Output resolution. If not specified, is the same as the input resolution.");
        MandatoryOff("outres");
        AddParameter(ParameterType_String,  "outresampled",   "The out mask flags image corresponding to the LAI mono date resampled at the given resolution");
        MandatoryOff("outresampled");
    }

    void DoUpdateParameters()
    {
      // Nothing to do.
    }

    void DoExecute()
    {
        const std::string inXml = GetParameterAsString("inxml");
        std::string outImg = GetParameterAsString("out");
        bool bUseCompression = (GetParameterInt("compress") != 0);

        auto factory = MetadataHelperFactory::New();
        auto pHelper = factory->GetMetadataHelper(inXml);
        MetadataHelper::SingleBandShortImageType::Pointer imgMsk = pHelper->GetMasksImage(ALL, false);

        WriteOutput(imgMsk, outImg, -1, bUseCompression);

        if(HasValue("outres")) {
            if(HasValue("outresampled")) {
                std::string outImgRes = GetParameterAsString("outresampled");
                int nOutRes = GetParameterInt("outres");
                if(nOutRes != 10 && nOutRes != 20) {
                    itkExceptionMacro("Invalid output resolution specified (only 10 and 20 accepted)" << nOutRes);
                }
                WriteOutput(imgMsk, outImgRes, nOutRes, bUseCompression);
            } else {
                itkExceptionMacro("If you provide the outres parameter you must also provide an outresampled file name!");
            }
        }
    }

    void WriteOutput(MetadataHelper::SingleBandShortImageType::Pointer imgMsk, const std::string &outImg, int nRes, bool bUseCompression) {
        std::string fileName(outImg);
        if(bUseCompression) {
            fileName += std::string("?gdal:co:COMPRESS=DEFLATE");
        }

        // Create an output parameter to write the current output image
        OutputImageParameter::Pointer paramOut = OutputImageParameter::New();
        // Set the filename of the current output image
        paramOut->SetFileName(outImg);
        if(nRes == -1) {
            paramOut->SetValue(imgMsk);
        } else {
            // resample the image at the given resolution
            imgMsk->UpdateOutputInformation();
            int curRes = imgMsk->GetSpacing()[0];
            paramOut->SetValue(getResampledImage(curRes, nRes, imgMsk));
        }
        paramOut->SetPixelType(ImagePixelType_uint8);
        // Add the current level to be written
        paramOut->InitializeWriters();
        std::ostringstream osswriter;
        osswriter<< "Wrinting flags "<< outImg;
        AddProcess(paramOut->GetWriter(), osswriter.str());
        paramOut->Write();
    }

    MetadataHelper::SingleBandShortImageType::Pointer getResampledImage(int nCurRes, int nDesiredRes,
                                                 MetadataHelper::SingleBandShortImageType::Pointer inImg) {
        if(nCurRes == nDesiredRes)
            return inImg;
        float fMultiplicationFactor = ((float)nCurRes)/nDesiredRes;
        ResampleFilterType::Pointer resampler = m_Resampler.getResampler(inImg, fMultiplicationFactor, Interpolator_NNeighbor);
        return resampler->GetOutput();
    }

private:
    ImageResampler<MetadataHelper::SingleBandShortImageType, MetadataHelper::SingleBandShortImageType> m_Resampler;

};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::GenerateLaiMonoDateMaskFlags)



