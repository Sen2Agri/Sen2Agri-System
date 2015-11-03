#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbOGRIOHelper.h"
#include "ogr_geometry.h"

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


#define MAIN_FOLDER_CATEG "PRD"
#define TILE_LEGACY_FOLDER_CATEG "DAT"
#define TRUE_COLOR_FOLDER_CATEG "TCI"
#define QUICK_L0OK_IMG_CATEG "PVI"
#define METADATA_CATEG "MTD"
#define MASK_CATEG "MSK"
#define PARAMETER_CATEG "IPP"
#define NO_DATA_VALUE "-10000"

#define ORIGINATOR_SITE "CSRO"

#define PRODUCT_DESCRIPTOR "PR"
#define TILE_DESCRIPTOR "TL"
#define LEGACY_DESCRIPTOR "LY"

#define TILES_FOLDER_NAME "TILES"
#define AUX_DATA_FOLDER_NAME "AUX_DATA"
#define LEGACY_DATA_FOLDER_NAME "LEGACY_DATA"
#define IMG_DATA_FOLDER_NAME "IMG_DATA"
#define QI_DATA_FOLDER_NAME "QI_DATA"

#define LANDSAT    "LANDSAT"
#define SENTINEL   "SENTINEL"

#define L2A_PRODUCT "L2A product"
#define COMPOSITE_PRODUCT "Composite"
#define VEGETATION_PRODUCT "Vegetation status"
#define CROP_MASK "Crop mask"
#define CROP_TYPE "Crop type"

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
    CROP_TYPE_RASTER
}rasterTypes;

struct rasterInfo
{
    std::string strRasterFileName;
    int nResolution;
    rasterTypes iRasterType;
    std::string strNewRasterFileName;
};

struct geoProductInfo
{
    rasterTypes rasterType;
    int iResolution;
    std::vector<double>PosList;
    Bbox AreaOfInterest;
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
        AddParameter(ParameterType_InputFilenameList, "processor.composite.refls", "Reflectance raster files list for composite");
        MandatoryOff("processor.composite.refls");

        AddParameter(ParameterType_InputFilenameList, "processor.composite.weights", "Weights raster files list for composite");
        MandatoryOff("processor.composite.weights");

        AddParameter(ParameterType_InputFilenameList, "processor.composite.flags", "Flags mask files list for composite");
        MandatoryOff("processor.composite.flags");

        AddParameter(ParameterType_InputFilenameList, "processor.composite.dates", "Dates mask files list for composite");
        MandatoryOff("processor.composite.dates");

        AddParameter(ParameterType_InputFilenameList, "processor.composite.rgb", "TIFF file to be used to obtain preview for SPOT product");
        MandatoryOff("processor.composite.rgb");
 //vegetation parameters

         AddParameter(ParameterType_InputFilenameList, "processor.vegetation.lairepr", "LAI REPR raster files list for vegetation");
         MandatoryOff("processor.vegetation.lairepr");

         AddParameter(ParameterType_InputFilenameList, "processor.vegetation.laifit", "LAI FIT raster files list for vegetation");
         MandatoryOff("processor.vegetation.laifit");

//crop type parameters
        AddParameter(ParameterType_InputFilenameList, "processor.croptype.file", "CROP TYPE raster file");
        MandatoryOff("processor.croptype.file");

//crop mask parameters
        AddParameter(ParameterType_InputFilenameList, "processor.cropmask.file", "CROP MASK raster file");
        MandatoryOff("processor.cropmask.file");

        AddParameter(ParameterType_InputFilenameList, "il", "The xml files");
        MandatoryOff("il");

        /*AddParameter(ParameterType_InputFilenameList, "preview", "The quicklook files");
        MandatoryOff("preview");*/
        AddParameter(ParameterType_InputFilenameList, "gipp", "The GIPP files");
        MandatoryOff("gipp");

        SetDocExampleParameterValue("destroot", "/home/atrasca/sen2agri/sen2agri-processors-build/Testing/Temporary/Dest");
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
          // Get reflectance raster file list
          std::vector<std::string> rastersList;
          rastersList = this->GetParameterStringList("processor.composite.refls");
          rasterInfo rasterInfoEl;
          //add them in global raster list
          for (const auto &rasterFileEl : rastersList) {
              rasterInfoEl.iRasterType = REFLECTANCE_RASTER;
              rasterInfoEl.strRasterFileName = rasterFileEl;
              m_rasterInfoList.emplace_back(rasterInfoEl);

          }

          rastersList = this->GetParameterStringList("processor.composite.weights");
          for (const auto &rasterFileEl : rastersList) {
              rasterInfoEl.iRasterType = WEIGHTS_RASTER;
              rasterInfoEl.strRasterFileName = rasterFileEl;
              m_rasterInfoList.emplace_back(rasterInfoEl);
          }

          rastersList = this->GetParameterStringList("processor.composite.flags");
          for (const auto &rasterFileEl : rastersList) {
              rasterInfoEl.iRasterType = FLAGS_MASK;
              rasterInfoEl.strRasterFileName = rasterFileEl;
              m_rasterInfoList.emplace_back(rasterInfoEl);
          }

          rastersList = this->GetParameterStringList("processor.composite.dates");
          for (const auto &rasterFileEl : rastersList) {
              rasterInfoEl.iRasterType = DATES_MASK;
              rasterInfoEl.strRasterFileName = rasterFileEl;
              m_rasterInfoList.emplace_back(rasterInfoEl);
          }

          m_previewList = this->GetParameterStringList("processor.composite.rgb");

     }

      //if is vegetation, read LAIREPR and LAIFIT raster files list
      if (m_strProductLevel.compare("L3B") == 0)
      {
          // Get LAIREPR raster file list
          std::vector<std::string> rastersList;
          rastersList = this->GetParameterStringList("processor.vegetation.lairepr");
          rasterInfo rasterInfoEl;
          //add them in global raster list
          for (const auto &rasterFileEl : rastersList) {
              rasterInfoEl.iRasterType = LAI_REPR_RASTER;
              rasterInfoEl.strRasterFileName = rasterFileEl;
              m_rasterInfoList.emplace_back(rasterInfoEl);

          }

          //get LAIFIT raster files list
          rastersList = this->GetParameterStringList("processor.vegetation.laifit");
          for (const auto &rasterFileEl : rastersList) {
              rasterInfoEl.iRasterType = LAI_FIT_RASTER;
              rasterInfoEl.strRasterFileName = rasterFileEl;
              m_rasterInfoList.emplace_back(rasterInfoEl);
          }

     }

      //if is CROP MASK, read raster file
      if (m_strProductLevel.compare("L4A") == 0)
      {

          std::vector<std::string> rastersList;
          rastersList = this->GetParameterStringList("processor.cropmask.file");
          rasterInfo rasterInfoEl;
          //add them in global raster list
          for (const auto &rasterFileEl : rastersList) {
              rasterInfoEl.iRasterType = CROP_MASK_RASTER;
              rasterInfoEl.strRasterFileName = rasterFileEl;
              m_rasterInfoList.emplace_back(rasterInfoEl);

          }
      }

      //if is CROP TYPE, read raster file
      if (m_strProductLevel.compare("L4B") == 0)
      {
          // Get reflectance raster file list
          std::vector<std::string> rastersList;
          rastersList = this->GetParameterStringList("processor.croptype.file");
          rasterInfo rasterInfoEl;
          //add them in global raster list
          for (const auto &rasterFileEl : rastersList) {
              rasterInfoEl.iRasterType = CROP_TYPE_RASTER;
              rasterInfoEl.strRasterFileName = rasterFileEl;
              m_rasterInfoList.emplace_back(rasterInfoEl);

          }
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

      /*strMainProductFolderName = PROJECT_ID;

      strMainProductFolderName +=  "_" + m_strFileClass + "_" + MAIN_FOLDER_CATEG + "_" + m_strProductLevel + "_" + ORIGINATOR_SITE +"_" + strCreationDate + "_V" + m_strTimePeriod;*/

      std::string strMainFolderFullPath = m_strDestRoot + "/" + strMainProductFolderName;

      //created all folders ierarchy
      bool bResult = createsAllFolders(strMainFolderFullPath);

      if(bResult)
      {
          LoadAllDescriptors(descriptors);
      }


      //
      if(bResult)
      {
          strTileName = ReplaceString(strTileName, MAIN_FOLDER_CATEG, TILE_LEGACY_FOLDER_CATEG);
          strTileName = ReplaceString(strTileName, "{product_descriptor}", TILE_DESCRIPTOR);
          //?????? TODO tile_id
          strTileName = ReplaceString(strTileName, "{tile_id}", TILE_ID);

          strTileName = ReplaceString(strTileName, "{processing_baseline}", "N" + m_strBaseline);
          m_strTilePath = strMainFolderFullPath + "/" + TILES_FOLDER_NAME + "/" +  ReplaceString(strTileName, MAIN_FOLDER_CATEG, TILE_LEGACY_FOLDER_CATEG);
          bResult = createsAllTileSubfolders(m_strTilePath);
          if(bResult)
          {

              strTileName = ReplaceString(strTileName, "_" + m_strBaseline, "");
              m_strTileNameRoot = strTileName;

              generateTileMetadataFile(strTileName);
              TransferRasterFiles();
              TransferPreviewFiles();

          }

          //create product metadata file
          if(bResult)
          {
              TransferAndRenameGIPPFiles();
              generateProductMetadataFile(strMainFolderFullPath + "/" + ReplaceString(m_strProductFileName, MAIN_FOLDER_CATEG, METADATA_CATEG) + ".xml");

          }

      }

  }

  std::string m_strFileClass;
  std::string m_strProductLevel;
  std::string m_strTimePeriod;
  std::string m_strDestRoot;
  std::string m_strBaseline;
  std::vector<std::string> m_previewList;
  std::vector<std::string> m_GIPPList;
  std::string m_strTilePath;
  ProductFileMetadata m_productMetadata;
  TileFileMetadata m_tileMetadata;
  std::vector<rasterInfo> m_rasterInfoList;
  std::string m_strTileNameWithoutExt;
  std::string m_strProductDirectoryName;

  bool m_bIsHDR; /* true if is  loaded a .HDR fiel, false if is a .xml file */
  std::string m_strTileNameRoot;
  std::string m_strProductFileName;
  std::vector<geoProductInfo> m_geoProductInfo;

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
                        OGRPoint &pt,
                        OGRRawPoint &ptOut)
  {
      pt.setX(v[0]);
      pt.setY(v[1]);
      pt.setZ(0.0);

      auto err = pt.transform(transform);

      ptOut.x = pt.getX();
      ptOut.y = pt.getY();

      return err;
  }

  template <typename TImage>
  std::vector<OGRRawPoint> GetExtent(const TImage *image)
  {
      std::vector<OGRRawPoint> extent;

      auto *sourceSRS = static_cast<OGRSpatialReference *>(
                           OSRNewSpatialReference(image->GetProjectionRef().c_str()));

      auto targetSRS = static_cast<OGRSpatialReference *>(OSRNewSpatialReference(SRS_WKT_WGS84));

      if (!targetSRS) {
          return extent;
      }
      auto transform = static_cast<OGRCoordinateTransformation *>(
          OCTNewCoordinateTransformation(sourceSRS, targetSRS));
      if (!transform) {
          return extent;
      }

      auto ok = true;
      auto point = static_cast<OGRPoint *>(OGRGeometryFactory::createGeometry(wkbPoint));
      OGRRawPoint pt;
      if (TransformPoint(image->GetUpperLeftCorner(), transform, *point, pt) == OGRERR_NONE) {
          extent.emplace_back(pt);
      } else {
          ok = false;
      }
      if (ok &&
          TransformPoint(image->GetLowerLeftCorner(), transform, *point, pt) == OGRERR_NONE) {
          extent.emplace_back(pt);
      } else {
          ok = false;
      }
      if (ok &&
          TransformPoint(image->GetLowerRightCorner(), transform, *point, pt) == OGRERR_NONE) {
          extent.emplace_back(pt);
      } else {
          ok = false;
      }
      if (ok &&
          TransformPoint(image->GetUpperRightCorner(), transform, *point, pt) == OGRERR_NONE) {
          extent.emplace_back(pt);
      } else {
          ok = false;
      }

      if (!ok) {
          extent.clear();
      }

      OGRGeometryFactory::destroyGeometry(point);

      OCTDestroyCoordinateTransformation(transform);
      OSRDestroySpatialReference(targetSRS);
      OSRDestroySpatialReference(sourceSRS);

      return extent;
  }

  void generateQuicklook(const std::string &rasterFullFilePath, const std::vector<std::string> &channels,const std::string &jpegFullFilePath)
  {
      int error, status;
          pid_t pid, waitres;
//          const char *myargs[14];
//          char *myenv[2] = { "ITK_AUTOLOanswer=42", NULL };

//          /* Make sure we have no child processes. */
//          while (waitpid(-1, NULL, 0) != -1)
//              ;
//          assert(errno == ECHILD);

          std::vector<const char *> args;
          args.emplace_back("otbApplicationLauncherCommandLine");
          args.emplace_back("Quicklook");
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



          error = posix_spawnp(&pid, args[0], NULL, NULL, (char *const *)args.data(), environ);
//          assert(error == 0);
          waitres = waitpid(pid, &status, 0);
//          assert(waitres == pid);
//          assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
  }

  void generateTileMetadataFile(const std::string &strTileName)
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

      m_tileMetadata.TileID = strTile;


      for (rasterInfo &rasterFileEl : m_rasterInfoList) {
          if((rasterFileEl.iRasterType != FLAGS_MASK) &&
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
                m_previewList.emplace_back(rasterFileEl.strRasterFileName);
                bPreview = true;
              }

              if(rasterFileEl.iRasterType != REFLECTANCE_RASTER)
              {
                  //get bands no
                  int iBands = output->GetNumberOfComponentsPerPixel();
                  Band bandEl;
                  for(int j = 1; j <= iBands; j++)
                  {
                      bandEl.Resolution = iResolution;
                      bandEl.BandName = "B" + std::to_string(j);
                      m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.BandList.emplace_back(bandEl);
                  }

              }

              rasterFileEl.nResolution = iResolution;
              //search the current resolution to seed if is already added in tileEl
              for (const auto &tileEl : m_tileMetadata.TileGeometricInfo.TileSizeList) {
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
                  m_tileMetadata.TileGeometricInfo.TileSizeList.emplace_back(tileSizeEl);
              }

              tileGeoposition.resolution = iResolution;

              for (const auto &geoPosEl : m_tileMetadata.TileGeometricInfo.TileGeopositionList) {
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
                  m_tileMetadata.TileGeometricInfo.TileGeopositionList.emplace_back(tileGeoposition);
              }

              if (auto oSRS = static_cast<OGRSpatialReference *>(OSRNewSpatialReference(output->GetProjectionRef().c_str()))) {
                  m_productMetadata.GeometricInfo.CoordReferenceSystem.HorizCSName = oSRS->GetAttrValue("PROJCS");
                  m_productMetadata.GeometricInfo.CoordReferenceSystem.HorizCSCode = std::string(oSRS->GetAuthorityName("PROJCS")) + ':' + oSRS->GetAuthorityCode("PROJCS");

                  m_tileMetadata.TileGeometricInfo.HorizontalCSName = m_productMetadata.GeometricInfo.CoordReferenceSystem.HorizCSName;
                  m_tileMetadata.TileGeometricInfo.HorizontalCSCode = m_productMetadata.GeometricInfo.CoordReferenceSystem.HorizCSCode;

                  OSRDestroySpatialReference(oSRS);
              }

              geoProductInfo geoProductInfoEl;
              geoProductInfoEl.iResolution = iResolution;
              geoProductInfoEl.rasterType = rasterFileEl.iRasterType;

              const auto &extent = GetExtent(output.GetPointer());
              if (extent.size()) {
                  for (const auto &p : extent) {
                      //std::cerr << p.x << ' ' << p.y << std::endl;
                      geoProductInfoEl.PosList.emplace_back(p.x);
                      geoProductInfoEl.PosList.emplace_back(p.y);
                  }
                  //std::cerr << "----\n";
                  //add again first point coordinates
                  geoProductInfoEl.PosList.emplace_back(extent[0].x);
                  geoProductInfoEl.PosList.emplace_back(extent[0].y);
              }


              geoProductInfoEl.AreaOfInterest.LowerCornerLon = extent[1].x;
              geoProductInfoEl.AreaOfInterest.LowerCornerLat = extent[1].y;
              geoProductInfoEl.AreaOfInterest.UpperCornerLon = extent[3].x;
              geoProductInfoEl.AreaOfInterest.UpperCornerLat = extent[3].y;
              m_geoProductInfo.emplace_back(geoProductInfoEl);

          }
      }

      ComputeNewNameOfRasterFiles();

      m_tileMetadata.TileThematicInfo = "";
      m_tileMetadata.TileImageContentQI.NoDataPixelPercentange = "";
      m_tileMetadata.TileImageContentQI.SaturatedDefectivePixelPercentange = "";
      m_tileMetadata.TileImageContentQI.CloudShadowPercentange = "";
      m_tileMetadata.TileImageContentQI.VegetationPercentange = "";
      m_tileMetadata.TileImageContentQI.WaterPercentange = "";
      m_tileMetadata.TileImageContentQI.LowProbaCloudsPercentange = "";
      m_tileMetadata.TileImageContentQI.MediumProbaCloudsPercentange = "";
      m_tileMetadata.TileImageContentQI.HighProbaCloudsPercentange = "";
      m_tileMetadata.TileImageContentQI.ThinCirrusPercentange = "";
      m_tileMetadata.TileImageContentQI.SnowIcePercentange = "";

      TileMask tileMask;
      for (const auto &rasterFileEl : m_rasterInfoList) {
         if((rasterFileEl.iRasterType == FLAGS_MASK) || (rasterFileEl.iRasterType == DATES_MASK))
         {
              tileMask.MaskType = "";// ??? TODO
              tileMask.BandId = 0;
              tileMask.Geometry = "FULL_RESOLUTION";
              tileMask.MaskFileName = rasterFileEl.strNewRasterFileName;
              m_tileMetadata.TileMasksList.emplace_back(tileMask);
          }
      }

      m_strTileNameWithoutExt = m_strTilePath + "/" + strTile;
      writer->WriteTileMetadata(m_tileMetadata, m_strTilePath + "/" + strTile + ".xml");
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

      int iGoodGeoPos = 0;
      geoProductInfo geoPosEl;
      for (int i = 0; i < m_geoProductInfo.size(); i++) {

          if((geoPosEl.rasterType == REFLECTANCE_RASTER) && (geoPosEl.iResolution == 10))
          {
              iGoodGeoPos = i;
              break;
          }
      }
      if(!m_geoProductInfo.empty())
      {
          geoPosEl = m_geoProductInfo[iGoodGeoPos];
          for (const auto &posPoint : geoPosEl.PosList) {
            m_productMetadata.GeometricInfo.ProductFootprint.ExtPosList.emplace_back(posPoint);
          }
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLon = geoPosEl.AreaOfInterest.LowerCornerLon;
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.LowerCornerLat = geoPosEl.AreaOfInterest.LowerCornerLat;
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLon = geoPosEl.AreaOfInterest.UpperCornerLon;
          m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AreaOfInterest.UpperCornerLat = geoPosEl.AreaOfInterest.UpperCornerLat;
      }

      FillBandList();

      //TODO ????
      m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.MetadataLevel = "SuperBrief";
      m_productMetadata.GeneralInfo.ProductInfo.QueryOptions.AuxListContent.ProductLevel = "Level-3A";


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



      //fill tiles list
      Granule granuleEl;

      //for the moment is only one tile
      granuleEl.GranuleIdentifier = m_strTileNameRoot;//m_strTilePath;
      granuleEl.ImageFormat = IMAGE_FORMAT;
      //fill the TIFF files for current tile
      for (const auto &rasterFileEl : m_rasterInfoList) {
          if((rasterFileEl.iRasterType != FLAGS_MASK) & (rasterFileEl.iRasterType != DATES_MASK))
          {
            granuleEl.ImageIDList.emplace_back(rasterFileEl.strNewRasterFileName);
          }
      }
      m_productMetadata.GeneralInfo.ProductInfo.ProductOrganisation.emplace_back(granuleEl);

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

      //no reports for now
      GranuleReport granuleReport;
      granuleReport.GranuleReportId = "";
      granuleReport.GranuleReportFileName = "";
      m_productMetadata.QualityIndicatorsInfo.QualityControlChecks.FailedInspections.emplace_back(granuleReport);

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

  void ComputeNewNameOfRasterFiles()
  {

      std::string strNewRasterFileName;

      for (auto &rasterFileEl : m_rasterInfoList) {
          strNewRasterFileName = m_strTileNameRoot;
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

  void TransferRasterFiles()
  {

      std::string strImgDataPath;

      for (const auto &rasterFileEl : m_rasterInfoList) {
              switch(rasterFileEl.iRasterType)
              {
                case REFLECTANCE_RASTER:
                case WEIGHTS_RASTER:
                case LAI_REPR_RASTER:
                case LAI_FIT_RASTER:
                case CROP_TYPE_RASTER:
                case CROP_MASK_RASTER:
                  strImgDataPath = m_strTilePath + "/" + IMG_DATA_FOLDER_NAME;
                  break;

                case DATES_MASK:
                  strImgDataPath = m_strTilePath + "/" + QI_DATA_FOLDER_NAME;
                  break;
                case FLAGS_MASK:
                  strImgDataPath = m_strTilePath + "/" + QI_DATA_FOLDER_NAME;
                  break;
              }

               CopyFile(strImgDataPath + "/" + rasterFileEl.strNewRasterFileName, rasterFileEl.strRasterFileName);
          }

   }

  /* Extract the extension of a file */
   std::string extractExtension(const std::string& filename) {
        size_t pos = filename.find_last_of(".");
        if (pos == std::string::npos) {
            return "";
        }

        return filename.substr(pos + 1, filename.length());
   }


  void TransferAndRenameGIPPFiles()
  {
      std::string strNewGIPPFileName;
      GIPPInfo GIPPEl;

      for (const auto &gippFileEl : m_GIPPList) {
          strNewGIPPFileName = m_strProductFileName;
          strNewGIPPFileName = ReplaceString(strNewGIPPFileName, MAIN_FOLDER_CATEG, PARAMETER_CATEG);
          strNewGIPPFileName = strNewGIPPFileName + "." + extractExtension(gippFileEl);

          GIPPEl.GIPPFileName = strNewGIPPFileName;
          GIPPEl.GIPPType = "";
          GIPPEl.GIPPVersion = GIPP_VERSION;
          m_productMetadata.AuxiliaryDataInfo.GIPPList.emplace_back(GIPPEl);

           //rasters file are copied to tileDirectory/IMG_DATA or QI_DATA
          CopyFile(m_strDestRoot + "/" + m_strProductDirectoryName + "/" + AUX_DATA_FOLDER_NAME + "/" + strNewGIPPFileName, gippFileEl);
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

      std::cout << "ChannelNo = " << iChannelNo << std::endl;

      for(int j = 1; j <= iChannelNo; j++)
      {
        strChannelsList.emplace_back("Channel" + std::to_string(j));
      }

      for (const auto &previewFileEl : m_previewList) {

            //for the moment the preview file for product and tile are the same

            //build product preview file name
             strProductPreviewFullPath = ReplaceString(m_strProductFileName, MAIN_FOLDER_CATEG, QUICK_L0OK_IMG_CATEG);
             strProductPreviewFullPath = strProductPreviewFullPath + JPEG_EXTENSION;

             m_productMetadata.GeneralInfo.ProductInfo.PreviewImageURL = strProductPreviewFullPath;

             strProductPreviewFullPath = m_strDestRoot + "/" + m_strProductDirectoryName + "/" + strProductPreviewFullPath;

             std::cout << "ProductPreviewFullPath = " << strProductPreviewFullPath << std::endl;

             //transform .tif file in .jpg file directly in product directory
             generateQuicklook(previewFileEl, strChannelsList, strProductPreviewFullPath);

            //build producty preview file name for tile
             strTilePreviewFullPath = m_strTileNameWithoutExt + JPEG_EXTENSION;
             strTilePreviewFullPath = ReplaceString(strTilePreviewFullPath, METADATA_CATEG, QUICK_L0OK_IMG_CATEG);

             std::cout << "TilePreviewFullPath = " << strTilePreviewFullPath << std::endl;

             CopyFile(strTilePreviewFullPath, strProductPreviewFullPath);

             //remove  file with extension jpg.aux.xml generated after preview obtained

             std::string strFileToBeRemoved = strProductPreviewFullPath + ".aux.xml";
             std::cout << "Remove file: " <<  strFileToBeRemoved<< std::endl;
             remove(strFileToBeRemoved.c_str());

    }
  }

   void CopyFile(const std::string &strDest, const std::string &strSrc)
   {
       std::ifstream  src(strSrc, std::ios::binary);
       std::ofstream  dst(strDest, std::ios::binary);

       dst << src.rdbuf();
   }

  /* Extract the folder from a given path.*/
   std::string extractFolder(const std::string& filename) {
        size_t pos = filename.find_last_of("/\\");
        if (pos == std::string::npos) {
            return "";
        }

        return filename.substr(0, pos) + "/";
    }

  void FillMetadataInfoForLandsat(std::unique_ptr<MACCSFileMetadata> &metadata)
  {
      /* the source is a HDR file */

    //SpecialValues specialValue;
    //specialValue.SpecialValueText = "NODATA";
    //specialValue.SpecialValueIndex = metadata->ImageInformation.NoDataValue;
    //m_productMetadata.GeneralInfo.ProductImageCharacteristics.SpecialValuesList.emplace_back(specialValue);

    //???? TODO
    //specialValue.SpecialValueIndex = "1";
    //specialValue.SpecialValueText = "NOTVALID";
    //m_productMetadata.GeneralInfo.ProductImageCharacteristics.SpecialValuesList.emplace_back(specialValue);

    //m_productMetadata.QualityIndicatorsInfo.CloudCoverage = "";
  }

  void FillMetadataInfoForSPOT(std::unique_ptr<SPOT4Metadata> &metadata)
  {
      /* the source is a SPOT file */
      //????? TODO
      SpecialValues specialValue;
      specialValue.SpecialValueIndex = "0";
      specialValue.SpecialValueText = "NODATA";
      m_productMetadata.GeneralInfo.ProductImageCharacteristics.SpecialValuesList.emplace_back(specialValue);

      //????NOT FOUND
      m_productMetadata.QualityIndicatorsInfo.CloudCoverage = "";
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
                  m_bIsHDR = true;
                  FillMetadataInfoForLandsat(meta);
              } else if (meta->Header.FixedHeader.Mission.find(SENTINEL) != std::string::npos) {
                  // Interpret sentinel product
                  m_bIsHDR  = true;
                  FillMetadataInfoForLandsat(meta);
              } else {
                  itkExceptionMacro("Unknown mission: " + meta->Header.FixedHeader.Mission);
              }

          }else if (auto meta = spot4MetadataReader->ReadMetadata(desc)) {

              m_bIsHDR = false;
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


