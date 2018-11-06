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

#include "otbOGRDataToClassStatisticsFilter.h"
#include "otbStatisticsXMLFileWriter.h"
#include "otbGeometriesProjectionFilter.h"
#include "otbGeometriesSet.h"
#include "otbWrapperElevationParametersHandler.h"
#include "otbConcatenateVectorImagesFilter.h"
#include "otbBandMathImageFilter.h"
#include "otbVectorImageToImageListFilter.h"
#include <boost/filesystem.hpp>
#include "otbAgricPractDataExtrFileWriter2.h"

namespace otb
{
namespace Wrapper
{

/** Utility function to negate std::isalnum */
bool IsNotAlphaNum(char c)
{
    return !std::isalnum(c);
}

class AgricPractDataExtraction : public Application
{
public:
    /** Standard class typedefs. */
    typedef AgricPractDataExtraction        Self;
    typedef Application                   Superclass;
    typedef itk::SmartPointer<Self>       Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Standard macro */
    itkNewMacro(Self);

    itkTypeMacro(AgricPractDataExtraction, otb::Application);

    /** Filters typedef */
    typedef float                                           PixelType;
    typedef otb::Image<PixelType, 2>                        SimpleImageType;

    typedef FloatVectorImageType                            ImageType;
    typedef UInt8ImageType                                  MaskImageType;

    typedef otb::OGRDataToClassStatisticsFilter<ImageType,UInt8ImageType> FilterType;

    typedef otb::AgricPractDataExtrFileWriter2<ImageType::PixelType> AgricPracticesWriterType2;

    typedef otb::GeometriesSet GeometriesType;

    typedef otb::GeometriesProjectionFilter ProjectionFilterType;

    typedef otb::ConcatenateVectorImagesFilter<ImageType>               ConcatenateImagesFilterType;
    typedef otb::ImageFileReader<ImageType>                             ImageReaderType;
    typedef otb::ObjectList<ImageReaderType>                            ReadersListType;

    typedef std::map<std::string , std::string>           GenericMapType;
    typedef std::map<std::string , GenericMapType>        GenericMapContainer;
    typedef struct {
        std::string inputImage;
        std::vector<std::string> s2MsksFiles;
    } S2MaskedInputFileInfoType;

private:
    AgricPractDataExtraction()
    {
        m_concatenateImagesFilter = ConcatenateImagesFilterType::New();
        //m_Readers = ReadersListType::New();
    }

    void DoInit() override
    {
        SetName("AgricPractDataExtraction");
        SetDescription("Computes statistics on a training polygon set.");

        // Documentation
        SetDocName("Polygon Class Statistics");
        SetDocLongDescription("The application processes a set of geometries "
        "intended for training (they should have a field giving the associated "
        "class). The geometries are analyzed against a support image to compute "
        "statistics : \n"
        "  - number of samples per class\n"
        "  - number of samples per geometry\n"
        "An optional raster mask can be used to discard samples. Different types"
        " of geometry are supported : polygons, lines, points. The behaviour is "
        "different for each type of geometry :\n"
        "  - polygon: select pixels whose center is inside the polygon\n"
        "  - lines  : select pixels intersecting the line\n"
        "  - points : select closest pixel to the point");
        SetDocLimitations("None");
        SetDocAuthors("OTB-Team");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Learning);

        AddParameter(ParameterType_StringList, "il", "Input Images");
        SetParameterDescription("il", "Support images that will be classified");

        AddParameter(ParameterType_InputFilename, "vec", "Input vectors");
        SetParameterDescription("vec","Input geometries to analyze");

        AddParameter(ParameterType_String, "outdir", "Output directory for writing agricultural practices data extractin files");
        SetParameterDescription("outdir","Output directory to store agricultural practices data extractin files (txt format)");

        AddParameter(ParameterType_ListView, "field", "Field Name");
        SetParameterDescription("field","Name of the field carrying the class name in the input vectors.");
        //SetListViewSingleSelectionMode("field",true);

        AddParameter(ParameterType_Int, "layer", "Layer Index");
        SetParameterDescription("layer", "Layer index to read in the input vector file.");
        MandatoryOff("layer");
        SetDefaultParameterInt("layer",0);

        AddParameter(ParameterType_StringList, "s2il", "S2 tiles parcels masks images");
        SetParameterDescription("s2il", "S2 tiles parcels masks images");
        MandatoryOff("s2il");

        AddParameter(ParameterType_StringList, "s2ilcnts", "Number of S2 tiles parcels masks images for each input image");
        SetParameterDescription("s2ilcnts", "Number of S2 tiles parcels masks images for each input image");
        MandatoryOff("s2ilcnts");

        AddParameter(ParameterType_Int, "convdb", "Convert pixel values to db before processing");
        SetParameterDescription("convdb", "Convert pixel values to db before processing.");
        SetDefaultParameterInt("convdb",0);
        MandatoryOff("convdb");

        AddParameter(ParameterType_Int, "oldnf", "Use old naming format");
        SetParameterDescription("oldnf", "Use the 2017 naming format for COHE and AMP format instead of the 2018 format.");
        MandatoryOff("oldnf");
        SetDefaultParameterInt("oldnf",0);

        AddParameter(ParameterType_Int, "outcsv", "Output the files into CSV format.");
        SetParameterDescription("outcsv", "Output the files into CSV format. If set to 0, outputs the files as XML");
        MandatoryOff("outcsv");
        SetDefaultParameterInt("outcsv",1);

        AddParameter(ParameterType_Int, "csvcompact", "Wite the CSV file compacted.");
        SetParameterDescription("csvcompact", "The entries for a field are written on the same line "
                                              "separated by |, without duplicating fid and suffix.");
        MandatoryOff("csvcompact");
        SetDefaultParameterInt("csvcompact",1);

        AddParameter(ParameterType_Int, "mfiles", "Output individual CVS files for each polygon");
        SetParameterDescription("mfiles", "If CSV format is used, then setting to true will produce individual files for each polygon.");
        MandatoryOff("mfiles");
        SetDefaultParameterInt("mfiles",0);

        AddParameter(ParameterType_InputImage,  "mask",   "Input validity mask");
        SetParameterDescription("mask", "Validity mask (only pixels corresponding to a mask value greater than 0 will be used for statistics)");
        MandatoryOff("mask");

        //ElevationParametersHandler::AddElevationParameters(this, "elev");

        AddRAMParameter();

        // Doc example parameter settings
        SetDocExampleParameterValue("in", "support_image.tif");
        SetDocExampleParameterValue("vec", "variousVectors.sqlite");
        SetDocExampleParameterValue("field", "label");
        SetDocExampleParameterValue("outdir","/path/to/output/");

        //SetOfficialDocLink();
    }

    void DoUpdateParameters() override
    {
        if ( HasValue("vec") )
        {
            std::string vectorFile = GetParameterString("vec");
            ogr::DataSource::Pointer ogrDS =
            ogr::DataSource::New(vectorFile, ogr::DataSource::Modes::Read);
            ogr::Layer layer = ogrDS->GetLayer(this->GetParameterInt("layer"));
            ogr::Feature feature = layer.ogr().GetNextFeature();

            ClearChoices("field");

            for(int iField=0; iField<feature.ogr().GetFieldCount(); iField++)
            {
                std::string key, item = feature.ogr().GetFieldDefnRef(iField)->GetNameRef();
                key = item;
                std::string::iterator end = std::remove_if(key.begin(),key.end(),IsNotAlphaNum);
                std::transform(key.begin(), end, key.begin(), tolower);

                OGRFieldType fieldType = feature.ogr().GetFieldDefnRef(iField)->GetType();

                if(fieldType == OFTString || fieldType == OFTInteger/* || ogr::version_proxy::IsOFTInteger64(fieldType)*/)
                {
                    std::string tmpKey="field."+key.substr(0, end - key.begin());
                    AddChoice(tmpKey,item);
                }
            }
        }
    }

    void DoExecute() override
    {
        // Retrieve the field name
        const std::vector<int> &selectedCFieldIdx = GetSelectedItems("field");
        if(selectedCFieldIdx.empty())
        {
            otbAppLogFATAL(<<"No field has been selected for data labelling!");
        }

        const std::vector<std::string> &imagesPaths = this->GetParameterStringList("il");
        if(imagesPaths.size() == 0) {
            otbAppLogFATAL(<<"No image was given as input!");
        }

        // Reproject geometries
        const std::string &vectFile = this->GetParameterString("vec");
        otbAppLogINFO("Loading vectors from file " << vectFile);
        otb::ogr::DataSource::Pointer vectors = otb::ogr::DataSource::New(vectFile);

        const std::vector<std::string> &cFieldNames = GetChoiceNames("field");
        const std::string &fieldName = cFieldNames[selectedCFieldIdx.front()];

        // Initialize the writer
        InitializeWriter();

        if (HasValue("s2il")) {
            const std::vector<std::string> &s2MsksPaths = this->GetParameterStringList("s2il");
            if (HasValue("s2ilcnts")) {
                const std::vector<std::string> &s2MsksCnts = this->GetParameterStringList("s2ilcnts");
                if (imagesPaths.size() != s2MsksCnts.size()) {
                    otbAppLogFATAL(<<"Invalid number of S2 masks given for the number of input images list! Expected: " <<
                                   imagesPaths.size() << " but got " << s2MsksCnts.size());
                }
                // first perform a validation of the counts
                int expectedS2Imgs = 0;
                for (int i = 0; i<s2MsksCnts.size(); i++) {
                    expectedS2Imgs += std::atoi(s2MsksCnts[i].c_str());
                }
                if (expectedS2Imgs != s2MsksPaths.size()) {
                    otbAppLogFATAL(<<"The S2 masks number and the provided S2 files do not match! Expected " <<
                                   expectedS2Imgs << " but got " << s2MsksPaths.size());
                }
                int curS2IlIdx = 0;
                for (int i = 0; i<imagesPaths.size(); i++) {
                    S2MaskedInputFileInfoType info;
                    info.inputImage = imagesPaths[i];
                    int curImgCnt = std::atoi(s2MsksCnts[i].c_str());
                    for(int j = 0; j<curImgCnt; j++) {
                        info.s2MsksFiles.emplace_back(s2MsksPaths[curS2IlIdx+j]);
                    }
                    m_s2MaskedInputFiles.emplace_back(info);
                    curS2IlIdx += curImgCnt;
                }
            } else {
                otbAppLogWARNING("WARNING: Using all s2 mask files for each input image!!!");
                for (const auto &inputImg: imagesPaths) {
                    S2MaskedInputFileInfoType info;
                    info.inputImage = inputImg;
                    info.s2MsksFiles = s2MsksPaths;
                    m_s2MaskedInputFiles.emplace_back(info);
                }
            }
            // TODO: call here the corresponding function
        } else {
            //otb::Wrapper::ElevationParametersHandler::SetupDEMHandlerFromElevationParameters(this,"elev");

            int i = 1;
            otb::ogr::DataSource::Pointer reprojVector = vectors;
            for (std::vector<std::string>::const_iterator itImages = imagesPaths.begin();
                 itImages != imagesPaths.end(); ++itImages)
            {
                if ( !boost::filesystem::exists( *itImages) ) {
                    otbAppLogWARNING("File " << *itImages << " does not exist on disk!");
                    continue;
                }
                FloatVectorImageType::Pointer inputImage = getInputImage(*itImages);
                otbAppLogINFO("Handling file " << *itImages);

                if (NeedsReprojection(reprojVector, inputImage)) {
                    otbAppLogINFO("Reprojecting vectors needed for file " << *itImages);
                    reprojVector = GetVector(vectors, inputImage);
                } else {
                    otbAppLogINFO("No need to reproject vectors for " << *itImages);
                }
                FilterType::Pointer filter = getStatisticsFilter(inputImage, reprojVector, fieldName);
                const FilterType::PixeMeanStdDevlValueMapType &meanStdValues = filter->GetMeanStdDevValueMap();
                m_agricPracticesDataWriter->AddInputMap<FilterType::PixeMeanStdDevlValueMapType>(*itImages, meanStdValues);

                otbAppLogINFO("Extracted a number of " << meanStdValues.size() << " values for file " << *itImages);
                otbAppLogINFO("Processed " << i << " products. Remaining products = " << imagesPaths.size() - i <<
                              ". Percent completed = " << (int)((((double)i) / imagesPaths.size()) * 100) << "%");

                filter->GetFilter()->Reset();
                i++;
            }
        }
        const std::string &outDir = this->GetParameterString("outdir");
        otbAppLogINFO("Writing outputs to folder " << outDir);
        m_agricPracticesDataWriter->Update();
        otbAppLogINFO("Writing outputs to folder done!");
    }

    FilterType::Pointer getStatisticsFilter(const FloatVectorImageType::Pointer &inputImg,
                                            const otb::ogr::DataSource::Pointer &reprojVector,
                                            const std::string &fieldName)
    {
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput(inputImg);

        if (IsParameterEnabled("mask") && HasValue("mask"))
        {
            filter->SetMask(this->GetParameterImage<UInt8ImageType>("mask"));
        }
        if (IsParameterEnabled("convdb") && HasValue("convdb"))
        {
            filter->SetConvertValuesToDecibels(GetParameterInt("convdb") != 0);
        }

        filter->SetOGRData(reprojVector);
        filter->SetFieldName(fieldName);
        filter->SetLayerIndex(this->GetParameterInt("layer"));
        filter->GetStreamer()->SetAutomaticAdaptativeStreaming(GetParameterInt("ram"));

        AddProcess(filter->GetStreamer(),"Analyze polygons...");
        filter->Update();

        return filter;
    }

    FloatVectorImageType::Pointer getInputImage(const std::string &imgPath) {
        ImageReaderType::Pointer imageReader = ImageReaderType::New();
        //m_Readers->PushBack(imageReader);
        m_ImageReader = imageReader;
        imageReader->SetFileName(imgPath);
        imageReader->UpdateOutputInformation();
        FloatVectorImageType::Pointer retImg = imageReader->GetOutput();
        retImg->UpdateOutputInformation();
        return retImg;
    }

    void InitializeWriter() {
        const std::vector<std::string> &imagesPaths = this->GetParameterStringList("il");
        const std::string &outDir = this->GetParameterString("outdir");
        m_agricPracticesDataWriter = AgricPracticesWriterType2::New();
        m_agricPracticesDataWriter->SetTargetFileName(BuildUniqueFileName(outDir, imagesPaths[0]));
        std::vector<std::string> header = {"KOD_PB", "date", "mean", "stdev"};
        m_agricPracticesDataWriter->SetHeaderFields(header);
        bool bUseLatestNamingFormat = (GetParameterInt("oldnf") == 0);
        m_agricPracticesDataWriter->SetUseLatestFileNamingFormat(bUseLatestNamingFormat);

        bool bOutputCsv = (GetParameterInt("outcsv") != 0);
        m_agricPracticesDataWriter->SetOutputCsvFormat(bOutputCsv);
        bool bCsvCompactMode = (GetParameterInt("csvcompact") != 0);
        m_agricPracticesDataWriter->SetCsvCompactMode(bCsvCompactMode);
        bool bMultiFileMode = (GetParameterInt("mfiles") != 0);
        m_agricPracticesDataWriter->SetUseMultiFileMode(bMultiFileMode);
    }

    otb::ogr::DataSource::Pointer GetVector(const otb::ogr::DataSource::Pointer &vectors, const FloatVectorImageType::Pointer &inputImg) {
        const std::string &imageProjectionRef = inputImg->GetProjectionRef();
        FloatVectorImageType::ImageKeywordlistType imageKwl = inputImg->GetImageKeywordlist();
        const std::string &vectorProjectionRef = vectors->GetLayer(GetParameterInt("layer")).GetProjectionRef();

        const OGRSpatialReference imgOGRSref = OGRSpatialReference( imageProjectionRef.c_str() );
        const OGRSpatialReference vectorOGRSref = OGRSpatialReference( vectorProjectionRef.c_str() );
        bool doReproj = true;
        // don't reproject for these cases
        if (  vectorProjectionRef.empty() || imgOGRSref.IsSame( &vectorOGRSref )
            || ( imageProjectionRef.empty() && imageKwl.GetSize() == 0) ) {
            doReproj = false;
        }

        GeometriesType::Pointer inputGeomSet;
        GeometriesType::Pointer outputGeomSet;
        ProjectionFilterType::Pointer geometriesProjFilter;
        if (doReproj)
        {
            inputGeomSet = GeometriesType::New(vectors);
            otb::ogr::DataSource::Pointer reprojVector = otb::ogr::DataSource::New();
            outputGeomSet = GeometriesType::New(reprojVector);
            // Filter instantiation
            geometriesProjFilter = ProjectionFilterType::New();
            geometriesProjFilter->SetInput(inputGeomSet);
            if (imageProjectionRef.empty())
            {
                geometriesProjFilter->SetOutputKeywordList(inputImg->GetImageKeywordlist()); // nec qd capteur
            }
            geometriesProjFilter->SetOutputProjectionRef(imageProjectionRef);
            geometriesProjFilter->SetOutput(outputGeomSet);
            otbAppLogINFO("Reprojecting input vectors ...");
            geometriesProjFilter->Update();
            otbAppLogINFO("Reprojecting input vectors done!");
            return reprojVector;
        }

        // if no reprojection, return the original vectors
        return vectors;
    }

    bool NeedsReprojection(const otb::ogr::DataSource::Pointer &vectors, const FloatVectorImageType::Pointer &inputImg) {
        const std::string &imageProjectionRef = inputImg->GetProjectionRef();
        FloatVectorImageType::ImageKeywordlistType imageKwl = inputImg->GetImageKeywordlist();
        const std::string &vectorProjectionRef = vectors->GetLayer(GetParameterInt("layer")).GetProjectionRef();

        const OGRSpatialReference imgOGRSref = OGRSpatialReference( imageProjectionRef.c_str() );
        const OGRSpatialReference vectorOGRSref = OGRSpatialReference( vectorProjectionRef.c_str() );
        bool doReproj = true;
        // don't reproject for these cases
        if (  vectorProjectionRef.empty() || imgOGRSref.IsSame( &vectorOGRSref )
            || ( imageProjectionRef.empty() && imageKwl.GetSize() == 0) ) {
            doReproj = false;
        }

        return doReproj;
    }

    std::string BuildUniqueFileName(const std::string &targetDir, const std::string &refFileName) {
        bool bOutputCsv = (GetParameterInt("outcsv") != 0);
        boost::filesystem::path rootFolder(targetDir);
        boost::filesystem::path pRefFile(refFileName);
        std::string fileName = pRefFile.stem().string() + (bOutputCsv ? ".csv" : ".xml");
        return (rootFolder / fileName).string();
    }

    private:
        ConcatenateImagesFilterType::Pointer m_concatenateImagesFilter;
        //ReadersListType::Pointer m_Readers;
        ImageReaderType::Pointer m_ImageReader;
        GenericMapContainer         m_GenericMapContainer;

        AgricPracticesWriterType2::Pointer m_agricPracticesDataWriter;

        std::vector<S2MaskedInputFileInfoType> m_s2MaskedInputFiles;
};

} // end of namespace Wrapper
} // end of namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::AgricPractDataExtraction)
