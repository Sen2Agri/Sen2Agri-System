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

#include "ImageResampler.h"
#include "GenericRSImageResampler.h"

#include "../../../Common/Filters/otbStreamingStatisticsMapFromLabelImageFilter.h"

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
    template< class TInput, class TOutput>
    class IntensityToDecibelsFunctor
    {
    public:
        IntensityToDecibelsFunctor() {}
        ~IntensityToDecibelsFunctor() {}

      bool operator!=( const IntensityToDecibelsFunctor &a) const
      {
        return false;
      }
      bool operator==( const IntensityToDecibelsFunctor & other ) const
      {
        return !(*this != other);
      }
      inline TOutput operator()( const TInput & A ) const
      {
          TOutput ret(A.GetSize());
          for (int i = 0; i<A.GetSize(); i++) {
              ret[i] = (10 * log10(A[i]));
          }
          return ret;
      }
    };

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

    typedef FloatVectorImageType                                                     FeatureImageType;
    typedef Int32ImageType                                                           ClassImageType;
    typedef otb::StreamingStatisticsMapFromLabelImageFilter<FeatureImageType, ClassImageType> StatisticsFilterType;
    typedef otb::ImageFileReader<ClassImageType>                             ClassImageReaderType;

    typedef itk::UnaryFunctorImageFilter<FeatureImageType,FeatureImageType,
                    IntensityToDecibelsFunctor<
                        FeatureImageType::PixelType,
                        FeatureImageType::PixelType> > IntensityToDbFilterType;


    typedef struct {
        std::string inputImage;
        std::vector<std::string> s2MsksFiles;
    } InputFileInfoType;

private:
    AgricPractDataExtraction()
    {
        m_concatenateImagesFilter = ConcatenateImagesFilterType::New();
        //m_Readers = ReadersListType::New();
        m_bOutputMinMax = false;
        m_bUseS2RasterMasks = false;
        m_fieldName = SEQ_UNIQUE_ID;
        m_bConvToDb = false;
        m_noDataValue = 0;
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

        AddParameter(ParameterType_StringList, "il", "Input Images or file containing the list");
        SetParameterDescription("il", "Support images that will be classified or a file containing these images");

        AddParameter(ParameterType_InputFilename, "vec", "Input vectors");
        SetParameterDescription("vec","Input geometries to analyze");
        MandatoryOff("vec");

        AddParameter(ParameterType_InputFilename, "filterids", "File containing ids to be filtered");
        SetParameterDescription("filterids","File containing ids to be filtered i.e. the monitorable parcels ids");
        MandatoryOff("filterids");

        AddParameter(ParameterType_String, "outdir", "Output directory for writing agricultural practices data extractin files");
        SetParameterDescription("outdir","Output directory to store agricultural practices data extractin files (txt format)");

        AddParameter(ParameterType_ListView, "field", "Field Name");
        SetParameterDescription("field","Name of the field carrying the class name in the input vectors.");
        //SetListViewSingleSelectionMode("field",true);

        AddParameter(ParameterType_Int, "layer", "Layer Index");
        SetParameterDescription("layer", "Layer index to read in the input vector file.");
        MandatoryOff("layer");
        SetDefaultParameterInt("layer",0);

//        AddParameter(ParameterType_StringList, "s2il", "S2 tiles parcels masks images");
//        SetParameterDescription("s2il", "S2 tiles parcels masks images");
//        MandatoryOff("s2il");

//        AddParameter(ParameterType_StringList, "s2ilcnts", "Number of S2 tiles parcels masks images for each input image");
//        SetParameterDescription("s2ilcnts", "Number of S2 tiles parcels masks images for each input image");
//        MandatoryOff("s2ilcnts");

        AddParameter(ParameterType_String, "prdtype", "Input product type (AMP/COHE/NDVI)");
        SetParameterDescription("prdtype", "Input product type (AMP/COHE/NDVI)");

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

        AddParameter(ParameterType_Int, "ifiles", "Output individual CVS files for input image");
        SetParameterDescription("ifiles", "Output individual CVS files for input image.");
        MandatoryOff("ifiles");
        SetDefaultParameterInt("ifiles",0);

        AddParameter(ParameterType_Int, "mfiles", "Output individual CVS files for each polygon");
        SetParameterDescription("mfiles", "If CSV format is used, then setting to true will produce individual files for each polygon.");
        MandatoryOff("mfiles");
        SetDefaultParameterInt("mfiles",0);

        AddParameter(ParameterType_InputImage,  "mask",   "Input validity mask");
        SetParameterDescription("mask", "Validity mask (only pixels corresponding to a mask value greater than 0 will be used for statistics)");
        MandatoryOff("mask");

        AddParameter(ParameterType_Int, "minmax", "Extract the min and max statistics.");
        SetParameterDescription("minmax", "Extracts also the minimum and maximum for each parcel");
        MandatoryOff("minmax");
        SetDefaultParameterInt("minmax",0);

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
            ogr::DataSource::Pointer ogrDS = ogr::DataSource::New(vectorFile, ogr::DataSource::Modes::Read);
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

                if(fieldType == OFTString || fieldType == OFTInteger || fieldType == OFTReal /* || ogr::version_proxy::IsOFTInteger64(fieldType)*/)
                {
                    std::string tmpKey="field."+key.substr(0, end - key.begin());
                    AddChoice(tmpKey,item);
                }
            }
        }
    }

    void DoExecute() override
    {
        m_bOutputMinMax = (GetParameterInt("minmax") != 0);
        const std::string &prdType = GetParameterAsString("prdtype");
        if (prdType == "AMP" && !m_bOutputMinMax) {
            m_bConvToDb = true;
        } else if (prdType == "NDVI") {
            m_noDataValue = NO_DATA_VALUE;
        }

        // Retrieve the field name
        if (HasValue("vec")) {
            const std::vector<int> &selectedCFieldIdx = GetSelectedItems("field");
            if(selectedCFieldIdx.empty())
            {
                otbAppLogWARNING(<<"No field has been selected for data labelling! Using " << SEQ_UNIQUE_ID);
            } else {
                const std::vector<std::string> &cFieldNames = GetChoiceNames("field");
                m_fieldName = cFieldNames[selectedCFieldIdx.front()];
            }
        }

        const std::vector<std::string> &imagesPaths = this->GetParameterStringList("il");
        if(imagesPaths.size() == 0) {
            otbAppLogFATAL(<<"No image was given as input!");
        } else if(imagesPaths.size() == 1) {
            // If we heva only one file, we check if this is a CSV file containing the
            // products and the masked S2 rasters
            boost::filesystem::path pathObj(imagesPaths[0]);
            std::string ext = pathObj.extension().string();
            if (boost::iequals(ext, ".csv")) {
                m_bUseS2RasterMasks = true;
            }
        }
        if (!m_bUseS2RasterMasks && !HasValue("vec")) {
            otbAppLogFATAL(<<"When S2 rasters files are not used, a shapefile should be provided via vec parameter!");
        }

        // Load the file containing the parcel ids filters, if specified
        LoadMonitorableParcelIdsFilters();

        // Initializes the internal image infos (with or without S2 masks)
        InitializeInputImageInfos(imagesPaths);

        m_bIndividualOutFilesForInputs = (GetParameterInt("ifiles") != 0);

        // Initialize the writer
        if (!m_bIndividualOutFilesForInputs) {
            InitializeWriter(imagesPaths);
        }

        const std::string &outDir = this->GetParameterString("outdir");

        int i = 1;

        // Reproject geometries
        const std::string &vectFile = this->GetParameterString("vec");
        if (vectFile.size() > 0) {
            otbAppLogINFO("Loading vectors from file " << vectFile);
            m_vectors = otb::ogr::DataSource::New(vectFile);
        }
        otb::ogr::DataSource::Pointer reprojVector = m_vectors;

        for (std::vector<InputFileInfoType>::const_iterator itInfos = m_InputFilesInfos.begin();
             itInfos != m_InputFilesInfos.end(); ++itInfos)
        {
            if ( !boost::filesystem::exists(itInfos->inputImage) ) {
                otbAppLogWARNING("File " << itInfos->inputImage << " does not exist on disk!");
                continue;
            }
            FloatVectorImageType::Pointer inputImage = GetInputImage(itInfos->inputImage);
            otbAppLogINFO("Handling file " << itInfos->inputImage);

            // if individual files for each input file, create a new writer for this image
            if (m_bIndividualOutFilesForInputs) {
                const std::vector<std::string> &imgPaths = {itInfos->inputImage};
                InitializeWriter(imgPaths);
            }

            if (itInfos->s2MsksFiles.size() == 0) {
                HandleImageUsingShapefile(inputImage, itInfos->inputImage, reprojVector);
            } else {
                // Handle the case when we have S2 tiles as polygon masks
                HandleImageUsingS2TilesParcelInfos(inputImage, *itInfos);
            }

            // if individual files for each input file, write the entries for this image
            if (m_bIndividualOutFilesForInputs) {
                otbAppLogINFO("Writing outputs to folder " << outDir);
                m_agricPracticesDataWriter->Update();
                otbAppLogINFO("Writing outputs to folder done!");
            }

            otbAppLogINFO("Processed " << i << " products. Remaining products = " << m_InputFilesInfos.size() - i <<
                          ". Percent completed = " << (int)((((double)i) / m_InputFilesInfos.size()) * 100) << "%");

            i++;
        }
        if (!m_bIndividualOutFilesForInputs) {
            otbAppLogINFO("Writing outputs to folder " << outDir);
            m_agricPracticesDataWriter->Update();
            otbAppLogINFO("Writing outputs to folder done!");
        }
    }

    void HandleImageUsingShapefile(const FloatVectorImageType::Pointer &inputImage, const std::string &imgPath,
                                   otb::ogr::DataSource::Pointer &reprojVector) {
        if (NeedsReprojection(reprojVector, inputImage)) {
            otbAppLogINFO("Reprojecting vectors needed for file " << imgPath);
            reprojVector = GetVector(m_vectors, inputImage);
        } else {
            otbAppLogINFO("No need to reproject vectors for " << imgPath);
        }
        FilterType::Pointer filter = GetStatisticsFilter(inputImage, reprojVector, m_fieldName);
        const FilterType::PixeMeanStdDevlValueMapType &meanStdValues = filter->GetMeanStdDevValueMap();
        const FilterType::PixelValueMapType &minValues = filter->GetMinValueMap();
        const FilterType::PixelValueMapType &maxValues = filter->GetMaxValueMap();
        const FilterType::PixelValueMapType &validPixelsCntValues = filter->GetValidPixelsCntMap();
        const FilterType::PixelValueMapType &invalidPixelsCntValues = filter->GetInvalidPixelsCntMap();
        m_agricPracticesDataWriter->AddInputMap<FilterType::PixeMeanStdDevlValueMapType,
                FilterType::PixelValueMapType>(imgPath, meanStdValues, minValues, maxValues,
                                               validPixelsCntValues, invalidPixelsCntValues);

        otbAppLogINFO("Extracted a number of " << meanStdValues.size() << " values for file " << imgPath);

        filter->GetFilter()->Reset();
    }

    void HandleImageUsingS2TilesParcelInfos(const FloatVectorImageType::Pointer &inputImage, const InputFileInfoType &imgInfos) {
        std::vector<std::string>::const_iterator it;
        FilterType::PixeMeanStdDevlValueMapType fieldsMap;
        for (it = imgInfos.s2MsksFiles.begin(); it != imgInfos.s2MsksFiles.end(); ++it) {
            StatisticsFilterType::Pointer statisticsFilter = StatisticsFilterType::New();
            // cut the input image according to the class image
            otbAppLogINFO("Extracting statistics for input image " << imgInfos.inputImage << " and class file " << *it);
            ClassImageType::Pointer classImg = GetClassImage(*it);
            const FloatVectorImageType::Pointer &cutInputImage = CutImage(inputImage, classImg);

            if (m_bConvToDb) {
                m_IntensityToDbFunctor = IntensityToDbFilterType::New();
                m_IntensityToDbFunctor->SetInput(cutInputImage);
                m_IntensityToDbFunctor->UpdateOutputInformation();
                statisticsFilter->SetInput(m_IntensityToDbFunctor->GetOutput());
            } else {
                statisticsFilter->SetInput(cutInputImage);
            }
            statisticsFilter->SetInputLabelImage(classImg);
            statisticsFilter->SetNoDataValue(m_noDataValue);
            statisticsFilter->SetUseNoDataValue(true);

            AddProcess(statisticsFilter->GetStreamer(), "Computing features...");
            statisticsFilter->Update();

            const auto &meanValues = statisticsFilter->GetMeanValueMap();
            const auto &stdDevValues = statisticsFilter->GetStandardDeviationValueMap();
            const auto &countValidValues = statisticsFilter->GetPixelCountMap();
            const auto &countAllValues = statisticsFilter->GetLabelPopulationMap();

//            for (otb::ogr::DataSource::const_iterator lb=m_vectors->begin(), le=m_vectors->end(); lb != le; ++lb)
//            {
//                otb::ogr::Layer const& inputLayer = *lb;
//                otb::ogr::Layer::const_iterator featIt = inputLayer.begin();
//                int colIndex = -1;
//                for(; featIt!=inputLayer.end(); ++featIt)
//                {
//                    OGRFeature &ogrFeat = (*featIt).ogr();
//                    if (colIndex == -1) {
//                        colIndex = ogrFeat.GetFieldIndex(m_fieldName.c_str());
//                    }
//                    if (colIndex == -1) {
//                        otbAppLogFATAL("Column for the unique index " << m_fieldName << " does not exists in the shapefile!")
//                    }
//                    int fieldValue = ogrFeat.GetFieldAsInteger(colIndex);
            StatisticsFilterType::PixelValueMapType::const_iterator itMean;
            StatisticsFilterType::PixelValueMapType::const_iterator itStdDev;
            StatisticsFilterType::PixelCountMapType::const_iterator itCountVald;
            StatisticsFilterType::LabelPopulationMapType::const_iterator itCountAll;
            int fieldValue;
            for(std::map<std::string,int>::const_iterator iter = m_FilterIdsMap.begin(); iter != m_FilterIdsMap.end(); ++iter)
            {
                fieldValue =  iter->second;
                itCountVald = countValidValues.find(fieldValue);
                if (itCountVald != countValidValues.end()) {
                    int validCnt = itCountVald->second[0];
                    itCountAll = countAllValues.find(fieldValue);
                    int allCnt = itCountAll->second;

                    if ((validCnt == 0) || (validCnt <= 0.1 * allCnt)) {
                        continue;
                    }
                    itMean = meanValues.find(fieldValue);
                    itStdDev = stdDevValues.find(fieldValue);
                    if (std::isnan(itMean->second[0]) || std::isnan(itStdDev->second[0])) {
                        continue;
                    }
                    fieldsMap[iter->first].mean = itMean->second;
                    fieldsMap[iter->first].stdDev = itStdDev->second;
                }
            }
//            m_agricPracticesDataWriter->AddInputMaps<StatisticsFilterType::PixelValueMapType>(imgInfos.inputImage,
//                                                                                             meanValues, stdDevValues);
        }
        FilterType::PixelValueMapType minMap;// = statisticsFilter->GetMinValueMap();
        FilterType::PixelValueMapType maxMap;// = statisticsFilter->GetMaxValueMap();
        FilterType::PixelValueMapType validPixelsCntMap;// = statisticsFilter->GetMaxValueMap();
        FilterType::PixelValueMapType invalidPixelsCntMap;// = statisticsFilter->GetMaxValueMap();
        m_agricPracticesDataWriter->AddInputMap<FilterType::PixeMeanStdDevlValueMapType,
                FilterType::PixelValueMapType>(imgInfos.inputImage, fieldsMap, minMap, maxMap,
                                               validPixelsCntMap, invalidPixelsCntMap);
    }

    FilterType::Pointer GetStatisticsFilter(const FloatVectorImageType::Pointer &inputImg,
                                            const otb::ogr::DataSource::Pointer &reprojVector,
                                            const std::string &fieldName)
    {
        FilterType::Pointer filter = FilterType::New();
        if (m_bConvToDb) {
            filter->SetConvertValuesToDecibels(m_bConvToDb);
            filter->SetInput(inputImg);
            // TODO: This can be switch as we get the same results
//            m_IntensityToDbFunctor = IntensityToDbFilterType::New();
//            m_IntensityToDbFunctor->SetInput(inputImg);
//            m_IntensityToDbFunctor->UpdateOutputInformation();
//            filter->SetInput(m_IntensityToDbFunctor->GetOutput());
        } else {
            filter->SetInput(inputImg);
        }

        if (IsParameterEnabled("mask") && HasValue("mask"))
        {
            filter->SetMask(this->GetParameterImage<UInt8ImageType>("mask"));
        }
        if (m_bOutputMinMax) {
            filter->SetComputeMinMax(true);
        }
        filter->SetFieldValueFilterIds(m_FilterIdsMap);
        filter->SetOGRData(reprojVector);
        filter->SetFieldName(fieldName);
        filter->SetLayerIndex(this->GetParameterInt("layer"));
        filter->GetStreamer()->SetAutomaticAdaptativeStreaming(GetParameterInt("ram"));

        AddProcess(filter->GetStreamer(),"Analyze polygons...");
        filter->Update();

        return filter;
    }

    FloatVectorImageType::Pointer GetInputImage(const std::string &imgPath) {
        ImageReaderType::Pointer imageReader = ImageReaderType::New();
        //m_Readers->PushBack(imageReader);
        m_ImageReader = imageReader;
        imageReader->SetFileName(imgPath);
        imageReader->UpdateOutputInformation();
        FloatVectorImageType::Pointer retImg = imageReader->GetOutput();
        retImg->UpdateOutputInformation();
        return retImg;
    }

    ClassImageType::Pointer GetClassImage(const std::string &imgPath) {
        ClassImageReaderType::Pointer imageReader = ClassImageReaderType::New();
        //m_Readers->PushBack(imageReader);
        m_classImageReader = imageReader;
        imageReader->SetFileName(imgPath);
        imageReader->UpdateOutputInformation();
        ClassImageType::Pointer retImg = imageReader->GetOutput();
        retImg->UpdateOutputInformation();
        return retImg;
    }

    void InitializeWriter(const std::vector<std::string> &imagesPaths) {
        const std::string &outDir = this->GetParameterString("outdir");
        m_agricPracticesDataWriter = AgricPracticesWriterType2::New();
        m_agricPracticesDataWriter->SetTargetFileName(BuildUniqueFileName(outDir, imagesPaths[0]));
        std::vector<std::string> header = {"KOD_PB", "date", "mean", "stdev"};
        if (m_bOutputMinMax) {
            header.push_back("min");
            header.push_back("max");
            header.push_back("valid_pixels_cnt");
            header.push_back("invalid_pixels_cnt");

            m_agricPracticesDataWriter->SetUseMinMax(true);
        }
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

    void InitializeInputImageInfos(const std::vector<std::string> &imagesPaths) {
//        if (HasValue("s2il")) {
//            const std::vector<std::string> &s2MsksPaths = this->GetParameterStringList("s2il");
//            if (HasValue("s2ilcnts")) {
//                const std::vector<std::string> &s2MsksCnts = this->GetParameterStringList("s2ilcnts");
//                if (imagesPaths.size() != s2MsksCnts.size()) {
//                    otbAppLogFATAL(<<"Invalid number of S2 masks given for the number of input images list! Expected: " <<
//                                   imagesPaths.size() << " but got " << s2MsksCnts.size());
//                }
//                // first perform a validation of the counts
//                size_t expectedS2Imgs = 0;
//                for (size_t i = 0; i<s2MsksCnts.size(); i++) {
//                    expectedS2Imgs += std::atoi(s2MsksCnts[i].c_str());
//                }
//                if (expectedS2Imgs != s2MsksPaths.size()) {
//                    otbAppLogFATAL(<<"The S2 masks number and the provided S2 files do not match! Expected " <<
//                                   expectedS2Imgs << " but got " << s2MsksPaths.size());
//                }
//                int curS2IlIdx = 0;
//                for (size_t i = 0; i<imagesPaths.size(); i++) {
//                    InputFileInfoType info;
//                    info.inputImage = imagesPaths[i];
//                    int curImgCnt = std::atoi(s2MsksCnts[i].c_str());
//                    for(int j = 0; j<curImgCnt; j++) {
//                        info.s2MsksFiles.emplace_back(s2MsksPaths[curS2IlIdx+j]);
//                    }
//                    m_InputFilesInfos.emplace_back(info);
//                    curS2IlIdx += curImgCnt;
//                }
//            } else {
//                otbAppLogWARNING("WARNING: Using all s2 mask files for each input image!!!");
//                for (const auto &inputImg: imagesPaths) {
//                    InputFileInfoType info;
//                    info.inputImage = inputImg;
//                    info.s2MsksFiles = s2MsksPaths;
//                    m_InputFilesInfos.emplace_back(info);
//                }
//            }
        if (m_bUseS2RasterMasks) {
            m_InputFilesInfos = LoadProductS2MasksCsvFile(imagesPaths[0]);
        } else {
            for (std::vector<std::string>::const_iterator itImages = imagesPaths.begin();
                 itImages != imagesPaths.end(); ++itImages) {
                InputFileInfoType info;
                info.inputImage = (*itImages);
                m_InputFilesInfos.emplace_back(info);
            }
        }
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

    FeatureImageType::Pointer CutImage(const FeatureImageType::Pointer &img, const ClassImageType::Pointer &clsImg) {
        FeatureImageType::Pointer retImg = img;

        double clsImgWidth = clsImg->GetLargestPossibleRegion().GetSize()[0];
        double clsImgHeight = clsImg->GetLargestPossibleRegion().GetSize()[1];

        //ImageType::SpacingType spacing = reader->GetOutput()->GetSpacing();
        float clsImgRes = static_cast<float>(clsImg->GetSpacing()[0]);
        ImageType::PointType  clsImgOrigin;
        clsImgOrigin = clsImg->GetOrigin();

        std::string clsImgProjRef = clsImg->GetProjectionRef();

        float imageWidth = img->GetLargestPossibleRegion().GetSize()[0];
        float imageHeight = img->GetLargestPossibleRegion().GetSize()[1];

        ImageType::PointType origin = img->GetOrigin();
        ImageType::PointType imageOrigin;
        imageOrigin[0] = origin[0];
        imageOrigin[1] = origin[1];

        if((imageWidth != clsImgWidth) || (imageHeight != clsImgHeight) ||
                (clsImgOrigin[0] != imageOrigin[0]) || (clsImgOrigin[1] != imageOrigin[1])) {

            Interpolator_Type interpolator = Interpolator_Linear;
            std::string imgProjRef = img->GetProjectionRef();
            // if the projections are equal
            if(imgProjRef == clsImgProjRef) {
                float curImgRes = static_cast<float>(img->GetSpacing()[0]);
                const float scale = (float)clsImgRes / curImgRes;
                // use the streaming resampler
                m_ImageResampler.SetNoDataValue(m_noDataValue);
                retImg = m_ImageResampler.getResampler(img, scale,clsImgWidth,
                            clsImgHeight,clsImgOrigin, interpolator)->GetOutput();
            } else {
                // use the generic RS resampler that allows reprojecting
                m_genericRSImageResampler.SetNoDataValue(m_noDataValue);
                retImg = m_genericRSImageResampler.getResampler(img, clsImg, interpolator)->GetOutput();
            }
            retImg->UpdateOutputInformation();
        }

        return retImg;
    }

    void LoadMonitorableParcelIdsFilters() {
        // Reproject geometries
        if (HasValue("filterids")) {
            const std::string &filterIdsFile = this->GetParameterString("filterids");
            if (boost::filesystem::exists(filterIdsFile ))
            {
                otbAppLogINFO("Loading filter IDs from file " << filterIdsFile);
                // load the indexes
                std::ifstream idxFileStream(filterIdsFile);
                std::string line;
                int i = 0;
                while (std::getline(idxFileStream, line)) {
                    // ignore the first line which is the header line
                    if (i == 0) {
                        i++;
                        continue;
                    }
                    NormalizeFieldId(line);
                    m_FilterIdsMap[line] = std::atoi(line.c_str());
                }
                otbAppLogINFO("Loading filter IDs file done!");
            }
        } else if (m_bUseS2RasterMasks) {
            otbAppLogFATAL("The filter IDs file should be provided when using the S2 raster masks");
        }
    }

    std::vector<InputFileInfoType> LoadProductS2MasksCsvFile(const std::string &fileName)
    {
        std::vector<InputFileInfoType> retFilesInfos;
        std::ifstream fStream(fileName);
        std::string line;
        int i = 0;
        while (std::getline(fStream, line)) {
            i++;
            const std::vector<std::string> &lineElems = CsvLineToVector(line);
            if (lineElems.size() <= 1) {
                otbAppLogWARNING("The line at index " << i << " will be ignored as it contains no S2 raster!");
                continue;
            }

            InputFileInfoType info;
            info.inputImage = lineElems[0];
            for(size_t j = 1; j<lineElems.size(); j++) {
                info.s2MsksFiles.emplace_back(lineElems[j]);
            }
            retFilesInfos.emplace_back(info);
        }
        return retFilesInfos;
    }

    std::vector<std::string> CsvLineToVector(const std::string &line)
    {
        std::vector<std::string> results;
        boost::algorithm::split(results, line, [](char c){return c == ';';});
        return results;
    }

    private:
        bool m_bConvToDb;
        int m_noDataValue;
        bool m_bOutputMinMax;
        std::string m_fieldName;
        otb::ogr::DataSource::Pointer m_vectors;
        bool m_bUseS2RasterMasks;
        ConcatenateImagesFilterType::Pointer m_concatenateImagesFilter;
        //ReadersListType::Pointer m_Readers;
        ImageReaderType::Pointer m_ImageReader;
        ClassImageReaderType::Pointer m_classImageReader;
        GenericMapContainer         m_GenericMapContainer;

        AgricPracticesWriterType2::Pointer m_agricPracticesDataWriter;

        std::vector<InputFileInfoType> m_InputFilesInfos;
        IntensityToDbFilterType::Pointer        m_IntensityToDbFunctor;

        ImageResampler<FeatureImageType, FeatureImageType>  m_ImageResampler;
        GenericRSImageResampler<FeatureImageType, FeatureImageType, ClassImageType>  m_genericRSImageResampler;

        bool m_bIndividualOutFilesForInputs;

        typedef std::map<std::string, int> FilterIdsMapType;
        FilterIdsMapType m_FilterIdsMap;
};

} // end of namespace Wrapper
} // end of namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::AgricPractDataExtraction)
