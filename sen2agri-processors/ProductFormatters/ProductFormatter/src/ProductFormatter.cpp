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

#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbOGRIOHelper.h"
#include "ogr_geometry.h"
#include <iostream>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <sys/wait.h>
#include <spawn.h>
#endif
#include <fstream>

#include "ProductMetadataWriter.hpp"
#include "TileMetadataWriter.hpp"

#include "MetadataHelperFactory.h"

#define PROJECT_ID                      "S2AGRI"
#define GIPP_VERSION                    "0001"
#define IMAGE_FORMAT                    "GEOTIFF"
#define TIF_EXTENSION                   ".TIF"
#define JPEG_EXTENSION                  ".jpg"

#define REFLECTANCE_SUFFIX              "SRFL"
#define WEIGHTS_SUFFIX                  "MWGT"
#define COMPOSITE_DATES_SUFFIX          "MDAT"
#define COMPOSITE_FLAGS_SUFFIX          "MFLG"
#define LAI_NDVI_SUFFIX                 "SNDVI"
#define LAI_MDATE_SUFFIX                "SLAIMONO"
#define FAPAR_MDATE_SUFFIX              "SFAPARMONO"
#define FCOVER_MDATE_SUFFIX             "SFCOVERMONO"
#define LAI_MDATE_ERR_SUFFIX            "MLAIERR"
#define LAI_REPR_SUFFIX                 "SLAIR"
#define LAI_FIT_SUFFIX                  "SLAIF"
#define PHENO_SUFFIX                    "SPHENO"
#define PHENO_FLAGS_SUFFIX              "MPHENOFLG"
#define LAI_MONO_DATE_FLAGS_SUFFIX      "MMONODFLG"

#define LAI_IN_DOMAIN_FLAGS_SUFFIX      "MINDOMFLG"
#define LAI_DOMAIN_FLAGS_SUFFIX         "MLAIDOMFLG"
#define FAPAR_DOMAIN_FLAGS_SUFFIX       "MFAPARDOMFLG"
#define FCOVER_DOMAIN_FLAGS_SUFFIX      "MFCOVERDOMFLG"

#define LAI_REPROC_FLAGS_SUFFIX         "MLAIRFLG"
#define LAI_FITTED_FLAGS_SUFFIX         "MLAIFFLG"
#define CROP_MASK_IMG_SUFFIX            "CM"
#define CROP_TYPE_IMG_SUFFIX            "CT"
#define CROP_TYPE_RAW_IMG_SUFFIX        "RAW"
#define CROP_MASK_RAW_IMG_SUFFIX        "RAW"
#define CROP_MASK_FLAGS_SUFFIX          "MCMFLG"
#define CROP_TYPE_FLAGS_SUFFIX          "MCTFLG"

#define GENERIC_CS_TYPE     "GEOGRAPHIC"
#define GENERIC_GEO_TABLES  "EPSG"


#define MAIN_FOLDER_CATEG "PRD"
#define LEGACY_FOLDER_CATEG "LY"
//#define TRUE_COLOR_FOLDER_CATEG "TCI"
#define QUICK_L0OK_IMG_CATEG "PVI"
#define METADATA_CATEG "MTD"
//#define MASK_CATEG "MSK"
#define PARAMETER_CATEG "IPP"
#define INSITU_CATEG "ISD"
#define LUT_CATEG       "LUT"
#define QUALITY_CATEG "QLT"
#define NO_DATA_VALUE "-10000"

#define ORIGINATOR_SITE "CSRO"

#define VECTOR_FOLDER_NAME          "VECTOR_DATA"
#define TILES_FOLDER_NAME           "TILES"
#define AUX_DATA_FOLDER_NAME        "AUX_DATA"
#define LEGACY_DATA_FOLDER_NAME     "LEGACY_DATA"
#define IMG_DATA_FOLDER_NAME        "IMG_DATA"
#define QI_DATA_FOLDER_NAME         "QI_DATA"

#define LANDSAT    "LANDSAT"
#define SENTINEL   "SENTINEL"

#define SEN2AGRI_L2A_PRD_LEVEL  "L2A"
#define SEN2AGRI_L3A_PRD_LEVEL  "L3A"
#define SEN2AGRI_L3B_PRD_LEVEL  "L3B"
#define SEN2AGRI_L3C_PRD_LEVEL  "L3C"
#define SEN2AGRI_L3D_PRD_LEVEL  "L3E"
#define SEN2AGRI_L3E_PRD_LEVEL  "L3E"
#define SEN2AGRI_L4A_PRD_LEVEL  "L4A"
#define SEN2AGRI_L4B_PRD_LEVEL  "L4B"

#define L2A_PRODUCT             "L2A product"
#define COMPOSITE_PRODUCT       "Composite"
#define LAI_MONO_DATE_PRODUCT   "LAI mono-date"
#define LAI_REPROC_PRODUCT      "LAI reprocessed"
#define LAI_FITTED_PRODUCT      "LAI reprocessed at end of season (fitted)"
#define PHENO_NDVI_PRODUCT      "Phenological NDVI Metrics"
#define CROP_MASK               "Crop mask"
#define CROP_TYPE               "Crop type"

struct Coord{
    double x;
    double y;
};

struct CompositeBand
{
    int iBandId;
    std::string strSpectralRange;
    std::string strSpectralDomain;
    int iSpatialResolution;
    std::string strBandName;
    std::string strSPOT4_5Name;
};
std::vector<CompositeBand> CompositeBandList = {
    {1, "490 nm", "Classical blue", 10, "BLUE", "FALSE_BLUE"},
    {2, "560 nm", "Green", 10, "GREEN", "GREEN"},
    {3, "665 nm", "Red", 10, "RED", "RED"},
    {4, "842 nm", "Near Infrared", 10, "NIR1", "NIR1"},
    {5, "705 nm", "Vegetation red-edge", 20, "RE1", ""},
    {6, "740 nm", "Vegetation red-edge", 20, "RE2", ""},
    {7, "783 nm", "Vegetation red-edge", 20, "RE3", ""},
    {8, "865 nm", "Vegetation red-edge", 20, "NIR2", ""},
    {9, "1610 nm", "Short-Wave infrared", 20, "SWIR1", "SWIR1"},
    {10, "2190 nm", "Short-Wave infrared", 20, "SWIR2", ""}
};

typedef enum{
    COMPOSITE_REFLECTANCE_RASTER = 0,
    COMPOSITE_WEIGHTS_RASTER,
    COMPOSITE_FLAGS_MASK,
    COMPOSITE_DATES_MASK,
    LAI_NDVI_RASTER,
    LAI_MONO_DATE_RASTER,
    LAI_MONO_DATE_ERR_RASTER,
    LAI_MONO_DATE_STATUS_FLAGS,
    LAI_MONO_DATE_IN_DOMAIN_FLAGS,
    LAI_MONO_DATE_DOMAIN_FLAGS,
    FAPAR_DOMAIN_FLAGS,
    FCOVER_DOMAIN_FLAGS,
    FAPAR_MONO_DATE_RASTER,
    FCOVER_MONO_DATE_RASTER,
    LAI_REPROC_FLAGS,
    LAI_FITTED_FLAGS,
    LAI_REPR_RASTER,
    LAI_FIT_RASTER,
    CROP_MASK_RASTER,
    RAW_CROP_MASK_RASTER,
    CROP_TYPE_RASTER,
    CROP_TYPE_RAW_RASTER,
    PHENO_RASTER,
    PHENO_FLAGS,
    CROP_MASK_FLAGS,
    CROP_TYPE_FLAGS,
    UNKONW_RASTER_TYPE
}rasterTypes;

struct rasterInfo
{
    std::string strRasterFileName;
    int nResolution;
    rasterTypes iRasterType;
    std::string strNewRasterFileName;
    std::string strTileID;
    bool bIsQiData;
    // quality data can be discrete values or float values.
    // if discrete values, we don't want to apply any linear resampling over them
    bool bQiDataIsDiscrete;
    std::string rasterTimePeriod;
    int nRasterExpectedBandsNo;
    std::string strNewRasterFullPath;
    bool bNeedsPreview;
};

struct qualityInfo
{
    std::string strFileName;
    std::string strTileID;
    std::string strRegion;
};

struct previewInfo
{
    std::string strPreviewFileName;
    std::string strTileID;
};

struct tileInfo
{
    std::string strTileID;
    std::string strTileNameRoot;
    std::string strTilePath;
    std::string strTileNameWithoutExt;
    TileFileMetadata tileMetadata;
};

struct geoProductInfo
{
    rasterTypes rasterType;
    int iResolution;
    std::vector<double>PosList;
    Bbox AreaOfInterest;
    CoordReferenceSystemMetadata CoordReferenceSystem;
};

namespace otb
{
namespace Wrapper
{
class ProductFormatter : public Application
{

public:
  typedef ProductFormatter Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  itkNewMacro(Self)
  itkTypeMacro(ProductFormatter, otb::Application)

private:

  void DoInit()
  {
        SetName("ProductFormatter");
        SetDescription("Creates folder ierarchy and metadata files");

        SetDocName("ProductFormatter");
        SetDocLongDescription("Creates folder ierarchy and metadata files");
        SetDocLimitations("None");
        SetDocAuthors("ATA");
        SetDocSeeAlso(" ");

        AddParameter(ParameterType_String, "destroot", "Destination root directory");
        AddParameter(ParameterType_String, "fileclass", "File class");
        AddParameter(ParameterType_String, "level", "Product level");
        AddParameter(ParameterType_String, "timeperiod", "First product date and last product date");
        MandatoryOff("timeperiod");
        AddParameter(ParameterType_String, "baseline", "Processing baseline");
        AddParameter(ParameterType_String, "outprops", "File containing processing properties like the product main folder");
        MandatoryOff("outprops");
        AddParameter(ParameterType_String, "siteid", "The site ID");

        AddParameter(ParameterType_Int, "compress", "Specifies if the rasters should be compressed or not upon product creation");
        MandatoryOff("compress");
        SetDefaultParameterInt("compress", 0);

        AddParameter(ParameterType_Int, "cog", "Specifies if the rasters should be translated into Cloud Optimized Geotiff upon product creation");
        MandatoryOff("cog");
        SetDefaultParameterInt("cog", 0);

        AddParameter(ParameterType_Choice, "processor", "Processor");
        SetParameterDescription("processor", "Specifies the product type");

        AddChoice("processor.composite", "Composite product");
        SetParameterDescription("processor.composite", "Specifies a Composite product");

        AddChoice("processor.vegetation", "Vegetation Status product");
        SetParameterDescription("processor.vegetation", "Specifies a Vegetation Status product");

        AddChoice("processor.phenondvi", "Phenological NDVI metrics product");
        SetParameterDescription("processor.phenondvi", "Specifies a Phenological NDVI metrics product");

        AddChoice("processor.croptype", "Crop type product");
        SetParameterDescription("processor.croptype", "Specifies a CropType product");

        AddChoice("processor.cropmask", "Crop mask product");
        SetParameterDescription("processor.cropmask", "Specifies a CropMask product");

        AddChoice("processor.generic", "A generic product where files are copied as they are");
        SetParameterDescription("processor.generic", "A generic product that will be simply created by copying "
                                                    "input file and no other operation");

  //composite parameters
        AddParameter(ParameterType_InputFilenameList, "processor.composite.refls", "Reflectance raster files list for composite separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.composite.refls");

        AddParameter(ParameterType_InputFilenameList, "processor.composite.weights", "Weights raster files list for composite separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.composite.weights");

        AddParameter(ParameterType_InputFilenameList, "processor.composite.flags", "Flags mask files list for composite separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.composite.flags");

        AddParameter(ParameterType_InputFilenameList, "processor.composite.dates", "Dates mask files list for composite separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.composite.dates");

        AddParameter(ParameterType_InputFilenameList, "processor.composite.rgb", "TIFF file to be used to obtain preview for SPOT product separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.composite.rgb");
 //vegetation parameters

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.laindvi", "LAI NDVI raster files list for vegetation  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.laindvi");

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.laimonodate", "LAI Mono-date raster files list for vegetation  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.laimonodate");

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.laimonodateerr", "LAI Mono-date Error raster files list for vegetation  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.laimonodateerr");

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.faparmonodate", "LAI FAPAR Mono-date raster files list for vegetation  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.faparmonodate");

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.fcovermonodate", "LAI FCOVER Mono-date raster files list for vegetation  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.fcovermonodate");

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.laistatusflgs", "LAI Mono date flags raster files list for vegetation  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.laistatusflgs");

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.indomainflgs", "LAI input domain flags raster files list for vegetation  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.indomainflgs");

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.laidomainflgs", "LAI output domain flags raster files list for vegetation  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.laidomainflgs");

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.fapardomainflgs", "FAPAR output domain flags raster files list for vegetation  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.fapardomainflgs");

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.fcoverdomaniflgs", "FCOVER output domain flags raster files list for vegetation  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.fcoverdomaniflgs");

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.filelaireproc", "File containing the LAI REPR raster files list for vegetation "
                                                                                            "separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.filelaireproc");

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.filelaireprocflgs", "File containing the LAI Reprocessing flags raster files "
                                                                                                "list for vegetation  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.filelaireprocflgs");

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.filelaifit", "File containing the LAI FIT raster files list for vegetation  "
                                                                                         "separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.filelaifit");

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.filelaifitflgs", "File containing the LAI Fitted flags raster files list for "
                                                                                             "vegetation  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.filelaifitflgs");

        AddParameter(ParameterType_InputFilenameList, "processor.phenondvi.metrics", "Phenological NDVI metrics raster files list for vegetation  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.phenondvi.metrics");
        AddParameter(ParameterType_InputFilenameList, "processor.phenondvi.flags", "Flags files list for phenological NDVI metrics separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.phenondvi.flags");


//crop type parameters
        AddParameter(ParameterType_InputFilenameList, "processor.croptype.file", "CROP TYPE raster files, separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.croptype.file");

        AddParameter(ParameterType_InputFilenameList, "processor.croptype.rawfile", "CROP TYPE unmasked raster files, separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.croptype.rawfile");

        AddParameter(ParameterType_InputFilenameList, "processor.croptype.quality", "CROP TYPE quality file");
        MandatoryOff("processor.croptype.quality");

        AddParameter(ParameterType_InputFilenameList, "processor.croptype.flags", "CROP TYPE flags file");
        MandatoryOff("processor.croptype.flags");

//crop mask parameters
        AddParameter(ParameterType_InputFilenameList, "processor.cropmask.file", "CROP MASK raster file  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.cropmask.file");

        AddParameter(ParameterType_InputFilenameList, "processor.cropmask.rawfile", "CROP MASK raw raster file  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.cropmask.rawfile");

        AddParameter(ParameterType_InputFilenameList, "processor.cropmask.quality", "CROP MASK quality file");
        MandatoryOff("processor.cropmask.quality");

        AddParameter(ParameterType_InputFilenameList, "processor.cropmask.flags", "CROP MASK flags files");
        MandatoryOff("processor.cropmask.flags");

// Generic product
        AddParameter(ParameterType_InputFilenameList, "processor.generic.files", "Generic product files");
        MandatoryOff("processor.generic.files");

// Other parameters
        AddParameter(ParameterType_InputFilenameList, "il", "The xml files");
        MandatoryOff("il");

        /*AddParameter(ParameterType_InputFilenameList, "preview", "The quicklook files");
        MandatoryOff("preview");*/
        AddParameter(ParameterType_InputFilenameList, "gipp", "The GIPP files");
        MandatoryOff("gipp");

        AddParameter(ParameterType_InputFilenameList, "isd", "In-situ data");
        MandatoryOff("isd");

        AddParameter(ParameterType_String, "lut", "The LUT file");
        MandatoryOff("lut");

        AddParameter(ParameterType_String, "lutqgis", "QGIS color map file");
        MandatoryOff("lutqgis");

        AddParameter(ParameterType_Int, "aggregatescale", "The aggregate rescale resolution");
        MandatoryOff("aggregatescale");
        SetDefaultParameterInt("aggregatescale", 60);

        AddParameter(ParameterType_Int, "vectprd", "Specifies if the product is a vector or raster product");
        MandatoryOff("vectprd");
        SetDefaultParameterInt("vectprd", 0);

        SetDocExampleParameterValue("destroot", "/home/ata/sen2agri/sen2agri-processors-build/Testing/Temporary/Dest");
        SetDocExampleParameterValue("fileclass", "SVT1");
        SetDocExampleParameterValue("level", "L3A");
        SetDocExampleParameterValue("timeperiod", "20130228_20130615");
        SetDocExampleParameterValue("baseline", "01.00");
        SetDocExampleParameterValue("il", "image1.xml image2.hdr");
        SetDocExampleParameterValue("gipp", "gippFile1.xml gippFile2.txt");


  }

  void DoUpdateParameters()
  {
  }

  void DoExecute()
  {
      // The Quicklook OTB app spawns thousands of threads, try to avoid that
      setenv("ITK_USE_THREADPOOL", "1", 0);

      m_bVectPrd = (this->GetParameterInt("vectprd") != 0);

      // by default, we expect a "timeperiod" parameter
      m_bDynamicallyTimePeriod = false;

      //get product level
      m_strProductLevel = this->GetParameterString("level");

      //get time period (first product date and last product date)
      if(HasValue("timeperiod")) {
        m_strTimePeriod = this->GetParameterString("timeperiod");
      }

      //get time period (first product date and last product date)
      if(HasValue("lut")) {
        m_strLutFile = this->GetParameterString("lut");
      }

      //get processing baseline
      m_strBaseline = this->GetParameterString("baseline");

      //get the site ID
      m_strSiteId = this->GetParameterString("siteid");

      //get destination root
      m_strDestRoot = this->GetParameterString("destroot");

      // Get GIPP file list
      m_GIPPList = this->GetParameterStringList("gipp");

      // Get ISD file list
      m_ISDList = this->GetParameterStringList("isd");

      //read .xml or .HDR files to fill the metadata structures
      // Get the list of input files
      if (HasValue("il")) {
          const std::vector<std::string> &descriptors = this->GetParameterStringList("il");
          LoadAllDescriptors(descriptors);
      }
      if(m_strTimePeriod.empty() && m_acquisitionDatesList.size() > 0) {
          const std::string &strMinAcquisitionDate = *std::min_element(std::begin(m_acquisitionDatesList), std::end(m_acquisitionDatesList));
          const std::string &strMaxAcquisitionDate = *std::max_element(std::begin(m_acquisitionDatesList), std::end(m_acquisitionDatesList));
          m_bDynamicallyTimePeriod = true;
          const std::string &minAcqDate = getDateFromAquisitionDateTime(strMinAcquisitionDate);
          const std::string &maxAcqDate = getDateFromAquisitionDateTime(strMaxAcquisitionDate);
          if(LevelHasAcquisitionTime()) {
              if(minAcqDate != maxAcqDate) {
                    itkGenericExceptionMacro(<< "You should have the same date for all tiles in the il parameter as this product has Aquisition time: " << m_strProductLevel);
              }
              // we have a single date and min acquisition date should be the same as max acquisition date
              m_strTimePeriod = strMaxAcquisitionDate;
          } else {
              // we have an interval
              m_strTimePeriod = minAcqDate + "_" + maxAcqDate;
          }
      }

      m_strProductDirectoryName = BuildProductDirectoryName();
      std::string strMainFolderFullPath = m_strDestRoot + "/" + m_strProductDirectoryName;

      // Create the lock file
      const std::string &lockFileName = createMainFolderLockFile(strMainFolderFullPath);
      //created all folders ierarchy
      bool bDirStructBuiltOk = createsAllFolders(strMainFolderFullPath);

      //if is composite read reflectance rasters, weights rasters, flags masks, dates masks
      if (!m_bVectPrd) {
          if (m_strProductLevel == SEN2AGRI_L3A_PRD_LEVEL)
          {
              std::vector<std::string> rastersList;

               // Get reflectance raster file list
              rastersList = this->GetParameterStringList("processor.composite.refls");
              //unpack and add them in global raster list
              UnpackRastersList(rastersList, COMPOSITE_REFLECTANCE_RASTER, false);

              //get weights rasters list
              rastersList = this->GetParameterStringList("processor.composite.weights");
              UnpackRastersList(rastersList, COMPOSITE_WEIGHTS_RASTER, true, false);

              rastersList = this->GetParameterStringList("processor.composite.flags");
              UnpackRastersList(rastersList, COMPOSITE_FLAGS_MASK, true);

              rastersList = this->GetParameterStringList("processor.composite.dates");
              UnpackRastersList(rastersList, COMPOSITE_DATES_MASK, true);

              rastersList = this->GetParameterStringList("processor.composite.rgb");
              std::string strTileID("");

              for (const auto &rasterFileEl : rastersList) {
                  if(rasterFileEl.compare(0, 5, "TILE_") == 0)
                  {
                      //if is TILE separator, read tileID
                      strTileID = rasterFileEl.substr(5, rasterFileEl.length() - 5);
                  }
                  else
                  {
                      previewInfo previewInfoEl;
                      previewInfoEl.strTileID = strTileID;
                      previewInfoEl.strPreviewFileName = rasterFileEl;
                      m_previewList.emplace_back(previewInfoEl);
                  }
              }
         }

          // read LAI Mono-date raster files list
          if (m_strProductLevel == SEN2AGRI_L3B_PRD_LEVEL) {
              std::vector<std::string> rastersList;

              rastersList = this->GetParameterStringList("processor.vegetation.laindvi");
              UnpackRastersList(rastersList, LAI_NDVI_RASTER, false);
              rastersList = this->GetParameterStringList("processor.vegetation.laimonodate");
              UnpackRastersList(rastersList, LAI_MONO_DATE_RASTER, false);

              rastersList = this->GetParameterStringList("processor.vegetation.faparmonodate");
              UnpackRastersList(rastersList, FAPAR_MONO_DATE_RASTER, false);
              rastersList = this->GetParameterStringList("processor.vegetation.fcovermonodate");
              UnpackRastersList(rastersList, FCOVER_MONO_DATE_RASTER, false);

              rastersList = this->GetParameterStringList("processor.vegetation.laimonodateerr");
              UnpackRastersList(rastersList, LAI_MONO_DATE_ERR_RASTER, true, false);
              rastersList = this->GetParameterStringList("processor.vegetation.laistatusflgs");
              UnpackRastersList(rastersList, LAI_MONO_DATE_STATUS_FLAGS, true);

              rastersList = this->GetParameterStringList("processor.vegetation.indomainflgs");
              UnpackRastersList(rastersList, LAI_MONO_DATE_IN_DOMAIN_FLAGS, true);

              rastersList = this->GetParameterStringList("processor.vegetation.laidomainflgs");
              UnpackRastersList(rastersList, LAI_MONO_DATE_DOMAIN_FLAGS, true);

              rastersList = this->GetParameterStringList("processor.vegetation.fapardomainflgs");
              UnpackRastersList(rastersList, FAPAR_DOMAIN_FLAGS, true);

              rastersList = this->GetParameterStringList("processor.vegetation.fcoverdomaniflgs");
              UnpackRastersList(rastersList, FCOVER_DOMAIN_FLAGS, true);
          }

          // read LAIREPR raster files list
          if (m_strProductLevel == SEN2AGRI_L3C_PRD_LEVEL) {
              std::vector<std::string> rastersList;

              // LAI Reprocessed raster files list
              rastersList = this->GetFileListFromFile(this->GetParameterStringList("processor.vegetation.filelaireproc"));
              UnpackRastersList(rastersList, LAI_REPR_RASTER, false);
              rastersList = this->GetFileListFromFile(this->GetParameterStringList("processor.vegetation.filelaireprocflgs"));
              UnpackRastersList(rastersList, LAI_REPROC_FLAGS, true);
         }

          // read LAIFIT raster files list
          if (m_strProductLevel == SEN2AGRI_L3D_PRD_LEVEL) {
              std::vector<std::string> rastersList;

              rastersList = this->GetFileListFromFile(this->GetParameterStringList("processor.vegetation.filelaifit"));
              UnpackRastersList(rastersList, LAI_FIT_RASTER, false);
              rastersList = this->GetFileListFromFile(this->GetParameterStringList("processor.vegetation.filelaifitflgs"));
              UnpackRastersList(rastersList, LAI_FITTED_FLAGS, true);
         }

          //read Phenological NDVI metrics raster files list
          if (m_strProductLevel == SEN2AGRI_L3E_PRD_LEVEL) {
              std::vector<std::string> rastersList;
              rastersList = this->GetParameterStringList("processor.phenondvi.metrics");
              UnpackRastersList(rastersList, PHENO_RASTER, false);
              rastersList = this->GetParameterStringList("processor.phenondvi.flags");
              UnpackRastersList(rastersList, PHENO_FLAGS, true);
         }

          //if is CROP MASK, read raster file
          if (m_strProductLevel == SEN2AGRI_L4A_PRD_LEVEL)
          {

              std::vector<std::string> filesList;
              filesList = this->GetParameterStringList("processor.cropmask.file");
              UnpackRastersList(filesList, CROP_MASK_RASTER, false);

              filesList = this->GetParameterStringList("processor.cropmask.rawfile");
              UnpackRastersList(filesList, RAW_CROP_MASK_RASTER, false);

              //get quality file list
              filesList = this->GetParameterStringList("processor.cropmask.quality");
              UnpackQualityFlagsList(filesList);

              // get the CropMask flags
              filesList = this->GetParameterStringList("processor.cropmask.flags");
              UnpackRastersList(filesList, CROP_MASK_FLAGS, true);
          }

          //if is CROP TYPE, read raster file
          if (m_strProductLevel == SEN2AGRI_L4B_PRD_LEVEL)
          {
              // Get reflectance raster file list
              std::vector<std::string> filesList;
              filesList = this->GetParameterStringList("processor.croptype.file");
              UnpackRastersList(filesList, CROP_TYPE_RASTER, false);

              filesList = this->GetParameterStringList("processor.croptype.rawfile");
              UnpackRastersList(filesList, CROP_TYPE_RAW_RASTER, false);

              //get quality file list
              filesList = this->GetParameterStringList("processor.croptype.quality");
              UnpackQualityFlagsList(filesList);

              // get the CropType flags
              filesList = this->GetParameterStringList("processor.croptype.flags");
              UnpackRastersList(filesList, CROP_TYPE_FLAGS, true);
          }
      } else {
          const std::vector<std::string> &rastersList = this->GetParameterStringList("processor.generic.files");
          TransferGenericVectorFiles(rastersList);
      }
      if(bDirStructBuiltOk)
      {
          for (tileInfo &tileEl : m_tileIDList) {
              std::cout << "TileID =" << tileEl.strTileID << "  strTilePath =" << tileEl.strTilePath  << std::endl;
              CreateAndFillTile(tileEl, strMainFolderFullPath);
          }

          TransferPreviewFiles();
          if (HasValue("lutqgis")) {
              TransferAndRenameLUTFile(GetParameterString("lutqgis"));
          }
          TransferAndRenameGIPPFiles();
          TransferAndRenameISDFiles();
          const std::string &strProductFileName = BuildFileName(METADATA_CATEG, "", ".xml");
          generateProductMetadataFile(strMainFolderFullPath + "/" + strProductFileName);
          if (!m_bVectPrd) {
              bool bAgSuccess = ExecuteAgregateTiles(strMainFolderFullPath, this->GetParameterInt("aggregatescale"));
              std::cout << "Aggregating tiles " << (bAgSuccess ? "SUCCESS!" : "FAILED!") << std::endl;
              TransferMainProductPreviewFile();
          }
      }

      // Perform the consistency check of the product. If the main folder is renamed, then
      // the new path is returned
      strMainFolderFullPath = CheckProductConsistency(strMainFolderFullPath);

      if(HasValue("outprops")) {
            std::string outPropsFileName = this->GetParameterString("outprops");
            std::ofstream outPropsFile;
            try
            {
                outPropsFile.open(outPropsFileName.c_str(), std::ofstream::out);
                outPropsFile << strMainFolderFullPath << std::endl;
                outPropsFile.close();
            }
            catch(...)
            {
                itkGenericExceptionMacro(<< "Could not open file " << outPropsFileName);
            }
      }

      // finally, delete the lock file
      deleteMainFolderLockFile(lockFileName);
  }

  void CreateAndFillTile(tileInfo &tileInfoEl, const std::string &strMainFolderFullPath)
  {
      bool bResult;
      std::string strTileName = BuildTileName(tileInfoEl.strTileID);
      tileInfoEl.strTilePath = strMainFolderFullPath + "/" + TILES_FOLDER_NAME + "/" +  strTileName;

      bResult = createsAllTileSubfolders(tileInfoEl.strTilePath);

      if(bResult)
      {
          if(TileHasRasters(tileInfoEl)) {
              tileInfoEl.strTileNameRoot = strTileName;
              generateTileMetadataFile(tileInfoEl);
              TransferRasterFiles(tileInfoEl);

              //create product metadata file
              TransferAndRenameQualityFiles(tileInfoEl);
              FillProductMetadataForATile(tileInfoEl);
          } else {
              try
              {
                    boost::filesystem::remove_all(strMainFolderFullPath + "/" + TILES_FOLDER_NAME + "/" +  strTileName);
              } catch(boost::filesystem::filesystem_error const & e) {
                    otbAppLogWARNING("Error removing invalid tile folder "
                                     <<  strMainFolderFullPath + "/" + TILES_FOLDER_NAME + "/" +  strTileName
                                     << "Error was: " << e.what());
              }
          }
      }
 }

  bool TileHasRasters(const tileInfo &tileInfoEl) {
      for (const auto &rasterFileEl : m_rasterInfoList) {
          if(tileInfoEl.strTileID == rasterFileEl.strTileID) {
              if (boost::filesystem::exists(rasterFileEl.strRasterFileName)) {
                  return true;
              }
          }
      }
      return false;
  }

  void UnpackRastersList(std::vector<std::string> &rastersList, rasterTypes rasterType, bool bIsQiData, bool bQiDataIsDiscrete = true)
  {
      std::string strTileID;
      rasterInfo rasterInfoEl;
      rasterInfoEl.bIsQiData = bIsQiData;
      rasterInfoEl.bQiDataIsDiscrete = bQiDataIsDiscrete;
      rasterInfoEl.bNeedsPreview = false;

      // get the number of tiles elements in the rasters list (including duplicates)
      //std::string strTileID = UnpackTiles(rastersList, allTilesCnt);
      int allTilesCnt = CountTiles(rastersList);
      // second extract the rasters
      int curRaster = 0;
      bool bAllRastersHaveDate = ((rastersList.size()-allTilesCnt) == m_acquisitionDatesList.size());
      for (const auto &rasterFileEl : rastersList) {
          if(rasterFileEl.compare(0, 5, "TILE_") == 0)
          {
              //if is TILE separator, read tileID
              strTileID = rasterFileEl.substr(5, rasterFileEl.length() - 5);
              if(!IsTilePresent(strTileID))
              {
                tileInfo tileInfoEl;
                tileInfoEl.strTileID = strTileID;
                m_tileIDList.emplace_back(tileInfoEl);
              }
          } else  {
              rasterInfoEl.iRasterType = rasterType;
              rasterInfoEl.strRasterFileName = rasterFileEl;
              rasterInfoEl.strTileID = strTileID;
              rasterInfoEl.bNeedsPreview = IsRasterNeedsPreview(rasterType);
              // update the date
              if(bAllRastersHaveDate) {
                  if(LevelHasAcquisitionTime()) {
                      rasterInfoEl.rasterTimePeriod = m_acquisitionDatesList[curRaster];
                  } else {
                      rasterInfoEl.rasterTimePeriod = m_strTimePeriod;
                  }
              } else {
                  rasterInfoEl.rasterTimePeriod = m_strTimePeriod;
              }
              m_rasterInfoList.emplace_back(rasterInfoEl);
              curRaster++;
          }
      }
  }

  int CountTiles(std::vector<std::string> &filesList) {
      int nTotalFoundTiles = 0;
      for (const auto &file : filesList) {
          if(file.compare(0, 5, "TILE_") == 0)
              nTotalFoundTiles++;
      }
      return nTotalFoundTiles;
  }

  void UnpackQualityFlagsList(std::vector<std::string> &filesList) {
      qualityInfo qualityInfoEl;
      std::string strTileID;
      std::string strRegion;
      for (const auto &fileEl : filesList) {
          if(fileEl.compare(0, 5, "TILE_") == 0)
          {
              //if is TILE separator, read tileID
              strTileID = fileEl.substr(5, fileEl.length() - 5);
              if(!IsTilePresent(fileEl.substr(5, fileEl.length() - 5)))
              {
                tileInfo tileInfoEl;
                tileInfoEl.strTileID = strTileID;
                m_tileIDList.emplace_back(tileInfoEl);
              }
          } else if (fileEl.compare(0, 7, "REGION_") == 0) {
              strRegion = fileEl.substr(7, fileEl.length() - 7);
          } else  {
              qualityInfoEl.strFileName = fileEl;
              qualityInfoEl.strTileID = strTileID;
              qualityInfoEl.strRegion = strRegion;
              m_qualityList.emplace_back(qualityInfoEl);
          }
      }
  }

  bool IsTilePresent(const std::string &strTileID) {
      for (const auto &tileIDEl : m_tileIDList) {
          if(tileIDEl.strTileID == strTileID)
          {
              return true;
          }
      }
      return false;
  }

  // Get current date/time, format is YYYYMMDDThhmmss
  const std::string currentDateTimeFormattted(const std::string &strFormat) {
      char       buf[80];
      std::time_t now = std::time(NULL);
      std::strftime(buf, sizeof(buf), strFormat.c_str(), std::localtime(&now));
      return buf;
  }

  //creates a directory given by full path 'path'
  bool mkPath(const std::string &path)
  {
      boost::system::error_code ec;
      boost::filesystem::create_directories(path, ec);
      return !ec;
  }

  bool createsAllFolders(const std::string &strMainFolderPath)
  {
      bool bResult = true;
      /* create destination product folder */
      if(mkPath(strMainFolderPath) == false)
      {
          //fail to create main destination product folder
          bResult = false;
          itkExceptionMacro("Fail to create destination directory " << strMainFolderPath);
      }

      /* create TILES or VECTOR_DATA subfolder */
      const std::string &folderName = m_bVectPrd ? VECTOR_FOLDER_NAME : TILES_FOLDER_NAME;
      if(mkPath(strMainFolderPath + "/" + folderName) == false)
      {
           //fail to create TILES subfolder
          bResult = false;
          itkExceptionMacro("Fail to create TILES subfolder in destination directory!");
      }

      /*create AUX_DATA subfolder */
      if(mkPath(strMainFolderPath + "/" + AUX_DATA_FOLDER_NAME) == false)
      {
          bResult = false;
          itkExceptionMacro("Fail to create AUX_DATA subfolder in destination directory!");
      }

      /*create LEGACY_DATA subfolder */
      if(mkPath(strMainFolderPath + "/" + LEGACY_DATA_FOLDER_NAME) == false)
      {
          bResult = false;
          //fail to create destination directory
          itkExceptionMacro("Fail to create LEGACY_DATA subfolder in destination directory!");
      }
      return bResult;
  }

  std::string createMainFolderLockFile(const std::string &strMainFolderPath) {
      // ensure that the root folder is created
      if(mkPath(m_strDestRoot) == false)
      {
          itkExceptionMacro("Fail to create destination root directory " << m_strDestRoot);
      }

      std::string lockFileName = strMainFolderPath + ".lock";
      std::ofstream f( lockFileName );
      if ( !f ) {
          // if the lock file cannot be created, we throw an error as probably the other files cannot be created too
          itkGenericExceptionMacro(<< "Error creating lock file for " << strMainFolderPath);
          return lockFileName;
      }
      f << "lock";

      return lockFileName;
  }

  bool deleteMainFolderLockFile(const std::string &lockFileName) {
    return boost::filesystem::remove(lockFileName);
  }


  std::string ReplaceString(std::string subject, const std::string& search,
                            const std::string& replace) {
      size_t pos = 0;
      while((pos = subject.find(search, pos)) != std::string::npos) {
           subject.replace(pos, search.length(), replace);
           pos += replace.length();
      }
      return subject;
  }

  std::string GetProductType()
  {
      if (m_strProductLevel.compare("L2A") == 0)
         return L2A_PRODUCT;
      if (m_strProductLevel == SEN2AGRI_L3A_PRD_LEVEL)
         return COMPOSITE_PRODUCT;
      if (m_strProductLevel == SEN2AGRI_L3B_PRD_LEVEL)
         return LAI_MONO_DATE_PRODUCT;
      if (m_strProductLevel == SEN2AGRI_L3C_PRD_LEVEL)
         return LAI_REPROC_PRODUCT;
      if (m_strProductLevel == SEN2AGRI_L3D_PRD_LEVEL)
         return LAI_FITTED_PRODUCT;
      if (m_strProductLevel == SEN2AGRI_L3E_PRD_LEVEL)
         return PHENO_NDVI_PRODUCT;
      if (m_strProductLevel == SEN2AGRI_L4A_PRD_LEVEL)
         return CROP_MASK;
      if (m_strProductLevel == SEN2AGRI_L4B_PRD_LEVEL)
         return CROP_TYPE;
      if (m_strProductLevel.size() > 0) {
          return m_strProductLevel;
      }
      return "Unknowkn product";
  }

  void FillBandList()
  {
      Band bandEl;
      if (m_strProductLevel == SEN2AGRI_L3A_PRD_LEVEL)
              //if is composite product, fill bands
      {

          for (const auto &compositeBandEl : CompositeBandList) {
              bandEl.Resolution = compositeBandEl.iSpatialResolution;
              bandEl.BandName = compositeBandEl.strBandName;
              m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);
           }
      }
  }

  Coord PointFromVector(const FloatVectorImageType::VectorType &v)
  {
      Coord index;
      index.x = v[0];
      index.y = v[1];
      return index;
  }

  template <typename TVector>
  OGRErr TransformPoint(const TVector &v,
                        OGRCoordinateTransformation *transform,
                        OGRRawPoint &ptOut)
  {
      OGRPoint pt(v[0], v[1], 0.0);

      auto err = pt.transform(transform);

      ptOut.x = pt.getX();
      ptOut.y = pt.getY();

      return err;
  }

  template <typename TImage>
  std::vector<OGRRawPoint> GetExtent(const TImage *image)
  {
      std::vector<OGRRawPoint> extent;

      auto sourceSRS = static_cast<OGRSpatialReference *>(
                           OSRNewSpatialReference(image->GetProjectionRef().c_str()));
      if (!sourceSRS) {
          return extent;
      }

      auto targetSRS = static_cast<OGRSpatialReference *>(OSRNewSpatialReference(SRS_WKT_WGS84));
      if (!targetSRS) {
          return extent;
      }

      auto transform = static_cast<OGRCoordinateTransformation *>(
          OCTNewCoordinateTransformation(sourceSRS, targetSRS));
      if (!transform) {
          return extent;
      }

      OGRRawPoint pt;
      auto ok = true;
      if (TransformPoint(image->GetUpperLeftCorner(), transform, pt) == OGRERR_NONE) {
          extent.emplace_back(pt);
      } else {
          ok = false;
      }
      if (ok &&
          TransformPoint(image->GetLowerLeftCorner(), transform, pt) == OGRERR_NONE) {
          extent.emplace_back(pt);
      } else {
          ok = false;
      }
      if (ok &&
          TransformPoint(image->GetLowerRightCorner(), transform, pt) == OGRERR_NONE) {
          extent.emplace_back(pt);
      } else {
          ok = false;
      }
      if (ok &&
          TransformPoint(image->GetUpperRightCorner(), transform, pt) == OGRERR_NONE) {
          extent.emplace_back(pt);
      } else {
          ok = false;
      }

      if (!ok) {
          extent.clear();
      }

      OCTDestroyCoordinateTransformation(transform);
      OSRDestroySpatialReference(targetSRS);
      OSRDestroySpatialReference(sourceSRS);

      return extent;
  }

  bool generateRgbFromLut(const std::string &rasterFullFilePath, const std::string &ourRasterFullFilePath,
                          const std::string &lutMap, bool bIsRgbImg, bool bIsRangeMapFile)
  {
      std::vector<const char *> args;
      args.emplace_back("ContinuousColorMapping");
      args.emplace_back("-progress");
      args.emplace_back("false");
      args.emplace_back("-in");
      args.emplace_back(rasterFullFilePath.c_str());
      args.emplace_back("-out");
      args.emplace_back(ourRasterFullFilePath.c_str());
      args.emplace_back("-map");
      args.emplace_back(lutMap.c_str());
      args.emplace_back("-rgbimg");
      std::string strIsRgbImg = std::to_string(bIsRgbImg);
      args.emplace_back(strIsRgbImg.c_str());

      args.emplace_back("-isrange");
      std::string strIsRangeMapFile = std::to_string(bIsRangeMapFile);
      args.emplace_back(strIsRangeMapFile.c_str());

      return ExecuteExternalProgram("otbApplicationLauncherCommandLine", args);
  }

  bool generateQuicklook(const std::string &rasterFullFilePath, const std::vector<std::string> &channels,const std::string &jpegFullFilePath)
  {
      std::vector<const char *> args;
      args.emplace_back("Quicklook");
      args.emplace_back("-progress");
      args.emplace_back("false");
      args.emplace_back("-in");
      args.emplace_back(rasterFullFilePath.c_str());
      args.emplace_back("-out");
      args.emplace_back(jpegFullFilePath.c_str());
      args.emplace_back("uint8");
      args.emplace_back("-sr");
      args.emplace_back("10");
      args.emplace_back("-cl");
      for (const auto &channel : channels) {
          args.emplace_back(channel.c_str());
      }
      bool bRet = ExecuteExternalProgram("otbApplicationLauncherCommandLine", args);

      //remove  file with extension jpg.aux.xml generated after preview obtained
      std::string strFileToBeRemoved = jpegFullFilePath + ".aux.xml";
      //std::cout << "Remove file: " <<  strFileToBeRemoved<< std::endl;
      remove(strFileToBeRemoved.c_str());

      return bRet;
  }

  void generateTileMetadataFile(tileInfo &tileInfoEl)
  {
      TileSize tileSizeEl;
      TileGeoposition tileGeoposition;
      int iResolution;
      bool bResolutionExistingAlready = false;
      bool bGeoPositionExistingAlready = false;
      //bool bPreview = !m_previewList.empty();

      auto writer = itk::TileMetadataWriter::New();

      std::string strTile = BuildFileName(METADATA_CATEG, tileInfoEl.strTileID, "");
      tileInfoEl.tileMetadata.TileID = strTile;
      tileInfoEl.tileMetadata.ProductLevel = "Level-"  + m_strProductLevel;

      for (rasterInfo &rasterFileEl : m_rasterInfoList) {
          // we should not throw an exception here, as we might have just some rasters that do not exist
          if(!boost::filesystem::exists(rasterFileEl.strRasterFileName)) {
              continue;
          }

          if(rasterFileEl.bIsQiData) {
              if(rasterFileEl.strRasterFileName.find(".tif") > 0 || rasterFileEl.strRasterFileName.find(".TIF") > 0) {
                  auto imageReader = ImageFileReader<FloatVectorImageType>::New();
                  imageReader->SetFileName(rasterFileEl.strRasterFileName);
                  imageReader->UpdateOutputInformation();
                  FloatVectorImageType::Pointer output = imageReader->GetOutput();

                  rasterFileEl.nResolution = output->GetSpacing()[0];
              }
          } else if((rasterFileEl.strTileID == tileInfoEl.strTileID)) {
              //std::cout << "ImageFileReader =" << rasterFileEl.strRasterFileName << std::endl;

              auto imageReader = ImageFileReader<FloatVectorImageType>::New();
              imageReader->SetFileName(rasterFileEl.strRasterFileName);
              imageReader->UpdateOutputInformation();
              FloatVectorImageType::Pointer output = imageReader->GetOutput();

              iResolution = output->GetSpacing()[0];

              if(/*(!bPreview) && */IsPreviewNeeded(rasterFileEl, iResolution))
              {
                  bool bFound = false;
                  for(size_t i = 0; i < m_previewList.size(); i++) {
                      if(m_previewList[i].strPreviewFileName == rasterFileEl.strRasterFileName &&
                              m_previewList[i].strTileID == tileInfoEl.strTileID) {
                          bFound = true;
                          break;
                      }
                  }
                  if(!bFound) {
                      previewInfo previewInfoEl;
                      previewInfoEl.strPreviewFileName = rasterFileEl.strRasterFileName;
                      previewInfoEl.strTileID = tileInfoEl.strTileID;
                      m_previewList.emplace_back(previewInfoEl);
                      //bPreview = true;
                  }
              }

              if(rasterFileEl.iRasterType != COMPOSITE_REFLECTANCE_RASTER)
              {
                  //bands no = output->GetNumberOfComponentsPerPixel()
                  Band bandEl;
                  bool bFound;
                  for(size_t j = 1; j <= output->GetNumberOfComponentsPerPixel(); j++)
                  {
                      bFound = false;
                      bandEl.Resolution = iResolution;
                      bandEl.BandName = "B" + std::to_string(j);

                      //search if the band already exist
                      for (const auto &band : m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.BandList)
                      {
                          if((band.BandName  == bandEl.BandName) && (band.Resolution == bandEl.Resolution))
                          {
                              bFound = true;
                              break;
                          }
                      }

                      if(!bFound){
                          //element not found so add it
                           m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);
                      }
                  }

              }

              rasterFileEl.nResolution = iResolution;
              //search the current resolution to seed if is already added in tileEl
              for (const auto &tileEl : tileInfoEl.tileMetadata.TileGeometricInfo.TileSizeList) {
                  if(tileEl.resolution == iResolution)
                  {
                      bResolutionExistingAlready = true;
                      break;
                  }
               }

              if(!bResolutionExistingAlready){
                  tileSizeEl.resolution = iResolution;
                  tileSizeEl.nrows = output->GetLargestPossibleRegion().GetSize()[1];
                  tileSizeEl.ncols = output->GetLargestPossibleRegion().GetSize()[0];
                  tileInfoEl.tileMetadata.TileGeometricInfo.TileSizeList.emplace_back(tileSizeEl);
              }

              tileGeoposition.resolution = iResolution;

              for (const auto &geoPosEl : tileInfoEl.tileMetadata.TileGeometricInfo.TileGeopositionList) {
                  if(geoPosEl.resolution == iResolution)
                  {
                      bGeoPositionExistingAlready = true;
                      break;
                  }
               }

              if(!bGeoPositionExistingAlready)
              {
                  Coord coord = PointFromVector(output->GetUpperLeftCorner());
                  tileGeoposition.ulx = coord.x;
                  tileGeoposition.uly = coord.y;
                  tileGeoposition.xdim = output->GetSpacing()[0];
                  tileGeoposition.ydim = output->GetSpacing()[1];
                  tileInfoEl.tileMetadata.TileGeometricInfo.TileGeopositionList.emplace_back(tileGeoposition);
              }

              geoProductInfo geoProductInfoEl;

              if (auto oSRS = static_cast<OGRSpatialReference *>(OSRNewSpatialReference(output->GetProjectionRef().c_str()))) {

                  geoProductInfoEl.CoordReferenceSystem.HorizCSName = oSRS->GetAttrValue("PROJCS");
                  geoProductInfoEl.CoordReferenceSystem.HorizCSCode = std::string(oSRS->GetAuthorityName("PROJCS")) + ':' + oSRS->GetAuthorityCode("PROJCS");
                  std::cout << "HorizCSName: HorizCSCode" << geoProductInfoEl.CoordReferenceSystem.HorizCSName << ": " << geoProductInfoEl.CoordReferenceSystem.HorizCSCode << std::endl;


                  tileInfoEl.tileMetadata.TileGeometricInfo.HorizontalCSName = geoProductInfoEl.CoordReferenceSystem.HorizCSName;
                  tileInfoEl.tileMetadata.TileGeometricInfo.HorizontalCSCode = geoProductInfoEl.CoordReferenceSystem.HorizCSCode;

                  OSRDestroySpatialReference(oSRS);
              }


              geoProductInfoEl.iResolution = iResolution;
              geoProductInfoEl.rasterType = rasterFileEl.iRasterType;

              const auto &extent = GetExtent(output.GetPointer());

              geoProductInfoEl.AreaOfInterest.LowerCornerLon = extent[1].x;
              geoProductInfoEl.AreaOfInterest.LowerCornerLat = extent[1].y;
              geoProductInfoEl.AreaOfInterest.UpperCornerLon = extent[3].x;
              geoProductInfoEl.AreaOfInterest.UpperCornerLat = extent[3].y;
              m_geoProductInfo.emplace_back(geoProductInfoEl);

          }
      }

      ComputeNewNameOfRasterFiles(tileInfoEl);

      tileInfoEl.tileMetadata.TileThematicInfo = "";
      tileInfoEl.tileMetadata.TileImageContentQI.NoDataPixelPercentange = "";
      tileInfoEl.tileMetadata.TileImageContentQI.SaturatedDefectivePixelPercentange = "";
      tileInfoEl.tileMetadata.TileImageContentQI.CloudShadowPercentange = "";
      tileInfoEl.tileMetadata.TileImageContentQI.VegetationPercentange = "";
      tileInfoEl.tileMetadata.TileImageContentQI.WaterPercentange = "";
      tileInfoEl.tileMetadata.TileImageContentQI.LowProbaCloudsPercentange = "";
      tileInfoEl.tileMetadata.TileImageContentQI.MediumProbaCloudsPercentange = "";
      tileInfoEl.tileMetadata.TileImageContentQI.HighProbaCloudsPercentange = "";
      tileInfoEl.tileMetadata.TileImageContentQI.ThinCirrusPercentange = "";
      tileInfoEl.tileMetadata.TileImageContentQI.SnowIcePercentange = "";

      TileMask tileMask;
      for (const auto &rasterFileEl : m_rasterInfoList) {
         if((rasterFileEl.strTileID == tileInfoEl.strTileID) && rasterFileEl.bIsQiData)
         {
              tileMask.MaskType = "";// ??? TODO
              tileMask.BandId = 0;
              tileMask.Geometry = "FULL_RESOLUTION";
              tileMask.MaskFileName = rasterFileEl.strNewRasterFileName;
              tileInfoEl.tileMetadata.TileMasksList.emplace_back(tileMask);
          }
      }

      tileInfoEl.strTileNameWithoutExt = tileInfoEl.strTilePath + "/" + strTile;
      writer->WriteTileMetadata(tileInfoEl.tileMetadata, tileInfoEl.strTilePath + "/" + strTile + ".xml");
  }

  void generateProductMetadataFile(const std::string &strProductMetadataFilePath)
  {
      auto writer = itk::ProductMetadataWriter::New();

      m_productMetadata.GeneralInfo.ProductInfo.ProductURI = "";
      m_productMetadata.GeneralInfo.ProductInfo.ProcessingLevel = m_strProductLevel;

      m_productMetadata.GeneralInfo.ProductInfo.ProductType = GetProductType();
      m_productMetadata.GeneralInfo.ProductInfo.ProcessingBaseline = m_strBaseline;
      m_productMetadata.GeneralInfo.ProductInfo.GenerationTime = currentDateTimeFormattted("%Y-%m-%dT%H:%M:%S");/*"2015-07-04T10:12:29.000413Z";*/

      m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.PreviewImage = !m_previewList.empty();

      //build product preview file name
      m_productMetadata.GeneralInfo.ProductInfo.PreviewImageURL = BuildFileName(QUICK_L0OK_IMG_CATEG, "", JPEG_EXTENSION);

      geoProductInfo geoPosEl;

      if(!m_geoProductInfo.empty())
      {
          geoPosEl = m_geoProductInfo[0];

          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLon = geoPosEl.AreaOfInterest.LowerCornerLon;
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLat = geoPosEl.AreaOfInterest.LowerCornerLat;
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLon = geoPosEl.AreaOfInterest.UpperCornerLon;
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLat = geoPosEl.AreaOfInterest.UpperCornerLat;

          for (size_t j = 1; j < m_geoProductInfo.size(); j++) {
              geoPosEl = m_geoProductInfo[j];
              if(geoPosEl.AreaOfInterest.LowerCornerLon < m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLon){
                  m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLon = geoPosEl.AreaOfInterest.LowerCornerLon;
              }
              if(geoPosEl.AreaOfInterest.LowerCornerLat < m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLat){
                  m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLat = geoPosEl.AreaOfInterest.LowerCornerLat;
              }
              if(geoPosEl.AreaOfInterest.UpperCornerLon > m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLon){
                  m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLon = geoPosEl.AreaOfInterest.UpperCornerLon;
              }
              if(geoPosEl.AreaOfInterest.UpperCornerLat > m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLat){
                  m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLat = geoPosEl.AreaOfInterest.UpperCornerLat;
              }
          }

      }

      m_productMetadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLon);
      m_productMetadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLat);

      m_productMetadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLon);
      m_productMetadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLat);


      m_productMetadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLon);
      m_productMetadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLat);

      m_productMetadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLon);
      m_productMetadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLat);

      m_productMetadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLon);
      m_productMetadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLat);


      //if(m_tileIDList.size() > 1)
      if (!m_geoProductInfo.empty())
      {
          //for multiple tiles if there are differences between HorizCSName values and HorizCSCode values, add GEO_TABLES = "EPSG" and HorizCSType = "GEOGRAPHIC"
          //check for differeces
          geoPosEl = m_geoProductInfo[0];
          std::string strHorizCSName(geoPosEl.CoordReferenceSystem.HorizCSName);
          std::string strHorizCSCode(geoPosEl.CoordReferenceSystem.HorizCSCode);

          std::cout << "strHorizCSName " << strHorizCSName << std::endl;
          std::cout << "strHorizCSCode " << strHorizCSCode << std::endl;

          bool bIsDifferent = false;
          std::cout << "m_geoProductInfo.size() =" << m_geoProductInfo.size() << std::endl;

          for (size_t i = 1; i < m_geoProductInfo.size(); i++) {

              geoPosEl = m_geoProductInfo[i];
              if(strHorizCSName != geoPosEl.CoordReferenceSystem.HorizCSName)
              {
                  m_productMetadata.GeometricInfo.CoordReferenceSystem.HorizCSType = GENERIC_CS_TYPE;
                  bIsDifferent = true;
                  std::cout << "strHorizCSType " << GENERIC_CS_TYPE << std::endl;
              }
              if(strHorizCSCode != geoPosEl.CoordReferenceSystem.HorizCSCode)
              {
                  m_productMetadata.GeometricInfo.CoordReferenceSystem.GeoTables = GENERIC_GEO_TABLES;
                  m_productMetadata.GeometricInfo.CoordReferenceSystem.nGeoTablesVersion = 1;
                  std::cout << "GEO_TABLES = " << GENERIC_CS_TYPE << std::endl;
                  bIsDifferent = true;
              }
          }
          if(!bIsDifferent)
          {
              m_productMetadata.GeometricInfo.CoordReferenceSystem.HorizCSName = strHorizCSName;
              m_productMetadata.GeometricInfo.CoordReferenceSystem.HorizCSCode = strHorizCSCode;
          }

      }


      FillBandList();

      m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.MetadataLevel = "SuperBrief";
      m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AuxListContent.ProductLevel = "Level-"  + m_strProductLevel;


      if(m_GIPPList.empty())
      {
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AuxListContent.GIPP = "NO";
      }
      else
      {
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AuxListContent.GIPP = "YES";
      }
      if(m_ISDList.empty())
      {
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AuxListContent.ISD = "NO";
      }
      else
      {
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AuxListContent.ISD = "YES";
      }
      m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.ProductFormat = "SAFE";
      m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AggregationFlag = true;


      m_productMetadata.GeneralInfo.ProductImageCharacteristics.ImageDisplayOrder.RedChannel = 0;
      m_productMetadata.GeneralInfo.ProductImageCharacteristics.ImageDisplayOrder.GreenChannel = 0;
      m_productMetadata.GeneralInfo.ProductImageCharacteristics.ImageDisplayOrder.BlueChannel = 0;

      if ((m_strProductLevel == SEN2AGRI_L2A_PRD_LEVEL) ||
          (m_strProductLevel == SEN2AGRI_L3A_PRD_LEVEL))
      {
          std::cout << "Red, green and blue" << std::endl;
          m_productMetadata.GeneralInfo.ProductImageCharacteristics.ImageDisplayOrder.RedChannel = 3;
          m_productMetadata.GeneralInfo.ProductImageCharacteristics.ImageDisplayOrder.GreenChannel = 2;
          m_productMetadata.GeneralInfo.ProductImageCharacteristics.ImageDisplayOrder.BlueChannel = 1;

      }

      m_productMetadata.GeneralInfo.ProductImageCharacteristics.QuantificationUnit = "none";
      m_productMetadata.GeneralInfo.ProductImageCharacteristics.QuantificationValue = 1000;

      /* GIPP infos are completed in TransferAndRenameGIPPFiles*/

      m_productMetadata.QualityIndicatorsInfo.TechnicalQualityAssessment.DegratedANCDataPercentage = "";
      m_productMetadata.QualityIndicatorsInfo.TechnicalQualityAssessment.DegratedMSIDataPercentage = "";

      m_productMetadata.QualityIndicatorsInfo.QualityControlChecks.QualityInspections.FormatCorectnessFlag = "";
      m_productMetadata.QualityIndicatorsInfo.QualityControlChecks.QualityInspections.GeometricQualityFlag = "";
      m_productMetadata.QualityIndicatorsInfo.QualityControlChecks.QualityInspections.GeneralQualityFlag = "";
      m_productMetadata.QualityIndicatorsInfo.QualityControlChecks.QualityInspections.SensorQualityFlag = "";
      m_productMetadata.QualityIndicatorsInfo.QualityControlChecks.QualityInspections.RadiometricQualityFlag = "";


      m_productMetadata.GeometricInfo.ProductFootprint.RatserCSType = "POINT";
      m_productMetadata.GeometricInfo.ProductFootprint.PixelOrigin = 1;

      //???? TODO
      SpecialValues specialValue;
      specialValue.SpecialValueIndex = NO_DATA_VALUE;
      specialValue.SpecialValueText = "NODATA";
      m_productMetadata.GeneralInfo.ProductImageCharacteristics.SpecialValuesList.push_back(specialValue);

      if (m_strProductLevel == "L4A")
      {
          specialValue.SpecialValueIndex = "0";
          specialValue.SpecialValueText = "NO CROP";
          m_productMetadata.GeneralInfo.ProductImageCharacteristics.SpecialValuesList.push_back(specialValue);

          specialValue.SpecialValueIndex = "1";
          specialValue.SpecialValueText = "CROP";
          m_productMetadata.GeneralInfo.ProductImageCharacteristics.SpecialValuesList.push_back(specialValue);
      }

      m_productMetadata.QualityIndicatorsInfo.CloudCoverage = "";

      writer->WriteProductMetadata(m_productMetadata, strProductMetadataFilePath);
  }

  void FillProductMetadataForATile(const tileInfo &tileInfoEl)
  {

      //fill tiles list
      Granule granuleEl;

      //for the moment is only one tile
      granuleEl.GranuleIdentifier = tileInfoEl.strTileNameRoot;
      granuleEl.ImageFormat = IMAGE_FORMAT;
      //fill the TIFF files for current tile
      for (const auto &rasterFileEl : m_rasterInfoList) {
          if((rasterFileEl.strTileID == tileInfoEl.strTileID) && !rasterFileEl.bIsQiData)
          {
            granuleEl.ImageIDList.emplace_back(rasterFileEl.strNewRasterFileName);

          }
      }
      m_productMetadata.GeneralInfo.ProductInfo.ProductOrganisation.emplace_back(granuleEl);

      //no reports for now
      GranuleReport granuleReport;
      granuleReport.GranuleReportId = tileInfoEl.strTileNameRoot;
      granuleReport.GranuleReportFileName = "";
      m_productMetadata.QualityIndicatorsInfo.QualityControlChecks.FailedInspections.emplace_back(granuleReport);

  }

  bool createsAllTileSubfolders(const std::string &strTileFullPath)
  {
      bool bResult = true;

      /* create tile folder */
      if(mkPath(strTileFullPath) == false)
      {
          //fail to create tile folder
          bResult = false;
          itkExceptionMacro("Fail to create tile directory " + strTileFullPath);
      }

      /* create IMG_DATA subfolder */
      if(mkPath(strTileFullPath + "/" + IMG_DATA_FOLDER_NAME) == false)
      {
           //fail to create IMG_DATA subfolder
          bResult = false;
          itkExceptionMacro("Fail to create IMG_DATA subfolder in tile directory " + strTileFullPath);
      }

      /* create QI_DATA subfolder */
      if(mkPath(strTileFullPath + "/" + QI_DATA_FOLDER_NAME) == false)
      {
           //fail to create QI_DATA subfolder
          bResult = false;
          itkExceptionMacro("Fail to create QI_DATA subfolder in tile directory " + strTileFullPath);
      }

      return bResult;
  }

  bool IsRasterNeedsPreview(rasterTypes rasterType) {
      switch(rasterType) {
          case COMPOSITE_REFLECTANCE_RASTER:
              // this does not needs a preview because it is actually the RGB image that is previewed
              return false;
          case LAI_MONO_DATE_RASTER:
              return true;
          case LAI_MONO_DATE_ERR_RASTER:
              return true;
          case FAPAR_MONO_DATE_RASTER:
              return true;
          case FCOVER_MONO_DATE_RASTER:
              return true;
          case LAI_REPR_RASTER:
              return true;
          case LAI_FIT_RASTER:
              return true;
          case PHENO_RASTER:
               return true;
          case CROP_TYPE_RASTER:
              return true;
          case CROP_TYPE_RAW_RASTER:
              return false;
          case CROP_MASK_RASTER:
             return true;
          default:
             return false;
      }
      return false;
  }

  void ComputeNewNameOfRasterFiles(const tileInfo &tileInfoEl)
  {
      std::string rasterCateg;

      for (rasterInfo &rasterFileEl : m_rasterInfoList) {
          if(rasterFileEl.strTileID != tileInfoEl.strTileID) {
              continue;
          }
          bool bAddResolutionToSuffix = false;
          int expectedBandsNo = 1;
          std::string suffix = TIF_EXTENSION;
          switch(rasterFileEl.iRasterType)
              {
                case COMPOSITE_REFLECTANCE_RASTER:
                    rasterCateg = REFLECTANCE_SUFFIX;
                    bAddResolutionToSuffix = true;
                    expectedBandsNo = 4;
                    break;
                case COMPOSITE_WEIGHTS_RASTER:
                    rasterCateg = WEIGHTS_SUFFIX;
                    bAddResolutionToSuffix = true;
                    expectedBandsNo = 4;
                    break;
                case LAI_NDVI_RASTER:
                    rasterCateg = LAI_NDVI_SUFFIX;
                  break;
                case LAI_MONO_DATE_RASTER:
                    rasterCateg = LAI_MDATE_SUFFIX;
                    break;
                case LAI_MONO_DATE_ERR_RASTER:
                    rasterCateg = LAI_MDATE_ERR_SUFFIX;
                  break;
                case FAPAR_MONO_DATE_RASTER:
                    rasterCateg = FAPAR_MDATE_SUFFIX;
                    break;
                case FCOVER_MONO_DATE_RASTER:
                    rasterCateg = FCOVER_MDATE_SUFFIX;
                    break;
                case LAI_REPR_RASTER:
                    rasterCateg = LAI_REPR_SUFFIX;
                   break;
                case LAI_FIT_RASTER:
                    rasterCateg = LAI_FIT_SUFFIX;
                   break;
                case PHENO_RASTER:
                     rasterCateg = PHENO_SUFFIX;
                     expectedBandsNo = 4;
                     break;
                case CROP_TYPE_RASTER:
                    rasterCateg = CROP_TYPE_IMG_SUFFIX;
                    break;
                case CROP_TYPE_RAW_RASTER:
                    rasterCateg = CROP_TYPE_RAW_IMG_SUFFIX;
                    break;
                case CROP_MASK_RASTER:
                    rasterCateg = CROP_MASK_IMG_SUFFIX;
                   break;
                case RAW_CROP_MASK_RASTER:
                    rasterCateg = CROP_MASK_RAW_IMG_SUFFIX;
                    break;
                case COMPOSITE_DATES_MASK:
                    rasterCateg = COMPOSITE_DATES_SUFFIX;
                    bAddResolutionToSuffix = true;
                    break;
                case COMPOSITE_FLAGS_MASK:
                    rasterCateg = COMPOSITE_FLAGS_SUFFIX;
                    bAddResolutionToSuffix = true;
                    break;
                case LAI_MONO_DATE_STATUS_FLAGS:
                    rasterCateg = LAI_MONO_DATE_FLAGS_SUFFIX;
                    break;

                case LAI_MONO_DATE_IN_DOMAIN_FLAGS:
                    rasterCateg = LAI_IN_DOMAIN_FLAGS_SUFFIX;
                    break;
                case LAI_MONO_DATE_DOMAIN_FLAGS:
                    rasterCateg = LAI_DOMAIN_FLAGS_SUFFIX;
                    break;
                case FAPAR_DOMAIN_FLAGS:
                    rasterCateg = FAPAR_DOMAIN_FLAGS_SUFFIX;
                    break;
                case FCOVER_DOMAIN_FLAGS:
                    rasterCateg = FCOVER_DOMAIN_FLAGS_SUFFIX;
                    break;

                case LAI_REPROC_FLAGS:
                    rasterCateg = LAI_REPROC_FLAGS_SUFFIX;
                    break;
                case LAI_FITTED_FLAGS:
                    rasterCateg = LAI_FITTED_FLAGS_SUFFIX;
                    break;
                case PHENO_FLAGS:
                    rasterCateg = PHENO_FLAGS_SUFFIX;
                    break;
                case CROP_MASK_FLAGS:
                    rasterCateg = CROP_MASK_FLAGS_SUFFIX;
                    break;
                case CROP_TYPE_FLAGS:
                    rasterCateg = CROP_TYPE_FLAGS_SUFFIX;
                    break;
              }
              if(bAddResolutionToSuffix) {
                  suffix = "_" + std::to_string(rasterFileEl.nResolution) + "M" + suffix;
              }

              rasterFileEl.strNewRasterFileName = BuildFileName(rasterCateg, tileInfoEl.strTileID, suffix, rasterFileEl.rasterTimePeriod);
              rasterFileEl.nRasterExpectedBandsNo = expectedBandsNo;
      }
  }

  void TransferRasterFiles(const tileInfo &tileInfoEl)
  {

      std::string strImgDataPath;

      for (auto &rasterFileEl : m_rasterInfoList) {
          if(tileInfoEl.strTileID == rasterFileEl.strTileID)
          {
              if(rasterFileEl.bIsQiData) {
                  strImgDataPath = tileInfoEl.strTilePath + "/" + QI_DATA_FOLDER_NAME;
              } else {
                  strImgDataPath = tileInfoEl.strTilePath + "/" + IMG_DATA_FOLDER_NAME;
              }
              rasterFileEl.strNewRasterFullPath = strImgDataPath + "/" + rasterFileEl.strNewRasterFileName;

              // call script for COG/COMPRESS, if needed
              ExecuteGdalTranslateOps(rasterFileEl.strRasterFileName, (rasterFileEl.bIsQiData && rasterFileEl.bQiDataIsDiscrete));

              CopyFile(rasterFileEl.strNewRasterFullPath, rasterFileEl.strRasterFileName);
          }
        }
   }


  void TransferAndRenameLUTFile(const std::string &lut)
  {
      boost::filesystem::path p(lut);
      std::string outputFilename = BuildFileName(LUT_CATEG, "", p.extension().string());

      CopyFile(m_strDestRoot + "/" + m_strProductDirectoryName + "/" + AUX_DATA_FOLDER_NAME + "/" + outputFilename, lut);
  }

  void TransferAndRenameGIPPFiles()
  {
      std::string strNewGIPPFileName;
      GIPPInfo GIPPEl;


      for (const auto &gippFileEl : m_GIPPList) {

          strNewGIPPFileName = ReplaceString(strNewGIPPFileName, MAIN_FOLDER_CATEG, PARAMETER_CATEG);
          boost::filesystem::path p(gippFileEl);
          if(m_GIPPList.size() > 1)
          {
              strNewGIPPFileName = BuildFileName(PARAMETER_CATEG, "", "_" + p.stem().string() + p.extension().string());
          }
          else
          {
              strNewGIPPFileName = BuildFileName(PARAMETER_CATEG, "", p.extension().string());
          }

          //std::cout << "strNewGIPPFileName = " << strNewGIPPFileName << std::endl;

          GIPPEl.GIPPFileName = strNewGIPPFileName;
          GIPPEl.GIPPType = "";
          GIPPEl.GIPPVersion = GIPP_VERSION;
          m_productMetadata.AuxiliaryDataInfo.GIPPList.emplace_back(GIPPEl);

           //std::cout << "destGIPP = " << m_strDestRoot + "/" + m_strProductDirectoryName + "/" + AUX_DATA_FOLDER_NAME + "/" + strNewGIPPFileName << std::endl;

          //gipp files are copied to AUX_DATA
          CopyFile(m_strDestRoot + "/" + m_strProductDirectoryName + "/" + AUX_DATA_FOLDER_NAME + "/" + strNewGIPPFileName, gippFileEl);
    }
  }

  void TransferAndRenameISDFiles()
  {
      std::string strNewISDFileName;
      ISDInfo ISDEl;


      for (const auto &isdFileEl : m_ISDList) {

          strNewISDFileName = ReplaceString(strNewISDFileName, MAIN_FOLDER_CATEG, INSITU_CATEG);
          boost::filesystem::path p(isdFileEl);
          if(m_GIPPList.size() > 1)
          {
              strNewISDFileName = BuildFileName(INSITU_CATEG, "", "_" + p.stem().string() + p.extension().string());
          }
          else
          {
              strNewISDFileName = BuildFileName(INSITU_CATEG, "", p.extension().string());
          }

          //std::cout << "strNewGIPPFileName = " << strNewGIPPFileName << std::endl;

          ISDEl.ISDFileName = strNewISDFileName;
          m_productMetadata.AuxiliaryDataInfo.ISDList.emplace_back(ISDEl);

           //std::cout << "destGIPP = " << m_strDestRoot + "/" + m_strProductDirectoryName + "/" + AUX_DATA_FOLDER_NAME + "/" + strNewGIPPFileName << std::endl;

          //files are copied to AUX_DATA
          CopyFile(m_strDestRoot + "/" + m_strProductDirectoryName + "/" + AUX_DATA_FOLDER_NAME + "/" + strNewISDFileName, isdFileEl);
    }
  }
  void TransferAndRenameQualityFiles(const tileInfo &tileInfoEl)
  {
      std::string strNewQualityFileName;

      for (const auto &qualityFileEl : m_qualityList) {
          if(qualityFileEl.strTileID.length() != 0) {
            if(tileInfoEl.strTileID == qualityFileEl.strTileID)
            {
                std::string strImgDataPath = tileInfoEl.strTilePath + "/" + QI_DATA_FOLDER_NAME;
                boost::filesystem::path p(qualityFileEl.strFileName);
                strNewQualityFileName = BuildFileName(QUALITY_CATEG, tileInfoEl.strTileID, p.extension().string());
                CopyFile(strImgDataPath + "/" + strNewQualityFileName, qualityFileEl.strFileName);
            }
          } else {
              boost::filesystem::path p(qualityFileEl.strFileName);
              strNewQualityFileName = BuildFileName(QUALITY_CATEG, "", p.extension().string(), "", "", "", qualityFileEl.strRegion);

               //quality files are copied to tileDirectory/QI_DATA
              CopyFile(m_strDestRoot + "/" + m_strProductDirectoryName +
                       "/" + AUX_DATA_FOLDER_NAME + "/" + strNewQualityFileName, qualityFileEl.strFileName);
          }
    }
  }
  void TransferPreviewFiles()
  {

      std::string strTilePreviewFullPath;
      int iChannelNo = 1;
      std::vector<std::string> strChannelsList;
      // check if we should use the LUT table for LAI
      bool bUseLut = false;
      bool bIsRgbImg = false;
      bool bIsRangeMapFile = true;

      if ((m_strProductLevel == SEN2AGRI_L2A_PRD_LEVEL) ||
          (m_strProductLevel == SEN2AGRI_L3A_PRD_LEVEL))
      {
          iChannelNo = 3;
          if(m_strLutFile != "") {
            bUseLut = true;
            bIsRgbImg = true;
          }
      }
      if(((m_strProductLevel == SEN2AGRI_L3B_PRD_LEVEL) ||
          (m_strProductLevel == SEN2AGRI_L3C_PRD_LEVEL) ||
          (m_strProductLevel == SEN2AGRI_L3D_PRD_LEVEL) ||
          (m_strProductLevel == SEN2AGRI_L4A_PRD_LEVEL) ||
          (m_strProductLevel == SEN2AGRI_L4B_PRD_LEVEL)) &&
          (m_strLutFile != ""))
      {
            iChannelNo = 3;
            bUseLut = true;
            if(m_strProductLevel == SEN2AGRI_L4A_PRD_LEVEL || m_strProductLevel == SEN2AGRI_L4B_PRD_LEVEL) {
                bIsRangeMapFile = false;
            }
      }

      //std::cout << "ChannelNo = " << iChannelNo << std::endl;

      for(int j = 1; j <= iChannelNo; j++)
      {
        strChannelsList.emplace_back("Channel" + std::to_string(j));
      }

      for (const auto &tileID : m_tileIDList) {
          for (const auto &previewFileEl : m_previewList) {

              if(tileID.strTileID == previewFileEl.strTileID)
              {
                 //build producty preview file name for tile
                 strTilePreviewFullPath = tileID.strTileNameWithoutExt + JPEG_EXTENSION;
                 strTilePreviewFullPath = ReplaceString(strTilePreviewFullPath, METADATA_CATEG, QUICK_L0OK_IMG_CATEG);

                 bool bQuicklookGenerated = false;
                 if(bUseLut) {
                     std::string outL3BRgbPreviewFile = previewFileEl.strPreviewFileName + "_RGB.tif";
                     if(!generateRgbFromLut(previewFileEl.strPreviewFileName, outL3BRgbPreviewFile, m_strLutFile,
                                            bIsRgbImg, bIsRangeMapFile)) {
                         otbAppLogWARNING("Error creating RGB file from LUT " << strTilePreviewFullPath);
                     } else {
                         //transform .tif file in .jpg file directly in tile directory
                         if(!generateQuicklook(outL3BRgbPreviewFile, strChannelsList, strTilePreviewFullPath)) {
                             otbAppLogWARNING("Error creating quicklook file " << strTilePreviewFullPath);
                         } else {
                             bQuicklookGenerated = true;
                         }
                     }
                     remove(outL3BRgbPreviewFile.c_str());
                 }
                 if(!bQuicklookGenerated) {
                     //transform .tif file in .jpg file directly in tile directory
                     if(!generateQuicklook(previewFileEl.strPreviewFileName, strChannelsList, strTilePreviewFullPath)) {
                         otbAppLogWARNING("Error creating quickloof file " << strTilePreviewFullPath);
                     }
                 }
            }
        }
      }
  }

  void TransferMainProductPreviewFile() {
        std::string strProductPreviewFullPath = m_strDestRoot + "/" + m_strProductDirectoryName +
                "/" + m_productMetadata.GeneralInfo.ProductInfo.PreviewImageURL;
        const std::string strMosaicPreviewFileName = BuildFileName(QUICK_L0OK_IMG_CATEG + std::string("_") +
                                                                   LEGACY_FOLDER_CATEG, "", ".jpg");
        std::string mosaicPreviewFullPath = m_strDestRoot + "/" + m_strProductDirectoryName +
                "/" + LEGACY_DATA_FOLDER_NAME + "/" + strMosaicPreviewFileName;
        CopyFile(strProductPreviewFullPath, mosaicPreviewFullPath);

        try
        {
              boost::filesystem::remove(mosaicPreviewFullPath);
        } catch(boost::filesystem::filesystem_error const & e) {
              otbAppLogWARNING("Error removing file " << mosaicPreviewFullPath
                               << "Error was: " << e.what());
        }

  }

  void TransferGenericVectorFiles(const std::vector<std::string> &files)
  {
      for(const auto &file: files) {
          boost::filesystem::path filePath(file);
          if (boost::filesystem::is_directory(filePath)) {
              for(const boost::filesystem::directory_entry& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(filePath), {})) {
                //std::cout << entry.path(). << "\n";
                std::string fileName = entry.path().filename().c_str();
                CopyFile(m_strDestRoot + "/" + m_strProductDirectoryName + "/" + VECTOR_FOLDER_NAME + "/" + fileName, entry.path().string());

              }
          } else {
              std::string fileName = filePath.filename().c_str();
              CopyFile(m_strDestRoot + "/" + m_strProductDirectoryName + "/" + VECTOR_FOLDER_NAME + "/" + fileName, file);
          }
      }
  }



   void CopyFile(const std::string &strDest, const std::string &strSrc)
   {
       boost::system::error_code ec;
       boost::filesystem::copy_file(strSrc, strDest, boost::filesystem::copy_option::overwrite_if_exists, ec);
       if (ec) {
           otbAppLogWARNING("Error copying file " << strSrc << " to file " << strDest);
       }
   }


  void AddAcquisitionDate(const std::string &acquisitionDate) {
      if(std::find(m_acquisitionDatesList.begin(), m_acquisitionDatesList.end(), acquisitionDate) == m_acquisitionDatesList.end()) {
          m_acquisitionDatesList.push_back(acquisitionDate);
      }
  }

  std::string getDateFromAquisitionDateTime(const std::string &acqDateTime) {
      std::string::size_type pos = acqDateTime.find('T');
      return (pos!= std::string::npos) ? acqDateTime.substr(0, pos) : acqDateTime;
  }

  void LoadAllDescriptors(const std::vector<std::string> &descriptors)
  {
      auto factory = MetadataHelperFactory::New();
      for (const std::string& desc : descriptors) {
          const std::unique_ptr<MetadataHelper<float, uint8_t>> &pHelper = factory->GetMetadataHelper<float, uint8_t>(desc);
          const std::string &acqDateTime = pHelper->GetAcquisitionDateTime();
          AddAcquisitionDate(acqDateTime);
      }
  }

  bool IsPreviewNeeded(const rasterInfo &rasterFileEl, int nRes) {
        if(!rasterFileEl.bNeedsPreview || rasterFileEl.bIsQiData ||
           ((rasterFileEl.iRasterType == COMPOSITE_REFLECTANCE_RASTER) && (nRes != 10))) {
            return false;
        }
        return true;
  }

  std::vector<std::string> GetFileListFromFile(const std::vector<std::string> &tileAndFileName) {
      std::vector<std::string> retList;
      int cnt = tileAndFileName.size();
      if((cnt > 0) && (cnt % 2) == 0) {
          int nbTupples = tileAndFileName.size() / 2;
          for(int i = 0; i < nbTupples; i++) {
              retList.push_back(tileAndFileName[i*2]);
              const std::vector<std::string> &filesList = GetFileListFromFile(tileAndFileName[i*2+1]);
              if(filesList.size() > 0) {
                  retList.insert(std::end(retList), std::begin(filesList), std::end(filesList));
              } else {
                  // remove the previous tile id if we don't have no file
                  retList.pop_back();
              }
          }
      } else {
          itkExceptionMacro("Invalid usage. You should provide a tile name and a file name containing file paths");
      }
      return retList;
  }
  std::vector<std::string> GetFileListFromFile(const std::string &fileName) {
      std::vector<std::string> retList;
      // if we have actually a TIF file, just return it
      if (boost::algorithm::ends_with(fileName, ".TIF")) {
          retList.push_back(fileName);
          return retList;
      }

        std::ifstream file;
        file.open(fileName);
        if (!file.is_open()) {
            return retList;
        }

        std::string value;
        while (std::getline(file, value)) {
          retList.push_back(value);
        }
        // close the file
        file.close();
        return retList;
  }

  bool ExecuteAgregateTiles(const std::string &strMainFolderFullPath, int rescaleRes) {
      std::cout << "Starting aggregating tiles for product " << strMainFolderFullPath << std::endl;
      if(rescaleRes <= 20) {
          rescaleRes = 60;
      }
      std::vector<const char *> args;
      args.emplace_back("-prodfolder");
      args.emplace_back(strMainFolderFullPath.c_str());
      args.emplace_back("-rescaleval");
      std::string rescaleStr = std::to_string(rescaleRes);
      args.emplace_back(rescaleStr.c_str());
      return ExecuteExternalProgram("aggregate_tiles.py", args);
  }

  bool ExecuteGdalTranslateOps(const std::string &rasterFileName, bool bHasDiscreteValues) {
      bool compress = (GetParameterInt("compress") != 0);
      bool cog = (GetParameterInt("cog") != 0);
      if (!compress && !cog) {
          return true;
      }
      std::cout << "Starting gdal operations for raster " << rasterFileName << std::endl;
      std::vector<const char *> args;
      if (compress) {
          args.emplace_back("--compress");
          args.emplace_back("DEFLATE");
      } else {
          args.emplace_back("--no-compress");
      }

      if (!bHasDiscreteValues) {
          args.emplace_back("--no-data");
          args.emplace_back("-10000");
      }

      if (cog) {
          if (bHasDiscreteValues) {
              args.emplace_back("--resampler");
              args.emplace_back("nearest");
          } else {
              args.emplace_back("--resampler");
              args.emplace_back("average");
          }
          args.emplace_back("--overviews");
          args.emplace_back("--tiled");
      } else {
          args.emplace_back("--no-overviews");
          args.emplace_back("--stripped");
      }
      args.emplace_back(rasterFileName.c_str());
      return ExecuteExternalProgram("optimize_gtiff.py", args);
  }

  bool ExecuteExternalProgram(const char *appExe, std::vector<const char *> appArgs) {
      std::vector<const char *> args;
      std::string cmdInfo;
      args.emplace_back(appExe);
      cmdInfo = appExe;
      for (auto arg : appArgs) {
          cmdInfo += " ";
          cmdInfo += arg;
      }
      otbAppLogINFO("Executing external command: " << cmdInfo);

 #ifndef _WIN32
      for (auto arg : appArgs) {
          args.emplace_back(arg);
      }
      args.emplace_back(nullptr);

      int error, status;
      pid_t pid, waitres;
      posix_spawnattr_t attr;
      posix_spawnattr_init(&attr);
      posix_spawnattr_setflags(&attr, POSIX_SPAWN_USEVFORK);
      error = posix_spawnp(&pid, args[0], NULL, &attr, (char *const *)args.data(), environ);
      if(error != 0) {
        otbAppLogWARNING("Error creating process for " << appExe << ". The resulting files will not be created. Error was: " << error);
        return false;
      }
      waitres = waitpid(pid, &status, 0);
      if(waitres == pid && (WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
        return true;
      }
#else
      int status = system(cmdInfo.c_str());
      if (status == -1) {
          otbAppLogWARNING("Error creating process for " << appExe << ". The resulting files will not be created. Error was: " << errno);
          return false;
      } else if (status == 0) {
          return true;
      }
#endif
      otbAppLogWARNING("Error running " << appExe << ". The resulting file(s) might not be created. The return was: " << status);
      return false;
  }

  std::string BuildProductDirectoryName() {
      const std::string &strCreationDate = currentDateTimeFormattted("%Y%m%dT%H%M%S");
      return BuildFileName(MAIN_FOLDER_CATEG, "", "", m_strTimePeriod, m_strSiteId, strCreationDate);
  }

  std::string BuildTileName(const std::string &tileId) {
      return BuildFileName("", tileId);
  }

  std::string BuildFileName(const std::string &fileCateg, const std::string &tileId, const std::string &extension="", const std::string &strTimePeriod = "",
                            const std::string &site = "", const std::string &creationDate = "", const std::string &region = "") {
      std::string strFileName = "{project_id}_{product_level}_{file_category}_S{originator_site}_{creation_date}_V{time_period}_T{tile_id}_R{region}";
      strFileName = ReplaceString(strFileName, "{project_id}", PROJECT_ID);
      strFileName = ReplaceString(strFileName, "{product_level}", m_strProductLevel);
      if(fileCateg.length() > 0) {
          strFileName = ReplaceString(strFileName, "{file_category}", fileCateg);
      } else {
          strFileName = ReplaceString(strFileName, "_{file_category}", "");
      }

      if(site.length() > 0) {
          strFileName = ReplaceString(strFileName, "{originator_site}", site);
      } else {
          strFileName = ReplaceString(strFileName, "_S{originator_site}", "");
      }

      if(creationDate.length() > 0) {
          strFileName = ReplaceString(strFileName, "{creation_date}", creationDate);
      } else {
          strFileName = ReplaceString(strFileName, "_{creation_date}", "");
      }

      std::string strTimeValue = m_strTimePeriod;
      if(strTimePeriod.length() > 0) {
          strTimeValue = strTimePeriod;
      }
      if(LevelHasAcquisitionTime()) {
          strFileName = ReplaceString(strFileName, "_V{time_period}", "_A" + strTimeValue);
      } else {
          strFileName = ReplaceString(strFileName, "{time_period}", strTimeValue);
      }

      if(tileId.length() == 0 || tileId.find("_T") == 0) {
          strFileName = ReplaceString(strFileName, "_T{tile_id}", tileId);
      } else {
          strFileName = ReplaceString(strFileName, "_T{tile_id}", "_T" + tileId);
      }

      if (!region.empty()) {
          strFileName = ReplaceString(strFileName, "{region}", region);
      } else {
          strFileName = ReplaceString(strFileName, "_R{region}", "");
      }

      if(extension.length() > 0)
          strFileName.append(extension);

      return strFileName;
  }

  bool LevelHasAcquisitionTime() {
      if(m_strProductLevel == "L3B") {
          return true;
      }
      return false;
  }

  std::string CheckProductConsistency(const std::string &strProductMainFolder) {
        std::string retPath = strProductMainFolder;

        bool bValidProduct = true;
        for (rasterInfo &rasterFileEl : m_rasterInfoList) {
            // we check only the rasters and not the qi data
            if(!rasterFileEl.bIsQiData) {
                std::vector<const char *> args;
                args.emplace_back(rasterFileEl.strNewRasterFullPath.c_str());
                args.emplace_back("--number-of-bands");
                std::string rasterBandsNo = std::to_string(rasterFileEl.nRasterExpectedBandsNo);
                args.emplace_back(rasterBandsNo.c_str());
                if(!ExecuteExternalProgram("validity_checker.py", args)) {
                    bValidProduct = false;
                }
            }
            if(!bValidProduct) {
                // The product is not valid ... change its name
                retPath = strProductMainFolder + "_NOTV";
                otbAppLogWARNING("Invalid product found in folder " << strProductMainFolder
                                 << ". Trying to rename it into " << retPath);
                bool bErr = false;
                try {
                    boost::filesystem::rename(strProductMainFolder, retPath);
                }
                catch (...)
                {
                    bErr = true;

                }
                if(bErr || !boost::filesystem::exists(retPath)) {
                    otbAppLogWARNING("Error renaming with _NOTV the folder " << strProductMainFolder);
                    // in this case restore the folder name
                    retPath = strProductMainFolder;
                }
                break;
            }
        }

        return retPath;
  }


private:
  std::string m_strProductLevel;
  std::string m_strTimePeriod;
  std::string m_strLutFile;
  std::string m_strDestRoot;
  std::string m_strBaseline;
  std::string m_strSiteId;
  std::vector<previewInfo> m_previewList;
  std::vector<std::string> m_GIPPList;
  std::vector<std::string> m_ISDList;
  std::vector<qualityInfo> m_qualityList;
  std::vector<tileInfo> m_tileIDList;

  ProductFileMetadata m_productMetadata;
  std::vector<rasterInfo> m_rasterInfoList;
  std::string m_strProductDirectoryName;

  //bool m_bIsHDR; /* true if is  loaded a .HDR fiel, false if is a .xml file */
  //std::string m_strTileNameRoot;
  //std::string m_strProductFileName;
  std::vector<geoProductInfo> m_geoProductInfo;

    std::vector<std::string> m_acquisitionDatesList;
    bool m_bDynamicallyTimePeriod;

    bool m_bVectPrd;

};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::ProductFormatter)


