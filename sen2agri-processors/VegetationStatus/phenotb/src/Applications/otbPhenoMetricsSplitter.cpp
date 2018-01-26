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
#include "otbMultiChannelExtractROI.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "GlobalDefs.h"

#include <vector>

namespace otb
{

namespace Wrapper
{

class PhenoMetricsSplitter : public Application
{
public:    

    typedef PhenoMetricsSplitter Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    itkNewMacro(Self)

    itkTypeMacro(PhenoMetricsSplitter, otb::Application)

    typedef UInt16VectorImageType                   InputImageType;
    typedef otb::Image<short, 2>                    InternalImageType;
    typedef otb::Image<short, 2>                    OutFlagsImageType;

    /** Filters typedef */
    typedef otb::MultiChannelExtractROI<InputImageType::InternalPixelType,
                                        FloatVectorImageType::InternalPixelType> FilterType1;

    /** Filters typedef */
    typedef otb::MultiToMonoChannelExtractROI<InputImageType::InternalPixelType,
                                              InternalImageType::InternalPixelType> FilterType2;

    typedef otb::ImageFileReader<InputImageType> ReaderType;

    typedef itk::UnaryFunctorImageFilter<InternalImageType,OutFlagsImageType,
                    FloatToShortTranslationFunctor<
                        InternalImageType::PixelType,
                        OutFlagsImageType::PixelType> > FlagsFloatToShortTransFilterType;

private:
    void DoInit()
    {
        SetName("PhenoMetricsSplitter");
        SetDescription("Extracts the phenological parameters in a raster and the flags into another raster.");

        SetDocName("PhenoMetricsSplitter");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");        
        AddDocTag(Tags::Vector);
        AddParameter(ParameterType_String,  "in",   "The phenologic parameters and the flags raster");
        AddParameter(ParameterType_OutputImage, "outparams", "The output phenologic parameters raster.");
        AddParameter(ParameterType_OutputImage, "outflags", "The output flags raster.");
        AddParameter(ParameterType_Int, "compress", "If set to a value different of 0, the output is compressed");
        MandatoryOff("compress");
        SetDefaultParameterInt("compress", 0);

        AddParameter(ParameterType_Int, "cog", "If set to a value different of 0, the output is created in cloud optimized geotiff and compressed.");
        MandatoryOff("cog");
        SetDefaultParameterInt("cog", 0);
    }

    void DoUpdateParameters()
    {
      // Nothing to do.
    }

    void DoExecute()
    {
        std::string inImgStr = GetParameterString("in");
        m_reader = ReaderType::New();
        m_reader->SetFileName(inImgStr);
        m_reader->UpdateOutputInformation();
        int nTotalBands = m_reader->GetOutput()->GetNumberOfComponentsPerPixel();
        if(nTotalBands != 5)
        {
            itkExceptionMacro("Wrong number of bands " << nTotalBands << ". It should be 5!");
        }

        // Set the extract filter input image
        m_FilterParams = FilterType1::New();
        m_FilterParams->SetInput(m_reader->GetOutput());
        // Set the channel to extract
        m_FilterParams->SetFirstChannel(1);
        m_FilterParams->SetLastChannel(4);

        m_FilterFlags = FilterType2::New();
        m_FilterFlags->SetInput(m_reader->GetOutput());
        m_FilterFlags->SetChannel(5);

        m_FilterParams->UpdateOutputInformation();
        m_FilterFlags->UpdateOutputInformation();

        m_flagsFloatToShortFunctor = FlagsFloatToShortTransFilterType::New();
        m_flagsFloatToShortFunctor->SetInput(m_FilterFlags->GetOutput());
        m_flagsFloatToShortFunctor->GetFunctor().Initialize(1, 0);

        // modify the name if we have compression
        SetParameterString("outparams", GetFileName("outparams"));
        SetParameterString("outflags", GetFileName("outflags"));
        // write the files
        // the pheno ndvi metrics are already int values
        SetParameterOutputImage("outparams", m_FilterParams->GetOutput());
        SetParameterOutputImagePixelType("outparams", ImagePixelType_int16);
        SetParameterOutputImage("outflags", m_flagsFloatToShortFunctor->GetOutput());
        SetParameterOutputImagePixelType("outflags", ImagePixelType_uint8);

        return;
    }

    std::string GetFileName(const std::string &outParamName) {
        bool bUseCompression = (GetParameterInt("compress") != 0);
        bool bClodOptimizedGeotiff = (GetParameterInt("cog") != 0);
        std::string ofname = GetParameterString(outParamName);
        std::ostringstream fileNameStream;
        fileNameStream << ofname;
        if (bClodOptimizedGeotiff) {
            fileNameStream << "?gdal:co:TILED=YES&gdal:co:COPY_SRC_OVERVIEWS=YES&gdal:co:COMPRESS=DEFLATE&gdal:co:BIGTIFF=YES";
        } else if (bUseCompression) {
            fileNameStream << "?gdal:co:COMPRESS=DEFLATE&gdal:co:BIGTIFF=YES";
        }
        return fileNameStream.str();
    }

    ReaderType::Pointer m_reader;
    FilterType1::Pointer        m_FilterParams;
    FilterType2::Pointer        m_FilterFlags;

    FlagsFloatToShortTransFilterType::Pointer m_flagsFloatToShortFunctor;
};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::PhenoMetricsSplitter)



