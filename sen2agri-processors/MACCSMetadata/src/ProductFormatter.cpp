#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbOGRIOHelper.h"
#include "ogr_geometry.h"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/progress.hpp"
#include <iostream>

#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <spawn.h>
#include <fstream>

#include "ProductMetadataWriter.hpp"
#include "TileMetadataWriter.hpp"
#include "MACCSMetadataReader.hpp"
#include "SPOT4MetadataReader.hpp"

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
#define LAI_MDATE_ERR_SUFFIX            "MLAIERR"
#define LAI_REPR_SUFFIX                 "SLAIR"
#define LAI_FIT_SUFFIX                  "SLAIF"
#define PHENO_SUFFIX                    "SPHENO"
#define PHENO_FLAGS_SUFFIX              "MPHENOFLG"
#define LAI_MONO_DATE_FLAGS_SUFFIX      "MMONODFLG"
#define LAI_REPROC_FLAGS_SUFFIX         "MLAIRFLG"
#define LAI_FITTED_FLAGS_SUFFIX         "MLAIFFLG"
#define CROP_MASK_IMG_SUFFIX            "CM"
#define CROP_TYPE_IMG_SUFFIX            "CT"
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
#define QUALITY_CATEG "QLT"
#define NO_DATA_VALUE "-10000"

#define ORIGINATOR_SITE "CSRO"

#define TILES_FOLDER_NAME           "TILES"
#define AUX_DATA_FOLDER_NAME        "AUX_DATA"
#define LEGACY_DATA_FOLDER_NAME     "LEGACY_DATA"
#define IMG_DATA_FOLDER_NAME        "IMG_DATA"
#define QI_DATA_FOLDER_NAME         "QI_DATA"

#define LANDSAT    "LANDSAT"
#define SENTINEL   "SENTINEL"

#define L2A_PRODUCT             "L2A product"
#define COMPOSITE_PRODUCT       "Composite"
#define LAI_MONO_DATE_PRODUCT   "LAI mono-date"
#define LAI_REPROC_PRODUCT      "LAI reprocessed"
#define LAI_FITTED_PRODUCT      "LAI reprocessed at end of season (fitted)"
#define PHENO_NDVI_PRODUCT      "Phenological NDVI Metrics"
#define CROP_MASK               "Crop mask"
#define CROP_TYPE               "Crop type"

typedef itk::MACCSMetadataReader MACCSMetadataReaderType;
typedef itk::SPOT4MetadataReader SPOT4MetadataReaderType;

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
    {5, "705 nm", "Vegetation red-edge", 10, "RE1", ""},
    {6, "740 nm", "Vegetation red-edge", 10, "RE2", ""},
    {7, "783 nm", "Vegetation red-edge", 10, "RE3", ""},
    {8, "865 nm", "Vegetation red-edge", 10, "NIR2", ""},
    {9, "1610 nm", "Short-Wave infrared", 10, "SWIR1", "SWIR1"},
    {10, "2190 nm", "Short-Wave infrared", 10, "SWIR2", ""}
};

typedef enum{
    COMPOSITE_REFLECTANCE_RASTER = 0,
    COMPOSITE_WEIGHTS_RASTER,
    COMPOSITE_FLAGS_MASK,
    COMPOSITE_DATES_MASK,
    LAI_NDVI_RASTER,
    LAI_MONO_DATE_RASTER,
    LAI_MONO_DATE_ERR_RASTER,
    LAI_MONO_DATE_FLAGS,
    LAI_REPROC_FLAGS,
    LAI_FITTED_FLAGS,
    LAI_REPR_RASTER,
    LAI_FIT_RASTER,
    CROP_MASK_RASTER,
    RAW_CROP_MASK_RASTER,
    CROP_TYPE_RASTER,
    PHENO_RASTER,
    PHENO_FLAGS,
    CROP_MASK_FLAGS,
    CROP_TYPE_FLAGS
}rasterTypes;

struct rasterInfo
{
    std::string strRasterFileName;
    int nResolution;
    rasterTypes iRasterType;
    std::string strNewRasterFileName;
    std::string strTileID;
    bool bIsQiData;
    std::string rasterTimePeriod;
    int nRasterExpectedBandsNo;
    std::string strNewRasterFullPath;
};

struct qualityInfo
{
    std::string strFileName;
    std::string strTileID;
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

        AddParameter(ParameterType_InputFilenameList, "processor.vegetation.laimdateflgs", "LAI Mono date flags raster files list for vegetation  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.vegetation.laimdateflgs");

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
        AddParameter(ParameterType_InputFilenameList, "processor.croptype.file", "CROP TYPE raster file  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.croptype.file");

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

        AddParameter(ParameterType_InputFilenameList, "il", "The xml files");
        MandatoryOff("il");

        /*AddParameter(ParameterType_InputFilenameList, "preview", "The quicklook files");
        MandatoryOff("preview");*/
        AddParameter(ParameterType_InputFilenameList, "gipp", "The GIPP files");
        MandatoryOff("gipp");

        AddParameter(ParameterType_Int, "aggregatescale", "The aggregate rescale resolution");
        MandatoryOff("aggregatescale");
        SetDefaultParameterInt("aggregatescale", 60);

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
      // by default, we expect a "timeperiod" parameter
      m_bDynamicallyTimePeriod = false;

      //get product level
      m_strProductLevel = this->GetParameterString("level");

      //get time period (first product date and last product date)
      if(HasValue("timeperiod")) {
        m_strTimePeriod = this->GetParameterString("timeperiod");
      }

      //get processing baseline
      m_strBaseline = this->GetParameterString("baseline");

      //get the site ID
      m_strSiteId = this->GetParameterString("siteid");

      //get destination root
      m_strDestRoot = this->GetParameterString("destroot");

      // Get GIPP file list
      m_GIPPList = this->GetParameterStringList("gipp");

      //read .xml or .HDR files to fill the metadata structures
      // Get the list of input files
      std::vector<std::string> descriptors = this->GetParameterStringList("il");
      LoadAllDescriptors(descriptors);
      m_strMinAcquisitionDate = *std::min_element(std::begin(m_acquisitionDatesList), std::end(m_acquisitionDatesList));
      m_strMaxAcquisitionDate = *std::max_element(std::begin(m_acquisitionDatesList), std::end(m_acquisitionDatesList));
      if(m_strTimePeriod.empty()) {
          m_bDynamicallyTimePeriod = true;
          if(LevelHasAcquisitionTime()) {
              if(m_strMinAcquisitionDate != m_strMaxAcquisitionDate)
                    itkGenericExceptionMacro(<< "You should have the same date for all tiles in the il parameter as this product has Aquisition time: " << m_strProductLevel);
              // we have a single date and min acquisition date should be the same as max acquisition date
              m_strTimePeriod = m_strMaxAcquisitionDate;
          } else {
              // we have an interval
              m_strTimePeriod = m_strMinAcquisitionDate + "_" + m_strMaxAcquisitionDate;
          }
      }

      m_strProductDirectoryName = BuildProductDirectoryName();
      std::string strMainFolderFullPath = m_strDestRoot + "/" + m_strProductDirectoryName;

      //created all folders ierarchy
      bool bDirStructBuiltOk = createsAllFolders(strMainFolderFullPath);

      //if is composite read reflectance rasters, weights rasters, flags masks, dates masks
      if (m_strProductLevel.compare("L3A") == 0)
      {
          std::vector<std::string> rastersList;

           // Get reflectance raster file list
          rastersList = this->GetParameterStringList("processor.composite.refls");
          //unpack and add them in global raster list
          UnpackRastersList(rastersList, COMPOSITE_REFLECTANCE_RASTER, false);

          //get weights rasters list
          rastersList = this->GetParameterStringList("processor.composite.weights");
          UnpackRastersList(rastersList, COMPOSITE_WEIGHTS_RASTER, true);

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
      if (m_strProductLevel.compare("L3B") == 0) {
          std::vector<std::string> rastersList;

          rastersList = this->GetParameterStringList("processor.vegetation.laindvi");
          UnpackRastersList(rastersList, LAI_NDVI_RASTER, false);
          rastersList = this->GetParameterStringList("processor.vegetation.laimonodate");
          UnpackRastersList(rastersList, LAI_MONO_DATE_RASTER, false);
          rastersList = this->GetParameterStringList("processor.vegetation.laimonodateerr");
          UnpackRastersList(rastersList, LAI_MONO_DATE_ERR_RASTER, true);
          rastersList = this->GetParameterStringList("processor.vegetation.laimdateflgs");
          UnpackRastersList(rastersList, LAI_MONO_DATE_FLAGS, true);
      }

      // read LAIREPR raster files list
      if (m_strProductLevel.compare("L3C") == 0) {
          std::vector<std::string> rastersList;

          // LAI Reprocessed raster files list
          rastersList = this->GetFileListFromFile(this->GetParameterStringList("processor.vegetation.filelaireproc"));
          UnpackRastersList(rastersList, LAI_REPR_RASTER, false);
          rastersList = this->GetFileListFromFile(this->GetParameterStringList("processor.vegetation.filelaireprocflgs"));
          UnpackRastersList(rastersList, LAI_REPROC_FLAGS, true);
     }

      // read LAIFIT raster files list
      if (m_strProductLevel.compare("L3D") == 0) {
          std::vector<std::string> rastersList;

          rastersList = this->GetFileListFromFile(this->GetParameterStringList("processor.vegetation.filelaifit"));
          UnpackRastersList(rastersList, LAI_FIT_RASTER, false);
          rastersList = this->GetFileListFromFile(this->GetParameterStringList("processor.vegetation.filelaifitflgs"));
          UnpackRastersList(rastersList, LAI_FITTED_FLAGS, true);
     }

      //read Phenological NDVI metrics raster files list
      if (m_strProductLevel.compare("L3E") == 0) {
          std::vector<std::string> rastersList;
          rastersList = this->GetParameterStringList("processor.phenondvi.metrics");
          UnpackRastersList(rastersList, PHENO_RASTER, false);
          rastersList = this->GetParameterStringList("processor.phenondvi.flags");
          UnpackRastersList(rastersList, PHENO_FLAGS, true);
     }

      //if is CROP MASK, read raster file
      if (m_strProductLevel.compare("L4A") == 0)
      {

          std::vector<std::string> filesList;
          filesList = this->GetParameterStringList("processor.cropmask.file");
          UnpackRastersList(filesList, CROP_MASK_RASTER, false);

          filesList = this->GetParameterStringList("processor.cropmask.rawfile");
          UnpackRastersList(filesList, RAW_CROP_MASK_RASTER, false);

          //get quality file list
          filesList = this->GetParameterStringList("processor.cropmask.quality");
          UnpackNonRastersList(filesList);

          // get the CropMask flags
          filesList = this->GetParameterStringList("processor.cropmask.flags");
          UnpackRastersList(filesList, CROP_MASK_FLAGS, true);
      }

      //if is CROP TYPE, read raster file
      if (m_strProductLevel.compare("L4B") == 0)
      {
          // Get reflectance raster file list
          std::vector<std::string> filesList;
          filesList = this->GetParameterStringList("processor.croptype.file");
          UnpackRastersList(filesList, CROP_TYPE_RASTER, false);

          //get quality file list
          filesList = this->GetParameterStringList("processor.croptype.quality");
          UnpackNonRastersList(filesList);

          // get the CropType flags
          filesList = this->GetParameterStringList("processor.croptype.flags");
          UnpackRastersList(filesList, CROP_TYPE_FLAGS, true);
      }

      for (const auto &tileInfoEl : m_tileIDList ) {
            std::cout << "TileID =" << tileInfoEl.strTileID << "  strTilePath =" << tileInfoEl.strTilePath  << std::endl;
      }

      if(bDirStructBuiltOk)
      {
          for (tileInfo &tileEl : m_tileIDList) {
              CreateAndFillTile(tileEl, strMainFolderFullPath);
          }

      }

      if(bDirStructBuiltOk)
      {
          TransferPreviewFiles();
          TransferAndRenameGIPPFiles();          
          const std::string strProductFileName = BuildFileName(METADATA_CATEG, "", ".xml");
          generateProductMetadataFile(strMainFolderFullPath + "/" + strProductFileName);
          bool bAgSuccess = ExecuteAgregateTiles(strMainFolderFullPath, this->GetParameterInt("aggregatescale"));
          std::cout << "Aggregating tiles " << (bAgSuccess ? "SUCCESS!" : "FAILED!") << std::endl;
          TransferMainProductPreviewFile();
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
              struct stat buf;
              if (stat(rasterFileEl.strRasterFileName.c_str(), &buf) != -1) {
                      return true;
              }
          }
      }
      return false;
  }

  void UnpackRastersList(std::vector<std::string> &rastersList, rasterTypes rasterType, bool bIsQiData)
  {
      std::string strTileID;
      rasterInfo rasterInfoEl;
      rasterInfoEl.bIsQiData = bIsQiData;

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
              // update the date
              if(bAllRastersHaveDate) {
                  if(LevelHasAcquisitionTime()) {
                      rasterInfoEl.rasterTimePeriod = m_acquisitionDatesList[curRaster];
                  } else {
                      rasterInfoEl.rasterTimePeriod = m_strMinAcquisitionDate + "_" + m_acquisitionDatesList[curRaster];
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

  void UnpackNonRastersList(std::vector<std::string> &filesList) {
      qualityInfo qualityInfoEl;
      std::string strTileID;
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
          } else  {
              qualityInfoEl.strFileName = fileEl;
              qualityInfoEl.strTileID = strTileID;
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
      bool bSuccess = false;
      int nRC = ::mkdir( path.c_str(), 0775 );
      if( nRC == -1 )
      {
          switch( errno )
          {
              case ENOENT:
                  //parent didn't exist, try to create it
                  if( mkPath( path.substr(0, path.find_last_of('/')) ) )
                      //Now, try to create again.
                      bSuccess = 0 == ::mkdir( path.c_str(), 0775 );
                  else
                      bSuccess = false;
                  break;
              case EEXIST:
                  //Done!
                  bSuccess = true;
                  break;
              default:
                  bSuccess = false;
                  break;
          }
      }
      else
          bSuccess = true;
      return bSuccess;
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

      /* create TILES subfolder */
      if(mkPath(strMainFolderPath + "/" + TILES_FOLDER_NAME) == false)
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
      if (m_strProductLevel.compare("L3A") == 0)
         return COMPOSITE_PRODUCT;
      if (m_strProductLevel.compare("L3B") == 0)
         return LAI_MONO_DATE_PRODUCT;
      if (m_strProductLevel.compare("L3C") == 0)
         return LAI_REPROC_PRODUCT;
      if (m_strProductLevel.compare("L3D") == 0)
         return LAI_FITTED_PRODUCT;
      if (m_strProductLevel.compare("L3E") == 0)
         return PHENO_NDVI_PRODUCT;
      if (m_strProductLevel.compare("L4A") == 0)
         return CROP_MASK;
      if (m_strProductLevel.compare("L4B") == 0)
         return CROP_TYPE;
      return "Unknowkn product";
  }

  void FillBandList()
  {
      Band bandEl;
      if (m_strProductLevel.compare("L3A") == 0)
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

  bool generateQuicklook(const std::string &rasterFullFilePath, const std::vector<std::string> &channels,const std::string &jpegFullFilePath)
  {
      int error, status;
          pid_t pid, waitres;
//          /* Make sure we have no child processes. */
//          while (waitpid(-1, NULL, 0) != -1)
//              ;
//          assert(errno == ECHILD);

          std::vector<const char *> args;
          args.emplace_back("otbcli_Quicklook");
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
          args.emplace_back(nullptr);

          posix_spawnattr_t attr;
          posix_spawnattr_init(&attr);
          posix_spawnattr_setflags(&attr, POSIX_SPAWN_USEVFORK);
          error = posix_spawnp(&pid, args[0], NULL, &attr, (char *const *)args.data(), NULL);
          if(error != 0) {
              otbAppLogWARNING("Error creating process for otbcli_Quicklook. The preview file will not be created. Error was: " << error);
              return false;
          }
          posix_spawnattr_destroy(&attr);
          waitres = waitpid(pid, &status, 0);
          if(waitres == pid && (WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
              return true;
          }
          otbAppLogWARNING("Error running otbcli_Quicklook. The preview file might not be created. The return of otbcli_Quicklook was: " << status);
          return false;
  }

  void generateTileMetadataFile(tileInfo &tileInfoEl)
  {
      TileSize tileSizeEl;
      TileGeoposition tileGeoposition;
      int iResolution;
      bool bResolutionExistingAlready = false;
      bool bGeoPositionExistingAlready = false;
      bool bPreview = !m_previewList.empty();

      auto writer = itk::TileMetadataWriter::New();

      std::string strTile = BuildFileName(METADATA_CATEG, tileInfoEl.strTileID, "");
      tileInfoEl.tileMetadata.TileID = strTile;
      tileInfoEl.tileMetadata.ProductLevel = "Level-"  + m_strProductLevel;

      for (rasterInfo &rasterFileEl : m_rasterInfoList) {
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

              if((!bPreview) && IsPreviewNeeded(rasterFileEl, iResolution))
              {
                previewInfo previewInfoEl;
                previewInfoEl.strPreviewFileName = rasterFileEl.strRasterFileName;
                previewInfoEl.strTileID = tileInfoEl.strTileID;
                m_previewList.emplace_back(previewInfoEl);
                bPreview = true;
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
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AuxListContent.GIPP = " NO";
      }
      else
      {
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AuxListContent.GIPP = " YES";
      }
      m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.ProductFormat = "SAFE";
      m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AggregationFlag = true;


      m_productMetadata.GeneralInfo.ProductImageCharacteristics.ImageDisplayOrder.RedChannel = 0;
      m_productMetadata.GeneralInfo.ProductImageCharacteristics.ImageDisplayOrder.GreenChannel = 0;
      m_productMetadata.GeneralInfo.ProductImageCharacteristics.ImageDisplayOrder.BlueChannel = 0;

      if ((m_strProductLevel.compare("L2A") == 0) ||
          (m_strProductLevel.compare("L3A") == 0))
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
      specialValue.SpecialValueIndex = "1";
      specialValue.SpecialValueText = "NOTVALID";
      m_productMetadata.GeneralInfo.ProductImageCharacteristics.SpecialValuesList.emplace_back(specialValue);

      specialValue.SpecialValueIndex = NO_DATA_VALUE;
      specialValue.SpecialValueText = "NODATA";
      m_productMetadata.GeneralInfo.ProductImageCharacteristics.SpecialValuesList.emplace_back(specialValue);


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
          itkExceptionMacro("Fail to create tile directory!");
      }

      /* create IMG_DATA subfolder */
      if(mkPath(strTileFullPath + "/" + IMG_DATA_FOLDER_NAME) == false)
      {
           //fail to create IMG_DATA subfolder
          bResult = false;
          itkExceptionMacro("Fail to create IMG_DATA subfolder in tile directory!");
      }

      /* create QI_DATA subfolder */
      if(mkPath(strTileFullPath + "/" + QI_DATA_FOLDER_NAME) == false)
      {
           //fail to create QI_DATA subfolder
          bResult = false;
          itkExceptionMacro("Fail to create QI_DATA subfolder in tile directory!");
      }

      return bResult;
  }

  void ComputeNewNameOfRasterFiles(const tileInfo &tileInfoEl)
  {
      std::string rasterCateg;

      for (rasterInfo &rasterFileEl : m_rasterInfoList) {
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
                case LAI_MONO_DATE_FLAGS:
                    rasterCateg = LAI_MONO_DATE_FLAGS_SUFFIX;
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
              CopyFile(rasterFileEl.strNewRasterFullPath, rasterFileEl.strRasterFileName);
          }
        }
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
              strNewQualityFileName = BuildFileName(QUALITY_CATEG, "", p.extension().string());

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

      if ((m_strProductLevel.compare("L2A") == 0) ||
          (m_strProductLevel.compare("L3A") == 0) )
      {
          iChannelNo = 3;
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
//                //for the moment the preview file for product and tile are the same

//                //build product preview file name
//                 strProductPreviewFullPath = BuildFileName(QUICK_L0OK_IMG_CATEG, "", JPEG_EXTENSION);

//                 m_productMetadata.GeneralInfo.ProductInfo.PreviewImageURL = strProductPreviewFullPath;

//                 strProductPreviewFullPath = m_strDestRoot + "/" + m_strProductDirectoryName + "/" + strProductPreviewFullPath;

                 //std::cout << "ProductPreviewFullPath = " << strProductPreviewFullPath << std::endl;

                 //build producty preview file name for tile
                 strTilePreviewFullPath = tileID.strTileNameWithoutExt + JPEG_EXTENSION;
                 strTilePreviewFullPath = ReplaceString(strTilePreviewFullPath, METADATA_CATEG, QUICK_L0OK_IMG_CATEG);
                 //transform .tif file in .jpg file directly in tile directory
                 if(!generateQuicklook(previewFileEl.strPreviewFileName, strChannelsList, strTilePreviewFullPath)) {
                     otbAppLogWARNING("Error creating quickloof file " << strTilePreviewFullPath);
                 }


//                 //transform .tif file in .jpg file directly in product directory
//                 if(generateQuicklook(previewFileEl.strPreviewFileName, strChannelsList, strTilePreviewFullPath)) {
//                     //build producty preview file name for tile
//                     strTilePreviewFullPath = tileID.strTileNameWithoutExt + JPEG_EXTENSION;
//                     strTilePreviewFullPath = ReplaceString(strTilePreviewFullPath, METADATA_CATEG, QUICK_L0OK_IMG_CATEG);
//                     CopyFile(strTilePreviewFullPath, strProductPreviewFullPath);
//                 }
                 //remove  file with extension jpg.aux.xml generated after preview obtained

                 std::string strFileToBeRemoved = strTilePreviewFullPath + ".aux.xml";
                 //std::cout << "Remove file: " <<  strFileToBeRemoved<< std::endl;
                 remove(strFileToBeRemoved.c_str());
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

   void CopyFile(const std::string &strDest, const std::string &strSrc)
   {
       struct stat buf;
       if (stat(strSrc.c_str(), &buf) != -1) {
           std::ifstream  src(strSrc, std::ios::binary);
           std::ofstream  dst(strDest, std::ios::binary);

           dst << src.rdbuf();
       } else {
           otbAppLogWARNING("Error copying file " << strSrc << " to file " << strDest);
       }
   }


  void FillMetadataInfoForLandsat(std::unique_ptr<MACCSFileMetadata> &metadata)
  {
      /* the source is a HDR file */

    SpecialValues specialValue;
    specialValue.SpecialValueText = "NODATA";
    specialValue.SpecialValueIndex = metadata->ImageInformation.NoDataValue;
    m_productMetadata.GeneralInfo.ProductImageCharacteristics.SpecialValuesList.emplace_back(specialValue);

    AddAcquisitionDate(metadata->InstanceId.AcquisitionDate);
  }

  void FillMetadataInfoForSPOT(std::unique_ptr<SPOT4Metadata> &metadata)
  {
      /* the source is a SPOT file */
      //nothing to load????
      AddAcquisitionDate(metadata->Header.DatePdv.substr(0,4) +
              metadata->Header.DatePdv.substr(5,2) + metadata->Header.DatePdv.substr(8,2));
  }

  void AddAcquisitionDate(const std::string &acquisitionDate) {
      if(std::find(m_acquisitionDatesList.begin(), m_acquisitionDatesList.end(), acquisitionDate) == m_acquisitionDatesList.end()) {
          m_acquisitionDatesList.push_back(acquisitionDate);
      }
  }

  void LoadAllDescriptors(std::vector<std::string> descriptors)
  {
      // load all descriptors
      MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
      SPOT4MetadataReaderType::Pointer spot4MetadataReader = SPOT4MetadataReaderType::New();

      for (const std::string& desc : descriptors) {


          if (auto meta = maccsMetadataReader->ReadMetadata(desc)) {
              // add the information to the list
              if (meta->Header.FixedHeader.Mission.find(LANDSAT) != std::string::npos) {
                  // Interpret landsat product
                  //m_bIsHDR = true;
                  FillMetadataInfoForLandsat(meta);
              } else if (meta->Header.FixedHeader.Mission.find(SENTINEL) != std::string::npos) {
                  // Interpret sentinel product
                  //m_bIsHDR  = true;
                  FillMetadataInfoForLandsat(meta);
              } else {
                  itkExceptionMacro("Unknown mission: " + meta->Header.FixedHeader.Mission);
              }

          }else if (auto meta = spot4MetadataReader->ReadMetadata(desc)) {

              //m_bIsHDR = false;
              FillMetadataInfoForSPOT(meta);
          } else {
              itkExceptionMacro("Unable to read metadata from " << desc);
          }
      }
  }

  bool IsPreviewNeeded(const rasterInfo &rasterFileEl, int nRes) {
        if(rasterFileEl.bIsQiData ||
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
              std::vector<std::string> filesList = GetFileListFromFile(tileAndFileName[i*2+1]);
              retList.insert(std::end(retList), std::begin(filesList), std::end(filesList));
          }
      } else {
          itkExceptionMacro("Invalid usage. You should provide a tile name and a file name containing file paths");
      }
      return retList;
  }
  std::vector<std::string> GetFileListFromFile(const std::string &fileName) {
        std::ifstream file;
        std::vector<std::string> retList;
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

  bool ExecuteExternalProgram(const char *appExe, std::vector<const char *> appArgs) {
          int error, status;
          pid_t pid, waitres;
          std::vector<const char *> args;
          args.emplace_back(appExe);
          for(unsigned int i = 0; i<appArgs.size(); i++) {
                args.emplace_back(appArgs[i]);
          }
          args.emplace_back(nullptr);

          posix_spawnattr_t attr;
          posix_spawnattr_init(&attr);
          posix_spawnattr_setflags(&attr, POSIX_SPAWN_USEVFORK);
          error = posix_spawnp(&pid, args[0], NULL, &attr, (char *const *)args.data(), environ);
          if(error != 0) {
              otbAppLogWARNING("Error creating process for " << appExe << ". The resulting files will not be created. Error was: " << error);
              return false;
          }
          posix_spawnattr_destroy(&attr);
          waitres = waitpid(pid, &status, 0);
          if(waitres == pid && (WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
              return true;
          }
          otbAppLogWARNING("Error running " << appExe << ". The resulting file(s) might not be created. The return was: " << status);
          return false;
  }

  std::string BuildProductDirectoryName() {
      std::string strCreationDate = currentDateTimeFormattted("%Y%m%dT%H%M%S");
      return BuildFileName(MAIN_FOLDER_CATEG, "", "", m_strTimePeriod, m_strSiteId, strCreationDate);
  }

  std::string BuildTileName(const std::string &tileId) {
      return BuildFileName("", tileId);
  }

  std::string BuildFileName(const std::string &fileCateg, const std::string &tileId, const std::string &extension="", const std::string &strTimePeriod = "",
                            const std::string &site = "", const std::string &creationDate = "") {
      std::string strFileName = "{project_id}_{product_level}_{file_category}_S{originator_site}_{creation_date}_V{time_period}_T{tile_id}";
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
                    break;
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
            }
        }

        return retPath;
  }


private:
  std::string m_strProductLevel;
  std::string m_strTimePeriod;
  std::string m_strDestRoot;
  std::string m_strBaseline;
  std::string m_strSiteId;
  std::vector<previewInfo> m_previewList;
  std::vector<std::string> m_GIPPList;
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
    std::string m_strMinAcquisitionDate;
    std::string m_strMaxAcquisitionDate;
    bool m_bDynamicallyTimePeriod;

};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::ProductFormatter)


