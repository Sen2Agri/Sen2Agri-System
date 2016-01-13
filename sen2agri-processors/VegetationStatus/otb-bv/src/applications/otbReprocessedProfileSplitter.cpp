#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"

#include <vector>
#include "otbImageFileWriter.h"

namespace otb
{

namespace Wrapper
{

class ReprocessedProfileSplitter : public Application
{
public:    

    typedef ReprocessedProfileSplitter Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    itkNewMacro(Self)

    itkTypeMacro(ReprocessedProfileSplitter, otb::Application)

    typedef FloatVectorImageType                    InputImageType;
    typedef otb::Image<float, 2>                    InternalImageType;
    typedef otb::VectorImage<float, 2>              OutImageType;

    typedef otb::ImageList<InternalImageType>                                       InternalImgListType;
    typedef otb::VectorImageToImageListFilter<InputImageType, InternalImgListType>  VectorImageToImageListType;
    typedef otb::ImageListToVectorImageFilter<InternalImgListType, OutImageType>    ImageListToVectorImageFilterType;

    typedef otb::ImageFileReader<InputImageType> ReaderType;
    typedef otb::ImageFileWriter<OutImageType> WriterType;

private:
    void DoInit()
    {
        SetName("ReprocessedProfileSplitter");
        SetDescription("Extracts the BV estimation values products one band for each raster.");

        SetDocName("ReprocessedProfileSplitter");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");        
        AddDocTag(Tags::Vector);
        AddParameter(ParameterType_String,  "in",   "The BV estimation values product containing all time series");
        AddParameter(ParameterType_OutputFilename, "outlist", "File containing the list of all files produced.");
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
        std::string inImgStr = GetParameterString("in");
        m_reader = ReaderType::New();
        m_reader->SetFileName(inImgStr);
        m_reader->UpdateOutputInformation();
        int nTotalBands = m_reader->GetOutput()->GetNumberOfComponentsPerPixel();
        if((nTotalBands % 2) != 0)
        {
            itkExceptionMacro("Wrong number of bands. It should be even! " + nTotalBands);
        }

        bool bUseCompression = (GetParameterInt("compress") != 0);

        std::string strOutFilesList = GetParameterString("outlist");
        std::ofstream filesListFile;
        try {
            filesListFile.open(strOutFilesList.c_str(), std::ofstream::out);
        } catch(...) {
            itkGenericExceptionMacro(<< "Could not open file " << strOutFilesList);
        }

        std::string strOutPrefix = inImgStr;
        size_t pos = inImgStr.find_last_of(".");
        if (pos != std::string::npos) {
            strOutPrefix = inImgStr.substr(0, pos);
        }

        m_ImgSplit = VectorImageToImageListType::New();
        m_ImgSplit->SetInput(m_reader->GetOutput());
        m_ImgSplit->UpdateOutputInformation();

        int nImgsNo = nTotalBands / 2;
        for(int i = 0; i < nImgsNo; i++) {
            InternalImgListType::Pointer listImgs = InternalImgListType::New();
            listImgs->PushBack(m_ImgSplit->GetOutput()->GetNthElement(i));
            listImgs->PushBack(m_ImgSplit->GetOutput()->GetNthElement(nImgsNo + i));

            ImageListToVectorImageFilterType::Pointer concater = ImageListToVectorImageFilterType::New();
            concater->SetInput(listImgs);

            WriterType::Pointer writer;
            writer = WriterType::New();
            std::ostringstream fileNameStream;
            fileNameStream << strOutPrefix << "_b" << i << ".tif";
            // we might have also compression and we do not want that in the name file
            // to be saved into the produced files list file
            std::string simpleFileName = fileNameStream.str();
            if(bUseCompression) {
                fileNameStream << "?gdal:co:COMPRESS=DEFLATE";
            }
            std::string fileName = fileNameStream.str();
            writer->SetFileName(fileName);
            writer->SetInput(concater->GetOutput());
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
            filesListFile << simpleFileName << std::endl;
        }

        filesListFile.close();

        return;
    }

    ReaderType::Pointer m_reader;
    VectorImageToImageListType::Pointer       m_ImgSplit;
};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::ReprocessedProfileSplitter)



