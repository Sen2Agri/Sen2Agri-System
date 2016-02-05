#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include <vector>
#include "MetadataHelperFactory.h"

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

        WriterType::Pointer writer;
        writer = WriterType::New();
        std::string fileName(outImg);
        if(bUseCompression) {
            fileName += std::string("?gdal:co:COMPRESS=DEFLATE");;
        }

        // Create an output parameter to write the current output image
        OutputImageParameter::Pointer paramOut = OutputImageParameter::New();
        // Set the filename of the current output image
        paramOut->SetFileName(outImg);
        paramOut->SetValue(imgMsk);
        paramOut->SetPixelType(ImagePixelType_int16);
        // Add the current level to be written
        paramOut->InitializeWriters();
        std::ostringstream osswriter;
        osswriter<< "Wrinting flags "<< outImg;
        AddProcess(paramOut->GetWriter(), osswriter.str());
        paramOut->Write();
    }
};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::GenerateLaiMonoDateMaskFlags)



