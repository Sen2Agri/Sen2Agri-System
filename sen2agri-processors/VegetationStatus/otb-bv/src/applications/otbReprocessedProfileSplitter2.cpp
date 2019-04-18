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
#include "otbMultiToMonoChannelExtractROI.h"

#include "otbImageListToVectorImageFilter.h"

#include <vector>
#include "MetadataHelperFactory.h"
#include "GlobalDefs.h"

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

    typedef short                                                             ShortPixelType;
    typedef otb::Image<ShortPixelType, 2>                                     ShortImageType;

    typedef FloatVectorImageType                    InputImageType;
    typedef otb::Image<float, 2>                    InternalImageType;

    /** Filters typedef */
    typedef otb::MultiToMonoChannelExtractROI<InputImageType::InternalPixelType,
                                              InternalImageType::InternalPixelType> FilterType;

    typedef otb::ImageFileReader<InputImageType> ReaderType;

    typedef itk::UnaryFunctorImageFilter<InternalImageType,ShortImageType,
                    FloatToShortTranslationFunctor<
                        InternalImageType::PixelType,
                        ShortImageType::PixelType> > FloatToShortTransFilterType;

    typedef otb::ImageList<ShortImageType>                                       ImgListType;
    typedef otb::VectorImage<ShortPixelType, 2>                                          ImageType;
    typedef otb::ImageListToVectorImageFilter<ImgListType, ImageType>           ImageListToVectorImageFilterType;

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

        AddParameter(ParameterType_StringList, "ildates", "The dates for the products");
        MandatoryOff("ildates");

        AddParameter(ParameterType_OutputFilename, "outrlist", "File containing the list of all raster files produced.");
        AddParameter(ParameterType_OutputFilename, "outflist", "File containing the list of all flag files produced.");

        AddParameter(ParameterType_Int, "grprasters", "Specifies if rasters and flags should be grouped in distinct files.");
        MandatoryOff("grprasters");
        SetDefaultParameterInt("grprasters", 1);

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
        if((nTotalBands % 2) != 0)
        {
            itkExceptionMacro("Wrong number of bands. It should be even! " + nTotalBands);
        }

        bool bUseCompression = (GetParameterInt("compress") != 0);
        bool bClodOptimizedGeotiff = (GetParameterInt("cog") != 0);
        bool bGroupRasters = (GetParameterInt("grprasters") != 0);

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
        std::vector<std::string> datesList;
        int dates_no = 0;
        if(HasValue("ilxml")) {
            xmlsList = this->GetParameterStringList("ilxml");
            dates_no = xmlsList.size();
            for (const std::string &strXml : xmlsList)
            {
                MetadataHelperFactory::Pointer factory = MetadataHelperFactory::New();
                // we are interested only in the 10m resolution as we need only the date
                auto pHelper = factory->GetMetadataHelper<short>(strXml);
                datesList.push_back(pHelper->GetAcquisitionDate());
            }
        } else if(HasValue("ildates")) {
            datesList = this->GetParameterStringList("ildates");
            dates_no = datesList.size();
        }
        if (dates_no == 0) {
            itkExceptionMacro("Either ilxml or ildates should be provided");
        }
        if((nTotalBands != 2) && (dates_no != (size_t)(nTotalBands / 2))) {
            itkExceptionMacro("Wrong number of input xml files or dates. It should be " + (nTotalBands/2));
        }

        std::string strOutPrefix = inImgStr;
        size_t pos = inImgStr.find_last_of(".");
        if (pos != std::string::npos) {
            strOutPrefix = inImgStr.substr(0, pos);
        }

        int nTotalBandsHalf = nTotalBands/2;
        bool bIsRaster;
        std::string groupRastersFileName;
        std::string groupFlagsFileName;
        for(int i = 0; i < nTotalBands; i++) {
            std::ostringstream fileNameStream;

            std::string curAcquisitionDate = datesList[(i < nTotalBandsHalf) ? i : (i-nTotalBandsHalf)];
            // if we did not generated all dates, we have only 2 bands and we consider
            // the last XML in the list
            if(nTotalBands == 2) {
                curAcquisitionDate = datesList[dates_no-1];
            }

            // writer label
            std::ostringstream osswriter;
            bIsRaster = (i < nTotalBandsHalf);
            if(bIsRaster) {
                fileNameStream << strOutPrefix << "_" << curAcquisitionDate << (bGroupRasters ? "_grp" : "") << "_img.tif";
                osswriter<< "writer (Image for date "<< i << " : " << curAcquisitionDate << ")";
            } else {
                fileNameStream << strOutPrefix << "_" << curAcquisitionDate << (bGroupRasters ? "_grp" : "") << "_flags.tif";
                osswriter<< "writer (Flags for date "<< i << " : " << curAcquisitionDate << ")";
            }
            // we might have also compression and we do not want that in the name file
            // to be saved into the produced files list file
            std::string simpleFileName = fileNameStream.str();
            if (bClodOptimizedGeotiff) {
                fileNameStream << "?gdal:co:TILED=YES&gdal:co:COPY_SRC_OVERVIEWS=YES&gdal:co:COMPRESS=DEFLATE&gdal:co:BIGTIFF=YES";
            } else if (bUseCompression) {
                fileNameStream << "?gdal:co:COMPRESS=DEFLATE&gdal:co:BIGTIFF=YES";
            }

            const std::string &fileName = fileNameStream.str();

            // Set the extract filter input image
            // normally, if grouping is not needed, we could use a single bands extractor.
            // But for grouping, we need a band extractor for each band otherwise the last band will be written
            // in all extracted bands
            FilterType::Pointer bandsExtractor = FilterType::New();
            bandsExtractor->SetInput(m_reader->GetOutput());
            // Set the channel to extract
            bandsExtractor->SetChannel(i+1);
            m_bandsExtractors.push_back(bandsExtractor);

            FloatToShortTransFilterType::Pointer floatToShortFunctor = FloatToShortTransFilterType::New();
            floatToShortFunctor->SetInput(bandsExtractor->GetOutput());
            floatToShortFunctor->GetFunctor().Initialize(1, NO_DATA_VALUE, true);
            m_floatToShortFunctors.push_back(floatToShortFunctor);

            if(bGroupRasters) {
                if(bIsRaster) {
                    if(groupRastersFileName.length() == 0) {
                        groupRastersFileName = fileName;
                        rasterFilesListFile << simpleFileName << std::endl;
                        m_RastersList = ImgListType::New();
                    }
                    m_RastersList->PushBack(floatToShortFunctor->GetOutput());

                } else {
                    if(groupFlagsFileName.length() == 0) {
                        groupFlagsFileName = fileName;
                        flagsFilesListFile << simpleFileName << std::endl;
                        m_FlagsList = ImgListType::New();
                    }
                    m_FlagsList->PushBack(floatToShortFunctor->GetOutput());
                }

            } else {
                // Create an output parameter to write the current output image
                OutputImageParameter::Pointer paramOut = OutputImageParameter::New();
                if(bIsRaster) {
                    // we have here the already quantified values that need no other quantification
                    paramOut->SetPixelType(ImagePixelType_int16);
                } else {
                    // we need no quantification value, just convert to byte
                    paramOut->SetPixelType(ImagePixelType_uint8);
                }
                paramOut->SetValue(floatToShortFunctor->GetOutput());
                // Set the filename of the current output image
                paramOut->SetFileName(fileName);
                // Add the current level to be written
                paramOut->InitializeWriters();
                AddProcess(paramOut->GetWriter(), osswriter.str());
                paramOut->Write();

                if(bIsRaster) {
                    rasterFilesListFile << simpleFileName << std::endl;
                } else {
                    flagsFilesListFile << simpleFileName << std::endl;
                }
            }
        }
        if(bGroupRasters) {
            writeGroupedImg(groupRastersFileName, "writer (rasters grouped)", true);
            writeGroupedImg(groupFlagsFileName, "writer (flags grouped)", false);
        }

        rasterFilesListFile.close();
        flagsFilesListFile.close();

        return;
    }

    void writeGroupedImg(const std:: string &fileName, const std:: string &descr, bool bIsRaster) {
        // Create an output parameter to write the current output image
        OutputImageParameter::Pointer paramOut = OutputImageParameter::New();
        ImageType::Pointer img;
        if(bIsRaster) {
            m_RastersConcat = ImageListToVectorImageFilterType::New();
            m_RastersConcat->SetInput(m_RastersList);
            img = m_RastersConcat->GetOutput();

            // we have here the already quantified values that need no other quantification
            paramOut->SetPixelType(ImagePixelType_int16);
        } else {
            m_FlagsConcat = ImageListToVectorImageFilterType::New();
            m_FlagsConcat->SetInput(m_FlagsList);
            img = m_FlagsConcat->GetOutput();

            // we need no quantification value, just convert to byte
            paramOut->SetPixelType(ImagePixelType_uint8);
        }
        paramOut->SetValue(img);
        // Set the filename of the current output image
        paramOut->SetFileName(fileName);
        // Add the current level to be written
        paramOut->InitializeWriters();
        AddProcess(paramOut->GetWriter(), descr);
        paramOut->Write();
    }

    ReaderType::Pointer m_reader;
    //FilterType::Pointer        m_Filter;
    std::vector<FilterType::Pointer> m_bandsExtractors;
    std::vector<FloatToShortTransFilterType::Pointer>  m_floatToShortFunctors;

    ImageListToVectorImageFilterType::Pointer m_RastersConcat;
    ImageListToVectorImageFilterType::Pointer m_FlagsConcat;
    ImgListType::Pointer m_RastersList;
    ImgListType::Pointer m_FlagsList;
};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::ReprocessedProfileSplitter2)



