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

#define PROJECT_ID   "S2AGRI"
#define GIPP_VERSION "0001"
#define IMAGE_FORMAT "GEOTIFF"
#define TILE_ID      "T15SWC"
#define TIF_EXTENSION      ".TIF"
#define JPEG_EXTENSION      ".jpg"

#define REFLECTANCE_SUFFIX "SRFL"
#define WEIGHTS_SUFFIX     "SWGT"
#define DATES_SUFFIX       "MDAT"
#define FLAGS_SUFFIX       "MFLG"
#define LAI_REPR_SUFFIX    "SLAIR"
#define LAI_FIT_SUFFIX     "SLAIF"
#define PHENO_SUFFIX       "SNVDIMET"

#define GENERIC_CS_TYPE     "GEOGRAPHIC"
#define GENERIC_GEO_TABLES  "EPSG"


#define MAIN_FOLDER_CATEG "PRD"
#define TILE_LEGACY_FOLDER_CATEG "DAT"
#define TRUE_COLOR_FOLDER_CATEG "TCI"
#define QUICK_L0OK_IMG_CATEG "PVI"
#define METADATA_CATEG "MTD"
#define MASK_CATEG "MSK"
#define PARAMETER_CATEG "IPP"
#define QUALITY_CATEG "QLT"
#define NO_DATA_VALUE "-10000"

#define ORIGINATOR_SITE "CSRO"

#define PRODUCT_DESCRIPTOR  "PR"
#define TILE_DESCRIPTOR     "TL"
#define LEGACY_DESCRIPTOR   "LY"

#define TILES_FOLDER_NAME           "TILES"
#define AUX_DATA_FOLDER_NAME        "AUX_DATA"
#define LEGACY_DATA_FOLDER_NAME     "LEGACY_DATA"
#define IMG_DATA_FOLDER_NAME        "IMG_DATA"
#define QI_DATA_FOLDER_NAME         "QI_DATA"

#define LANDSAT    "LANDSAT"
#define SENTINEL   "SENTINEL"

#define L2A_PRODUCT         "L2A product"
#define COMPOSITE_PRODUCT   "Composite"
#define VEGETATION_PRODUCT  "Vegetation status"
#define CROP_MASK           "Crop mask"
#define CROP_TYPE           "Crop type"

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
    REFLECTANCE_RASTER = 0,
    WEIGHTS_RASTER,
    FLAGS_MASK,
    DATES_MASK,
    LAI_REPR_RASTER,
    LAI_FIT_RASTER,
    CROP_MASK_RASTER,
    CROP_TYPE_RASTER,
    PHENO_RASTER
}rasterTypes;

struct rasterInfo
{
    std::string strRasterFileName;
    int nResolution;
    rasterTypes iRasterType;
    std::string strNewRasterFileName;
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
        AddParameter(ParameterType_String, "baseline", "Processing baseline");

        AddParameter(ParameterType_Choice, "processor", "Processor");
        SetParameterDescription("processor", "Specifies the product type");

        AddChoice("processor.composite", "Composite product");
        SetParameterDescription("processor.composite", "Specifies a Composite product");

        AddChoice("processor.vegetation", "Vegetation Status product");
        SetParameterDescription("processor.vegetation", "Specifies a Vegetation Status product");

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

         AddParameter(ParameterType_InputFilenameList, "processor.vegetation.lairepr", "LAI REPR raster files list for vegetation  separated by TILE_{tile_id} delimiter");
         MandatoryOff("processor.vegetation.lairepr");

         AddParameter(ParameterType_InputFilenameList, "processor.vegetation.laifit", "LAI FIT raster files list for vegetation  separated by TILE_{tile_id} delimiter");
         MandatoryOff("processor.vegetation.laifit");

         AddParameter(ParameterType_InputFilenameList, "processor.vegetation.pheno", "Metric estimation raster files list for vegetation  separated by TILE_{tile_id} delimiter");
         MandatoryOff("processor.vegetation.pheno");

//crop type parameters
        AddParameter(ParameterType_InputFilenameList, "processor.croptype.file", "CROP TYPE raster file  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.croptype.file");

        AddParameter(ParameterType_InputFilenameList, "processor.croptype.quality", "CROP TYPE quality file");
        MandatoryOff("processor.croptype.quality");

//crop mask parameters
        AddParameter(ParameterType_InputFilenameList, "processor.cropmask.file", "CROP MASK raster file  separated by TILE_{tile_id} delimiter");
        MandatoryOff("processor.cropmask.file");

        AddParameter(ParameterType_InputFilenameList, "processor.cropmask.quality", "CROP MASK quality file");
        MandatoryOff("processor.cropmask.quality");

        AddParameter(ParameterType_InputFilenameList, "il", "The xml files");
        MandatoryOff("il");

        /*AddParameter(ParameterType_InputFilenameList, "preview", "The quicklook files");
        MandatoryOff("preview");*/
        AddParameter(ParameterType_InputFilenameList, "gipp", "The GIPP files");
        MandatoryOff("gipp");

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

      //get file class
      m_strFileClass = this->GetParameterString("fileclass");

      //get product level
      m_strProductLevel = this->GetParameterString("level");

      //get time period (first product date and last product date)
      m_strTimePeriod = this->GetParameterString("timeperiod");

      //get processing baseline
      m_strBaseline = this->GetParameterString("baseline");

      //get destination root
      m_strDestRoot = this->GetParameterString("destroot");

      //if is composite read reflectance rasters, weights ratsters, flags masks, dates masks
      if (m_strProductLevel.compare("L3A") == 0)
      {
          std::vector<std::string> rastersList;

           // Get reflectance raster file list
          rastersList = this->GetParameterStringList("processor.composite.refls");
          //unpack and add them in global raster list
          UnpackRastersList(rastersList, REFLECTANCE_RASTER);

          //get weights rasters list
          rastersList = this->GetParameterStringList("processor.composite.weights");
          UnpackRastersList(rastersList, WEIGHTS_RASTER);

          rastersList = this->GetParameterStringList("processor.composite.flags");
          UnpackRastersList(rastersList, FLAGS_MASK);

          rastersList = this->GetParameterStringList("processor.composite.dates");
          UnpackRastersList(rastersList, DATES_MASK);

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

      //if is vegetation, read LAIREPR and LAIFIT raster files list
      if (m_strProductLevel.compare("L3B") == 0)
      {
          // Get LAIREPR raster file list
          std::vector<std::string> rastersList;
          rastersList = this->GetParameterStringList("processor.vegetation.lairepr");
          UnpackRastersList(rastersList, LAI_REPR_RASTER);


          //get LAIFIT raster files list
          rastersList = this->GetParameterStringList("processor.vegetation.laifit");
          UnpackRastersList(rastersList, LAI_FIT_RASTER);

          //get PHENO raster files list
          rastersList = this->GetParameterStringList("processor.vegetation.pheno");
          UnpackRastersList(rastersList, PHENO_RASTER);

     }

      //if is CROP MASK, read raster file
      if (m_strProductLevel.compare("L4A") == 0)
      {

          std::vector<std::string> rastersList;
          rastersList = this->GetParameterStringList("processor.cropmask.file");
          UnpackRastersList(rastersList, CROP_MASK_RASTER);


          //get quality file list
          m_qualityList = this->GetParameterStringList("processor.cropmask.quality");
      }

      //if is CROP TYPE, read raster file
      if (m_strProductLevel.compare("L4B") == 0)
      {
          // Get reflectance raster file list
          std::vector<std::string> rastersList;
          rastersList = this->GetParameterStringList("processor.croptype.file");
          UnpackRastersList(rastersList, CROP_TYPE_RASTER);

          //get quality file list
          m_qualityList = this->GetParameterStringList("processor.croptype.quality");
      }

      for (const auto &tileInfoEl : m_tileIDList ) {
            std::cout << "TileID =" << tileInfoEl.strTileID << "  strTilePath =" << tileInfoEl.strTilePath  << std::endl;
      }

      //read .xml or .HDR files to fill the metadata structures
      // Get the list of input files
      std::vector<std::string> descriptors = this->GetParameterStringList("il");

      // Get GIPP file list
      m_GIPPList = this->GetParameterStringList("gipp");

      std::string strCreationDate = currentDateTimeFormattted("%Y%m%dT%H%M%S");

      std::string strMainProductFolderName = "{project_id}_{file_class}_{file_category}_{product_level}_{product_descriptor}_{originator_site}_{creation_date}_V{time_period}_{tile_id}_{processing_baseline}";

      strMainProductFolderName = ReplaceString(strMainProductFolderName, "{project_id}", PROJECT_ID);
      strMainProductFolderName = ReplaceString(strMainProductFolderName, "{file_class}", m_strFileClass);
      strMainProductFolderName = ReplaceString(strMainProductFolderName, "{file_category}", MAIN_FOLDER_CATEG);
      strMainProductFolderName = ReplaceString(strMainProductFolderName, "{product_level}", m_strProductLevel);
      strMainProductFolderName = ReplaceString(strMainProductFolderName, "{originator_site}", ORIGINATOR_SITE);

      strMainProductFolderName = ReplaceString(strMainProductFolderName, "{creation_date}", strCreationDate);
      strMainProductFolderName = ReplaceString(strMainProductFolderName, "{time_period}", m_strTimePeriod);
      std::string strTileName = strMainProductFolderName;

      strMainProductFolderName = ReplaceString(strMainProductFolderName, "_{tile_id}", "");
      strMainProductFolderName = ReplaceString(strMainProductFolderName, "_{processing_baseline}", "");

      m_strProductFileName = ReplaceString(strMainProductFolderName, "{product_descriptor}", PRODUCT_DESCRIPTOR);

      strMainProductFolderName = ReplaceString(strMainProductFolderName, "_{product_descriptor}", "");
      m_strProductDirectoryName = strMainProductFolderName;

      std::string strMainFolderFullPath = m_strDestRoot + "/" + strMainProductFolderName;

      //created all folders ierarchy
      bool bResult = createsAllFolders(strMainFolderFullPath);

      if(bResult)
      {
          LoadAllDescriptors(descriptors);
      }


      strTileName = ReplaceString(strTileName, MAIN_FOLDER_CATEG, TILE_LEGACY_FOLDER_CATEG);
      strTileName = ReplaceString(strTileName, "{product_descriptor}", TILE_DESCRIPTOR);

      strTileName = ReplaceString(strTileName, "{processing_baseline}", "N" + m_strBaseline);

      if(bResult)
      {
          for (tileInfo &tileEl : m_tileIDList) {
              CreateAndFillTile(tileEl, strMainFolderFullPath, strTileName);
          }

      }

      if(bResult)
      {
          TransferPreviewFiles();
          TransferAndRenameGIPPFiles();
          generateProductMetadataFile(strMainFolderFullPath + "/" + ReplaceString(m_strProductFileName, MAIN_FOLDER_CATEG, METADATA_CATEG) + ".xml");
      }

  }

  std::string m_strFileClass;
  std::string m_strProductLevel;
  std::string m_strTimePeriod;
  std::string m_strDestRoot;
  std::string m_strBaseline;
  std::vector<previewInfo> m_previewList;
  std::vector<std::string> m_GIPPList;
  std::vector<std::string> m_qualityList;
  std::vector<tileInfo> m_tileIDList;

  ProductFileMetadata m_productMetadata;
  std::vector<rasterInfo> m_rasterInfoList;
  std::string m_strProductDirectoryName;

  //bool m_bIsHDR; /* true if is  loaded a .HDR fiel, false if is a .xml file */
  //std::string m_strTileNameRoot;
  std::string m_strProductFileName;
  std::vector<geoProductInfo> m_geoProductInfo;

  void CreateAndFillTile(tileInfo &tileInfoEl, const std::string &strMainFolderFullPath, const std::string &tileNameRoot)
  {
      bool bResult;
      std::string strTileName = tileNameRoot;
      strTileName = ReplaceString(strTileName, "{tile_id}", tileInfoEl.strTileID);

      strTileName = ReplaceString(strTileName, "{processing_baseline}", "N" + m_strBaseline);

      //std::cout << "TileName =" << strTileName << std::endl;

      tileInfoEl.strTilePath = strMainFolderFullPath + "/" + TILES_FOLDER_NAME + "/" +  ReplaceString(strTileName, MAIN_FOLDER_CATEG, TILE_LEGACY_FOLDER_CATEG);

      //std::cout << "TileID =" << tileInfoEl.strTileID << "  strTilePath =" << tileInfoEl.strTilePath  << std::endl;

      bResult = createsAllTileSubfolders(tileInfoEl.strTilePath);


      if(bResult)
      {

          strTileName = ReplaceString(strTileName, "_" + m_strBaseline, "");
          tileInfoEl.strTileNameRoot = strTileName;

          generateTileMetadataFile(tileInfoEl, strTileName);
          TransferRasterFiles(tileInfoEl);

      }

      //create product metadata file
      if(bResult)
      {
          //TransferAndRenameGIPPFiles();
          TransferAndRenameQualityFiles();
          FillProductMetadataForATile(tileInfoEl, strMainFolderFullPath + "/" + ReplaceString(m_strProductFileName, MAIN_FOLDER_CATEG, METADATA_CATEG) + ".xml");

      }

 }

  void UnpackRastersList(std::vector<std::string> &rastersList, rasterTypes rasterType)
  {
      rasterInfo rasterInfoEl;
      std::string strTileID("");
      bool bAlreadyExist = false;

      for (const auto &rasterFileEl : rastersList) {
          if(rasterFileEl.compare(0, 5, "TILE_") == 0)
          {
              //if is TILE separator, read tileID
              strTileID = rasterFileEl.substr(5, rasterFileEl.length() - 5);
              bAlreadyExist = false;
              for (const auto &tileIDEl : m_tileIDList) {
                  if(tileIDEl.strTileID == strTileID)
                  {
                      bAlreadyExist = true;
                      break;
                  }
              }
              if(!bAlreadyExist)
              {
                tileInfo tileInfoEl;
                tileInfoEl.strTileID = strTileID;
                m_tileIDList.emplace_back(tileInfoEl);
              }
          }
          else
          {
              rasterInfoEl.iRasterType = rasterType;
              rasterInfoEl.strRasterFileName = rasterFileEl;
              rasterInfoEl.strTileID = strTileID;
              m_rasterInfoList.emplace_back(rasterInfoEl);
          }

      }
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
          itkExceptionMacro("Fail to create destination directory!");
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
         return VEGETATION_PRODUCT;
      if (m_strProductLevel.compare("L4A") == 0)
         return CROP_MASK;
      if (m_strProductLevel.compare("L4B") == 0)
         return CROP_TYPE;
      return "Unknowkn product";
  }

  void FillBandList()
  {
      Band bandEl;
      if ((m_strProductLevel.compare("L3A") == 0) && (CompositeBandList.empty()))
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

  void generateQuicklook(const std::string &rasterFullFilePath, const std::vector<std::string> &channels,const std::string &jpegFullFilePath)
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
          posix_spawnattr_destroy(&attr);
//          assert(error == 0);
          waitres = waitpid(pid, &status, 0);
//          assert(waitres == pid);
//          assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
  }

  void generateTileMetadataFile(tileInfo &tileInfoEl, const std::string &strTileName)
  {
      TileSize tileSizeEl;
      TileGeoposition tileGeoposition;
      int iResolution;
      bool bResolutionExistingAlready = false;
      bool bGeoPositionExistingAlready = false;
      bool bPreview = !m_previewList.empty();

      auto writer = itk::TileMetadataWriter::New();

      std::string strTile = strTileName;
      strTile = ReplaceString(strTile, "_N" + m_strBaseline, "");
      strTile = ReplaceString(strTile, TILE_LEGACY_FOLDER_CATEG, METADATA_CATEG);

      tileInfoEl.tileMetadata.TileID = strTile;

      for (rasterInfo &rasterFileEl : m_rasterInfoList) {
          if((rasterFileEl.strTileID == tileInfoEl.strTileID) &&
             (rasterFileEl.iRasterType != FLAGS_MASK) &&
             (rasterFileEl.iRasterType != DATES_MASK))
          {

              //std::cout << "ImageFileReader =" << rasterFileEl.strRasterFileName << std::endl;

              auto imageReader = ImageFileReader<FloatVectorImageType>::New();
              imageReader->SetFileName(rasterFileEl.strRasterFileName);
              imageReader->UpdateOutputInformation();
              FloatVectorImageType::Pointer output = imageReader->GetOutput();

              iResolution = output->GetSpacing()[0];

              if((!bPreview) && (((rasterFileEl.iRasterType == REFLECTANCE_RASTER) && (iResolution == 10)) ||
                                (rasterFileEl.iRasterType == LAI_REPR_RASTER) ||
                                (rasterFileEl.iRasterType == CROP_TYPE_RASTER) ||
                                (rasterFileEl.iRasterType == CROP_MASK_RASTER)))
              {
                previewInfo previewInfoEl;
                previewInfoEl.strPreviewFileName = rasterFileEl.strRasterFileName;
                previewInfoEl.strTileID = tileInfoEl.strTileID;
                m_previewList.emplace_back(previewInfoEl);
                bPreview = true;
              }

              if(rasterFileEl.iRasterType != REFLECTANCE_RASTER)
              {
                  //bands no = output->GetNumberOfComponentsPerPixel()
                  Band bandEl;
                  bool bFound;
                  for(int j = 1; j <= output->GetNumberOfComponentsPerPixel(); j++)
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
         if((rasterFileEl.strTileID == tileInfoEl.strTileID) && ((rasterFileEl.iRasterType == FLAGS_MASK) || (rasterFileEl.iRasterType == DATES_MASK)))
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

      geoProductInfo geoPosEl;

      if(!m_geoProductInfo.empty())
      {
          geoPosEl = m_geoProductInfo[0];

          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLon = geoPosEl.AreaOfInterest.LowerCornerLon;
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLat = geoPosEl.AreaOfInterest.LowerCornerLat;
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLon = geoPosEl.AreaOfInterest.UpperCornerLon;
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLat = geoPosEl.AreaOfInterest.UpperCornerLat;

          for (int j = 1; j < m_geoProductInfo.size(); j++) {
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

          for (int i = 1; i < m_geoProductInfo.size(); i++) {

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
          (m_strProductLevel.compare("L3A") == 0) ||
          (m_strProductLevel.compare("L3B") == 0))
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

  void FillProductMetadataForATile(const tileInfo &tileInfoEl, const std::string &strProductMetadataFilePath)
  {

      //fill tiles list
      Granule granuleEl;

      //for the moment is only one tile
      granuleEl.GranuleIdentifier = tileInfoEl.strTileNameRoot;
      granuleEl.ImageFormat = IMAGE_FORMAT;
      //fill the TIFF files for current tile
      for (const auto &rasterFileEl : m_rasterInfoList) {
          if((rasterFileEl.strTileID == tileInfoEl.strTileID) && (rasterFileEl.iRasterType != FLAGS_MASK) & (rasterFileEl.iRasterType != DATES_MASK))
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

      std::string strNewRasterFileName;

      for (auto &rasterFileEl : m_rasterInfoList) {
          strNewRasterFileName = tileInfoEl.strTileNameRoot;
          strNewRasterFileName = ReplaceString(strNewRasterFileName, "_N" + m_strBaseline, "");
          switch(rasterFileEl.iRasterType)
              {
                case REFLECTANCE_RASTER:
                  strNewRasterFileName = strNewRasterFileName + "_" + REFLECTANCE_SUFFIX + "_" + std::to_string(rasterFileEl.nResolution) + TIF_EXTENSION;

                  break;
                case WEIGHTS_RASTER:
                  strNewRasterFileName = strNewRasterFileName + "_" + WEIGHTS_SUFFIX + TIF_EXTENSION;
                  break;
                case LAI_REPR_RASTER:
                   strNewRasterFileName = strNewRasterFileName + "_" + LAI_REPR_SUFFIX + "_" + std::to_string(rasterFileEl.nResolution) + TIF_EXTENSION;
                   break;
                case LAI_FIT_RASTER:
                   strNewRasterFileName = strNewRasterFileName + "_" + LAI_FIT_SUFFIX + "_" + std::to_string(rasterFileEl.nResolution) + TIF_EXTENSION;
                   break;
                case PHENO_RASTER:
                     strNewRasterFileName = strNewRasterFileName + "_" + PHENO_SUFFIX + "_" + std::to_string(rasterFileEl.nResolution) + TIF_EXTENSION;
                     break;
                case CROP_TYPE_RASTER:
                case CROP_MASK_RASTER:
                   strNewRasterFileName= strNewRasterFileName + TIF_EXTENSION;
                   break;
                case DATES_MASK:
                  strNewRasterFileName = ReplaceString(strNewRasterFileName, TILE_LEGACY_FOLDER_CATEG, MASK_CATEG);
                  strNewRasterFileName = strNewRasterFileName + "_" + DATES_SUFFIX + TIF_EXTENSION;
                  break;
                case FLAGS_MASK:
                  strNewRasterFileName = ReplaceString(strNewRasterFileName, TILE_LEGACY_FOLDER_CATEG, MASK_CATEG);
                  strNewRasterFileName = strNewRasterFileName + "_" + FLAGS_SUFFIX + TIF_EXTENSION;
                  break;
              }
              rasterFileEl.strNewRasterFileName = strNewRasterFileName;
      }



  }

  void TransferRasterFiles(const tileInfo &tileInfoEl)
  {

      std::string strImgDataPath;

      for (const auto &rasterFileEl : m_rasterInfoList) {
          if(tileInfoEl.strTileID == rasterFileEl.strTileID)
          {
              switch(rasterFileEl.iRasterType)
              {
                case REFLECTANCE_RASTER:
                //case WEIGHTS_RASTER:
                case LAI_REPR_RASTER:
                case LAI_FIT_RASTER:
                case PHENO_RASTER:
                case CROP_TYPE_RASTER:
                case CROP_MASK_RASTER:
                  strImgDataPath = tileInfoEl.strTilePath + "/" + IMG_DATA_FOLDER_NAME;
                  break;

                case WEIGHTS_RASTER:
                case DATES_MASK:
                case FLAGS_MASK:
                  strImgDataPath = tileInfoEl.strTilePath + "/" + QI_DATA_FOLDER_NAME;
                  break;
              }

               CopyFile(strImgDataPath + "/" + rasterFileEl.strNewRasterFileName, rasterFileEl.strRasterFileName);
          }
        }
   }


  void TransferAndRenameGIPPFiles()
  {
      std::string strNewGIPPFileName;
      GIPPInfo GIPPEl;


      for (const auto &gippFileEl : m_GIPPList) {
          strNewGIPPFileName = m_strProductFileName;
          strNewGIPPFileName = ReplaceString(strNewGIPPFileName, MAIN_FOLDER_CATEG, PARAMETER_CATEG);
          boost::filesystem::path p(gippFileEl);
          if(m_GIPPList.size() > 1)
          {

              strNewGIPPFileName = strNewGIPPFileName + "_" + p.stem().string() + p.extension().string();
          }
          else
          {
              strNewGIPPFileName = strNewGIPPFileName + p.extension().string();
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

  void TransferAndRenameQualityFiles()
  {
      std::string strNewQualityFileName;

      for (const auto &qualityFileEl : m_qualityList) {
          boost::filesystem::path p(qualityFileEl);
          strNewQualityFileName = m_strProductFileName;
          strNewQualityFileName = ReplaceString(strNewQualityFileName, MAIN_FOLDER_CATEG, QUALITY_CATEG);
          strNewQualityFileName = strNewQualityFileName + p.extension().string();

           //quality files are copied to tileDirectory/QI_DATA
          CopyFile(m_strDestRoot + "/" + m_strProductDirectoryName + "/" + AUX_DATA_FOLDER_NAME + "/" + strNewQualityFileName, qualityFileEl);
    }
  }
  void TransferPreviewFiles()
  {
      std::string strProductPreviewFullPath;
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
                //for the moment the preview file for product and tile are the same

                //build product preview file name
                 strProductPreviewFullPath = ReplaceString(m_strProductFileName, MAIN_FOLDER_CATEG, QUICK_L0OK_IMG_CATEG);
                 strProductPreviewFullPath = strProductPreviewFullPath + JPEG_EXTENSION;

                 m_productMetadata.GeneralInfo.ProductInfo.PreviewImageURL = strProductPreviewFullPath;

                 strProductPreviewFullPath = m_strDestRoot + "/" + m_strProductDirectoryName + "/" + strProductPreviewFullPath;

                 //std::cout << "ProductPreviewFullPath = " << strProductPreviewFullPath << std::endl;

                 //transform .tif file in .jpg file directly in product directory
                 generateQuicklook(previewFileEl.strPreviewFileName, strChannelsList, strProductPreviewFullPath);

                //build producty preview file name for tile
                 strTilePreviewFullPath = tileID.strTileNameWithoutExt + JPEG_EXTENSION;
                 strTilePreviewFullPath = ReplaceString(strTilePreviewFullPath, METADATA_CATEG, QUICK_L0OK_IMG_CATEG);

                 //std::cout << "TilePreviewFullPath = " << strTilePreviewFullPath << std::endl;

                 CopyFile(strTilePreviewFullPath, strProductPreviewFullPath);

                 //remove  file with extension jpg.aux.xml generated after preview obtained

                 std::string strFileToBeRemoved = strProductPreviewFullPath + ".aux.xml";
                 //std::cout << "Remove file: " <<  strFileToBeRemoved<< std::endl;
                 remove(strFileToBeRemoved.c_str());
            }
        }
      }
  }

   void CopyFile(const std::string &strDest, const std::string &strSrc)
   {
       std::ifstream  src(strSrc, std::ios::binary);
       std::ofstream  dst(strDest, std::ios::binary);

       dst << src.rdbuf();
   }


  void FillMetadataInfoForLandsat(std::unique_ptr<MACCSFileMetadata> &metadata)
  {
      /* the source is a HDR file */

    SpecialValues specialValue;
    specialValue.SpecialValueText = "NODATA";
    specialValue.SpecialValueIndex = metadata->ImageInformation.NoDataValue;
    m_productMetadata.GeneralInfo.ProductImageCharacteristics.SpecialValuesList.emplace_back(specialValue);

  }

  void FillMetadataInfoForSPOT(std::unique_ptr<SPOT4Metadata> &metadata)
  {
      /* the source is a SPOT file */
      //nothing to load????
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



};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::ProductFormatter)


