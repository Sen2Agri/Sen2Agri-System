#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbMultiToMonoChannelExtractROI.h"

#include <vector>
#include "MetadataHelperFactory.h"

namespace otb
{

namespace Wrapper
{

class ReprocessedProfileSplitter2 : public Application
{
public:    

    typedef ReprocessedProfileSplitter2 Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    itkNewMacro(Self)

    itkTypeMacro(ReprocessedProfileSplitter2, otb::Application)

    typedef FloatVectorImageType                    InputImageType;
    typedef otb::Image<float, 2>                    InternalImageType;

    /** Filters typedef */
    typedef otb::MultiToMonoChannelExtractROI<InputImageType::InternalPixelType,
                                              InternalImageType::InternalPixelType> FilterType;

    typedef otb::ImageFileReader<InputImageType> ReaderType;

private:
    void DoInit()
    {
        SetName("ReprocessedProfileSplitter2");
        SetDescription("Extracts the BV estimation values products one band for each raster.");

        SetDocName("ReprocessedProfileSplitter2");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");        
        AddDocTag(Tags::Vector);
        AddParameter(ParameterType_String,  "in",   "The BV estimation values product containing all time series");
        AddParameter(ParameterType_InputFilenameList, "ilxml", "The XML metadata files list");
        MandatoryOff("ilxml");
        AddParameter(ParameterType_OutputFilename, "outrlist", "File containing the list of all raster files produced.");
        AddParameter(ParameterType_OutputFilename, "outflist", "File containing the list of all flag files produced.");
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

        std::string strOutRasterFilesList = GetParameterString("outrlist");
        std::ofstream rasterFilesListFile;
        std::string strOutFlagsFilesList = GetParameterString("outflist");
        std::ofstream flagsFilesListFile;
        try {
            rasterFilesListFile.open(strOutRasterFilesList.c_str(), std::ofstream::out);
            flagsFilesListFile.open(strOutFlagsFilesList.c_str(), std::ofstream::out);
        } catch(...) {
            itkGenericExceptionMacro(<< "Could not open file " << strOutRasterFilesList);
        }

        std::vector<std::string> xmlsList;
        if(HasValue("ilxml")) {
            xmlsList = this->GetParameterStringList("ilxml");
            if(xmlsList.size() != (size_t)(nTotalBands / 2)) {
                itkExceptionMacro("Wrong number of input xml files. It should be " + (nTotalBands/2));
            }
        }

        std::string strOutPrefix = inImgStr;
        size_t pos = inImgStr.find_last_of(".");
        if (pos != std::string::npos) {
            strOutPrefix = inImgStr.substr(0, pos);
        }

        // Set the extract filter input image
        m_Filter = FilterType::New();
        m_Filter->SetInput(m_reader->GetOutput());


        int nTotalBandsHalf = nTotalBands/2;
        for(int i = 0; i < nTotalBands; i++) {
            std::ostringstream fileNameStream;

            std::string curXml = xmlsList[(i < nTotalBandsHalf) ? i : (i-nTotalBandsHalf)];
            auto factory = MetadataHelperFactory::New();
            auto pHelper = factory->GetMetadataHelper(curXml);
            std::string acquisitionDate = pHelper->GetAcquisitionDate();

            // writer label
            std::ostringstream osswriter;

            if(i < nTotalBandsHalf) {
                fileNameStream << strOutPrefix << "_" << acquisitionDate << "_img.tif";
                osswriter<< "writer (Image for date "<< i << " : " << acquisitionDate << ")";
            } else {
                fileNameStream << strOutPrefix << "_" << acquisitionDate << "_flags.tif";
                osswriter<< "writer (Flags for date "<< i << " : " << acquisitionDate << ")";
            }
            // we might have also compression and we do not want that in the name file
            // to be saved into the produced files list file
            std::string simpleFileName = fileNameStream.str();
            if(bUseCompression) {
                fileNameStream << "?gdal:co:COMPRESS=DEFLATE";
            }
            std::string fileName = fileNameStream.str();

            // Create an output parameter to write the current output image
            OutputImageParameter::Pointer paramOut = OutputImageParameter::New();
            // Set the channel to extract
            m_Filter->SetChannel(i+1);

            // Set the filename of the current output image
            paramOut->SetFileName(fileName);
            paramOut->SetValue(m_Filter->GetOutput());
            paramOut->SetPixelType(ImagePixelType_float);
            // Add the current level to be written
            paramOut->InitializeWriters();
            AddProcess(paramOut->GetWriter(), osswriter.str());
            paramOut->Write();

            if(i < nTotalBandsHalf) {
                rasterFilesListFile << simpleFileName << std::endl;
            } else {
                flagsFilesListFile << simpleFileName << std::endl;
            }
        }

        rasterFilesListFile.close();
        flagsFilesListFile.close();

        return;
    }

    ReaderType::Pointer m_reader;
    FilterType::Pointer        m_Filter;
};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::ReprocessedProfileSplitter2)



