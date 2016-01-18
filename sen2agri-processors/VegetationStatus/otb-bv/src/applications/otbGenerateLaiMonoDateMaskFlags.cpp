#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"

#include <vector>
#include "otbImageFileWriter.h"
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
        SetDescription("Extracts the BV estimation values products one band for each raster.");

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
        if(bUseCompression) {
            outImg += std::string("?gdal:co:COMPRESS=DEFLATE");;
        }
        writer->SetFileName(outImg);
        writer->SetInput(imgMsk);
        try
        {
            writer->Update();
        }
        catch (itk::ExceptionObject& err)
        {
            std::cout << "ExceptionObject caught !" << std::endl;
            std::cout << err << std::endl;
            itkExceptionMacro("Error writing output");
        }

        return;
    }
};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::GenerateLaiMonoDateMaskFlags)



