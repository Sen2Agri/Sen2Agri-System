/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


//  Software Guide : BeginCommandLineArgs
//    INPUTS: {reference polygons}, {sample ratio}
//    OUTPUTS: {training polygons}, {validation_polygons}
//  Software Guide : EndCommandLineArgs


//  Software Guide : BeginLatex
// The sample selection consists in splitting the reference data into 2 disjoint sets, the training set and the validation set.
// These sets are composed of polygons, not individual pixels.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"


#include "otbVectorImage.h"
#include "otbImageList.h"
#include "otbImageListToVectorImageFilter.h"

#include "otbStreamingResampleImageFilter.h"
#include "otbGridResampleImageFilter.h"
#include "otbStreamingStatisticsVectorImageFilter.h"
#include "otbLabelImageToVectorDataFilter.h"

//Transform
#include "itkScalableAffineTransform.h"
#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

#include "otbOGRIOHelper.h"


#include "otbStatisticsXMLFileWriter.h"

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"
#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"

// Filters
#include "otbMultiChannelExtractROI.h"
#include "otbConcatenateVectorImagesFilter.h"
#include "../Filters/otbCropTypeFeatureExtractionFilter.h"
#include "../Filters/otbTemporalResamplingFilter.h"
#include "../Filters/otbTemporalMergingFilter.h"

#include "../Filters/otbSpotMaskFilter.h"
#include "../Filters/otbLandsatMaskFilter.h"
#include "../Filters/otbSentinelMaskFilter.h"


#define LANDSAT    "LANDSAT"
#define SENTINEL   "SENTINEL"
#define SPOT       "SPOT"

#define TYPE_MACCS  0
#define TYPE_SPOT4  1

#define MASK_TYPE_NUA   0
#define MASK_TYPE_SAT   1
#define MASK_TYPE_DIV   2

#define EPSILON     1e-6


typedef otb::VectorImage<float, 2>                                 ImageType;

typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;

typedef itk::MACCSMetadataReader                                   MACCSMetadataReaderType;
typedef itk::SPOT4MetadataReader                                   SPOT4MetadataReaderType;

//typedef otb::StreamingResampleImageFilter<ImageType, ImageType, double>    ResampleFilterType;
typedef otb::GridResampleImageFilter<ImageType, ImageType, double>    ResampleFilterType;
typedef otb::ObjectList<ResampleFilterType>                           ResampleFilterListType;
typedef otb::BCOInterpolateImageFunction<ImageType,
                                            double>          BicubicInterpolationType;
typedef itk::LinearInterpolateImageFunction<ImageType,
                                            double>          LinearInterpolationType;
typedef itk::NearestNeighborInterpolateImageFunction<ImageType, double>             NearestNeighborInterpolationType;

//typedef itk::IdentityTransform<double, ImageType::ImageDimension>      IdentityTransformType;
typedef itk::ScalableAffineTransform<double, ImageType::ImageDimension> ScalableTransformType;
typedef ScalableTransformType::OutputVectorType                         OutputVectorType;

typedef otb::SpotMaskFilter<ImageType>                      SpotMaskFilterType;
typedef otb::ObjectList<SpotMaskFilterType>                 SpotMaskFilterListType;

typedef otb::LandsatMaskFilter<ImageType>                   LandsatMaskFilterType;
typedef otb::ObjectList<LandsatMaskFilterType>              LandsatMaskFilterListType;

typedef otb::SentinelMaskFilter<ImageType>                  SentinelMaskFilterType;
typedef otb::ObjectList<SentinelMaskFilterType>             SentinelMaskFilterListType;

typedef otb::MultiChannelExtractROI<float, float>
                                                            MultiChannelExtractROIType;
typedef otb::ObjectList<MultiChannelExtractROIType>         MultiChannelExtractROIListType;

typedef otb::ConcatenateVectorImagesFilter<ImageType>       ConcatenateVectorImagesFilterType;
typedef otb::ObjectList<ConcatenateVectorImagesFilterType>  ConcatenateVectorImagesFilterListType;

typedef otb::TemporalResamplingFilter<ImageType>            TemporalResamplingFilterType;
typedef otb::ObjectList<TemporalResamplingFilterType>       TemporalResamplingFilterListType;

typedef otb::CropTypeFeatureExtractionFilter<ImageType>     CropTypeFeatureExtractionFilterType;
typedef otb::ObjectList<CropTypeFeatureExtractionFilterType>
                                                            CropTypeFeatureExtractionFilterListType;

typedef otb::StreamingStatisticsVectorImageFilter<ImageType>
                                                            StreamingStatisticsVImageFilterType;
typedef otb::ObjectList<StreamingStatisticsVImageFilterType>
                                                            StreamingStatisticsVImageFilterListType;



struct SourceImageMetadata {
    MACCSFileMetadata  msccsFileMetadata;
    SPOT4Metadata      spot4Metadata;
};

// Data related to the size and location of each tile
struct TileData {
    double                                m_imageWidth;
    double                                m_imageHeight;
    ImageType::PointType                  m_imageOrigin;
};

struct ImageDescriptor {
    // the descriptor file
    std::string filename;
    // the aquisition date in format YYYYMMDD
    std::string aquisitionDate;
    // the mission name
    std::string mission;
    // this product belongs to the main mission
    bool isMain;

    // The four required bands (Green, Red, NIR, SWIR) resampled to match the tile area
    ImageType::Pointer          bands;
    // The combined mask
    ImageType::Pointer          mask;

    // the format of the metadata
    unsigned char type;
    // the metadata
    SourceImageMetadata metadata;
};

// define shortcut accessors
#define maccsDescriptor   metadata.msccsFileMetadata
#define spot4Descriptor   metadata.spot4Metadata

//  Software Guide : EndCodeSnippet

namespace otb
{

//  Software Guide : BeginLatex
//  Application class is defined in Wrapper namespace.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
namespace Wrapper
{
//  Software Guide : EndCodeSnippet


//  Software Guide : BeginLatex
//
//  SampleSelection class is derived from Application class.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
class CropTypeStatistics : public Application
//  Software Guide : EndCodeSnippet
{
public:
  //  Software Guide : BeginLatex
  // The \code{ITK} public types for the class, the superclass and smart pointers.
  // Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  typedef CropTypeStatistics Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
  // Software Guide : EndCodeSnippet

  //  Software Guide : BeginLatex
  //  Invoke the macros necessary to respect ITK object factory mechanisms.
  //  Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  itkNewMacro(Self)
;

  itkTypeMacro(CropTypeStatistics, otb::Application)
;
  //  Software Guide : EndCodeSnippet


private:

  //  Software Guide : BeginLatex
  //  \code{DoInit()} method contains class information and description, parameter set up, and example values.
  //  Software Guide : EndLatex

  void DoInit()
  {

    // Software Guide : BeginLatex
    // Application name and description are set using following methods :
    // \begin{description}
    // \item[\code{SetName()}] Name of the application.
    // \item[\code{SetDescription()}] Set the short description of the class.
    // \item[\code{SetDocName()}] Set long name of the application (that can be displayed \dots).
    // \item[\code{SetDocLongDescription()}] This methods is used to describe the class.
    // \item[\code{SetDocLimitations()}] Set known limitations (threading, invalid pixel type \dots) or bugs.
    // \item[\code{SetDocAuthors()}] Set the application Authors. Author List. Format : "John Doe, Winnie the Pooh" \dots
    // \item[\code{SetDocSeeAlso()}] If the application is related to one another, it can be mentioned.
    // \end{description}
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
      SetName("CropTypeStatistics");
      SetDescription("Build the statistics from a set of tiles");

      SetDocName("CropTypeStatistics");
      SetDocLongDescription("Build the statistics from a set of tiles.");
      SetDocLimitations("None");
      SetDocAuthors("LBU");
      SetDocSeeAlso(" ");
    //  Software Guide : EndCodeSnippet


    // Software Guide : BeginLatex
    // \code{AddDocTag()} method categorize the application using relevant tags.
    // \code{Code/ApplicationEngine/otbWrapperTags.h} contains some predefined tags defined in \code{Tags} namespace.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    AddDocTag(Tags::Vector);
    //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // The input parameters:
    // - ref: Vector file containing reference data
    // - ratio: Ratio between the number of training and validation polygons per class (dafault: 0.75)
    // The output parameters:
    // - tp: Vector file containing reference data for training
    // - vp: Vector file containing reference data for validation
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    AddParameter(ParameterType_InputFilenameList, "il", "Input descriptors");
    SetParameterDescription( "il", "The list of descriptors. They must be sorted by tiles." );

    AddParameter(ParameterType_OutputFilename, "out", "Output XML file");
    SetParameterDescription( "out", "XML filename where the statistics are saved for future reuse." );

    AddParameter(ParameterType_StringList, "prodpertile", "Products per tile");
    SetParameterDescription("prodpertile", "The number of products corresponding to each tile");
    MandatoryOff("prodpertile");

    AddParameter(ParameterType_Float, "bv", "Background Value");
    SetParameterDescription( "bv", "Background value to ignore in statistics computation." );
    MandatoryOff("bv");

    AddParameter(ParameterType_StringList, "sp", "Temporal sampling rate");

    AddParameter(ParameterType_Choice, "mode", "Mode");
    SetParameterDescription("mode", "Specifies the choice of output dates (default: resample)");
    AddChoice("mode.resample", "Specifies the temporal resampling mode");
    AddChoice("mode.gapfill", "Specifies the gapfilling mode");
    SetParameterString("mode", "resample");

    AddParameter(ParameterType_Float, "pixsize", "The size of a pixel, in meters");
    SetDefaultParameterFloat("pixsize", 10.0); // The default value is 10 meters
    SetMinimumParameterFloatValue("pixsize", 1.0);
    MandatoryOff("pixsize");

    AddParameter(ParameterType_String, "mission", "The main raster series that will be used. By default SPOT is used");
    MandatoryOff("mission");

     //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
    // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("il", "image1.xml image2.xml");
    SetDocExampleParameterValue("mode", "resample");
    SetDocExampleParameterValue("sp", "SENTINEL 10 LANDSAT 7");
    SetDocExampleParameterValue("out", "statistics.xml");
    //  Software Guide : EndCodeSnippet
  }

  // Software Guide : BeginLatex
  // \code{DoUpdateParameters()} is called as soon as a parameter value change. Section \ref{sec:appDoUpdateParameters}
  // gives a complete description of this method.
  // Software Guide : EndLatex
  //  Software Guide :BeginCodeSnippet
  void DoUpdateParameters()
  {
      // Reinitialize the object

      m_ResamplersList = ResampleFilterListType::New();
      m_ImageReaderList = ImageReaderListType::New();


      m_SpotMaskFilters = SpotMaskFilterListType::New();
      m_LandsatMaskFilters = LandsatMaskFilterListType::New();
      m_SentinelMaskFilters = SentinelMaskFilterListType::New();

      m_ChannelExtractors = MultiChannelExtractROIListType::New();

      m_ImageMergers = ConcatenateVectorImagesFilterListType::New();
      m_TempResamplers = TemporalResamplingFilterListType::New();
      m_FeatureExtrators = CropTypeFeatureExtractionFilterListType::New();
      m_Statistics = StreamingStatisticsVImageFilterListType::New();


  }
  //  Software Guide : EndCodeSnippet

  // Software Guide : BeginLatex
  // The algorithm consists in a random sampling without replacement of the polygons of each class with
  // probability p = sample_ratio value for the training set and
  // 1 - p for the validation set.
  // Software Guide : EndLatex
  //  Software Guide :BeginCodeSnippet
  void DoExecute()
  {
      // get the required pixel size
      m_pixSize = this->GetParameterFloat("pixsize");

      // get the main mission
      m_mission = SPOT;
      if (HasValue("mission")) {
          m_mission = this->GetParameterString("mission");
      }

      bool resample = GetParameterString("mode") == "resample";
      std::map<std::string, int> sp;
      if (HasValue("sp")) {
          const auto &spValues = GetParameterStringList("sp");
          auto n = spValues.size();
          if (n % 2) {
              itkExceptionMacro("Parameter 'sp' must be a list of string and number pairs.");
          }

          for (size_t i = 0; i < n; i += 2) {
              const auto sensor = spValues[i];
              const auto rateStr = spValues[i + 1];
              auto rate = std::stoi(rateStr);
              if (rate <= 0) {
                  itkExceptionMacro("Invalid sampling rate " << rateStr << " for sensor " << sensor)
              }
              sp[sensor] = rate;
          }

          std::cout << "Sampling rates by sensor:\n";
          for (const auto &sensor : sp) {
              std::cout << sensor.first << ": " << sensor.second << '\n';
          }
      }

      // Get the list of input files
      std::vector<std::string> descriptors = this->GetParameterStringList("il");
      if( descriptors.size()== 0 )
        {
        itkExceptionMacro("No input file set...");
        }

      // Get the number of products per tile
      std::vector<int> prodPerTile;
      unsigned int numDesc = 0;
      if (HasValue("prodpertile")) {
          const auto &prodPerTileStrings = GetParameterStringList("prodpertile");
          auto n = prodPerTileStrings.size();

          for (size_t i = 0; i < n; i ++) {
              const auto ppts = prodPerTileStrings[i];
              auto ppt = std::stoi(ppts);
              if (ppt <= 0) {
                  itkExceptionMacro("Invalid number of produts " << ppts)
              }
              prodPerTile.push_back(ppt);
              numDesc += ppt;
          }
      } else {
          // All products belong to the same tile
          prodPerTile.push_back(descriptors.size());
          numDesc = descriptors.size();
      }

      if( descriptors.size() != numDesc )
        {
        itkExceptionMacro("The number of descriptors (" << descriptors.size() << ") is not consistent with the sum of products per tile (" << numDesc << ")")
        }

      typedef std::vector<ImageDescriptor>      ImageDescriptorList;
      std::vector<ImageDescriptorList>         fullDescriptorsList;
      std::map<std::string, std::vector<int> > sensorInDays;
      std::map<std::string, std::vector<int> > sensorOutDays;

      // Loop through the sets of products
      int startIndex = 0;
      int endIndex;
      for (size_t i = 0; i < prodPerTile.size(); i ++) {
          int sz = prodPerTile[i];
          endIndex = startIndex + sz;
          TileData td;
          ImageDescriptorList descriptorsList;


          // compute the desired size of the processed rasters
          updateRequiredImageSize(descriptors, startIndex, endIndex, td);

          // loop through the descriptors
          MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
          SPOT4MetadataReaderType::Pointer spot4MetadataReader = SPOT4MetadataReaderType::New();
          for (const std::string& desc : descriptors) {
              ImageDescriptor id;
              if (auto meta = maccsMetadataReader->ReadMetadata(desc)) {
                  // add the information to the list
                  if (meta->Header.FixedHeader.Mission.find(LANDSAT) != std::string::npos) {
                      // Interpret landsat product
                      ProcessLandsat8Metadata(*meta, desc, id, td);
                  } else if (meta->Header.FixedHeader.Mission.find(SENTINEL) != std::string::npos) {
                      // Interpret sentinel product
                      ProcessSentinel2Metadata(*meta, desc, id, td);
                  } else {
                      itkExceptionMacro("Unknown mission: " + meta->Header.FixedHeader.Mission);
                  }

                  descriptorsList.push_back(id);
              }else if (auto meta = spot4MetadataReader->ReadMetadata(desc)) {
                  // add the information to the list
                  ProcessSpot4Metadata(*meta, desc, id, td);

                  descriptorsList.push_back(id);
              } else {
                  itkExceptionMacro("Unable to read metadata from " << desc);
              }
          }

          // sort the descriptors after the aquisition date
          std::sort(descriptorsList.begin(), descriptorsList.end(), CropTypeStatistics::SortUnmergedMetadata);

          for (const ImageDescriptor& id : descriptorsList) {
              int inDay = getDaysFromEpoch(id.aquisitionDate);
              std::map<std::string, std::vector<int> >::iterator it = sensorInDays.find(id.mission);

              if (it == sensorInDays.end()) {
                  std::vector<int> days;
                  days.push_back(inDay);
                  sensorInDays[id.mission] = days;
              } else {
                  if (std::find(it->second.begin(), it->second.end(), inDay) == it->second.end()) {
                    it->second.push_back(inDay);
                    std::sort(it->second.begin(), it->second.end());
                  }
              }

          }


          fullDescriptorsList.push_back(descriptorsList);
      }

      // Samples
      typedef double ValueType;
      typedef itk::VariableLengthVector<ValueType> MeasurementType;

      unsigned int nbSamples = 0;

      // Build a Measurement Vector of mean
      MeasurementType mean;

      // Build a MeasurementVector of variance
      MeasurementType variance;

      // loop through the sensors to determinte the output dates
      for (const auto& sensor : sensorInDays) {
          std::vector<int> outDates;
          if (resample) {
              auto it = sp.find(sensor.first);
              if (it == sp.end()) {
                  itkExceptionMacro("Sampling rate required for sensor " << sensor.first);
              }
              auto rate = it->second;

              for (int date = sensor.second.front(); date <= sensor.second.back(); date += rate) {
                  outDates.emplace_back(date);
              }
          } else {
              outDates.insert(outDates.end(), sensor.second.begin(), sensor.second.end());
          }
          sensorOutDays[sensor.first] = outDates;
      }


      for (size_t i = 0; i < prodPerTile.size(); i ++) {
          ImageDescriptorList& descriptorsList = fullDescriptorsList[i];
          // Merge the rasters and the masks
          ConcatenateVectorImagesFilterType::Pointer bandsConcat = ConcatenateVectorImagesFilterType::New();
          ConcatenateVectorImagesFilterType::Pointer maskConcat = ConcatenateVectorImagesFilterType::New();
          // Also build the image dates structures
          SensorDataCollection sdCollection;

          int index = 0;
          std::string lastMission = "";
          for (const ImageDescriptor& id : descriptorsList) {
              if (id.mission != lastMission) {
                  SensorData sd;
                  sd.sensorName = id.mission;
                  sd.outDates = sensorOutDays[id.mission];
                  sdCollection.push_back(sd);
                  lastMission = id.mission;
              }

              SensorData& sd = sdCollection[sdCollection.size() - 1];
              int inDay = getDaysFromEpoch(id.aquisitionDate);

              sd.inDates.push_back(inDay);

              bandsConcat->SetInput(index, id.bands);
              maskConcat->SetInput(index, id.mask);
              index++;
          }
          m_ImageMergers->PushBack(bandsConcat);
          m_ImageMergers->PushBack(maskConcat);

          // Set the temporal resampling / gap filling filter
          TemporalResamplingFilterType::Pointer tempResampler = TemporalResamplingFilterType::New();
          tempResampler->SetInputRaster(bandsConcat->GetOutput());
          tempResampler->SetInputMask(maskConcat->GetOutput());
          // The output days will be updated later
          tempResampler->SetInputData(sdCollection);
          m_TempResamplers->PushBack(tempResampler);

          // Set the feature extractors
          CropTypeFeatureExtractionFilterType::Pointer featExtractor = CropTypeFeatureExtractionFilterType::New();
          featExtractor->SetInput(tempResampler->GetOutput());
          m_FeatureExtrators->PushBack(featExtractor);

          featExtractor->UpdateOutputInformation();
          unsigned int nbBands = featExtractor->GetOutput()->GetNumberOfComponentsPerPixel();

          ImageType::SizeType size = featExtractor->GetOutput()->GetLargestPossibleRegion().GetSize();

          // Set the statistics computation filter
          StreamingStatisticsVImageFilterType::Pointer statisticsFilter = StreamingStatisticsVImageFilterType::New();
          statisticsFilter->SetInput(featExtractor->GetOutput());
          if( HasValue( "bv" )==true ) {
              statisticsFilter->SetIgnoreUserDefinedValue(true);
              statisticsFilter->SetUserIgnoredValue(GetParameterFloat("bv"));
          }
          m_Statistics->PushBack(statisticsFilter);
          if (i == 0) {
              mean.SetSize(nbBands);
              mean.Fill(0.);
              variance.SetSize(nbBands);
              variance.Fill(0.);
          }

          statisticsFilter->Update();
          mean += statisticsFilter->GetMean();
          for (unsigned int itBand = 0; itBand < nbBands; itBand++)
            {
            variance[itBand] += (size[0] * size[1] - 1) * (statisticsFilter->GetCovariance())(itBand, itBand);
            }
          //Increment nbSamples
          nbSamples += size[0] * size[1] * nbBands;

          startIndex = endIndex;
      }

      //Divide by the number of input images to get the mean over all layers
      mean /= m_Statistics->Size();
      //Apply the pooled variance formula
      variance /= (nbSamples - m_Statistics->Size());

      MeasurementType stddev(variance.GetSize());
      for (unsigned int i = 0; i < variance.GetSize(); ++i)
        {
        stddev[i] = vcl_sqrt(variance[i]);
        }

      // Write the Statistics via the statistic writer
      typedef otb::StatisticsXMLFileWriter<MeasurementType> StatisticsWriter;
      StatisticsWriter::Pointer writer = StatisticsWriter::New();
      writer->SetFileName(GetParameterString("out"));
      writer->AddInput("mean", mean);
      writer->AddInput("stddev", stddev);
      writer->Update();

  }
  //  Software Guide :EndCodeSnippet

  inline int getDaysFromEpoch(const std::string &date)
  {
      struct tm tm = {};
      if (strptime(date.c_str(), "%Y%m%d", &tm) == NULL) {
          itkExceptionMacro("Invalid value for a date: " + date);
      }
      return mktime(&tm) / 86400;
  }

  void updateRequiredImageSize(std::vector<std::string>& descriptors, int startIndex, int endIndex, TileData& td) {
      td.m_imageWidth = 0;
      td.m_imageHeight = 0;

      // look for the first raster belonging to the main mission
      MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
      SPOT4MetadataReaderType::Pointer spot4MetadataReader = SPOT4MetadataReaderType::New();
      for (int i = startIndex; i < endIndex; i++) {
          const std::string& desc = descriptors[i];
          if (auto meta = maccsMetadataReader->ReadMetadata(desc)) {
              // check if the raster corresponds to the main mission
              if (meta->Header.FixedHeader.Mission.find(m_mission) != std::string::npos) {
                  std::string rootFolder = extractFolder(desc);
                  std::string imageFile = getMACCSRasterFileName(rootFolder, meta->ProductOrganization.ImageFiles, "_FRE");
                  if (imageFile.size() == 0) {
                      imageFile = getMACCSRasterFileName(rootFolder, meta->ProductOrganization.ImageFiles, "_FRE_R1");
                  }
                  ImageReaderType::Pointer reader = getReader(imageFile);
                  reader->UpdateOutputInformation();
                  float curRes = reader->GetOutput()->GetSpacing()[0];


                  const float scale = (float)m_pixSize / curRes;
                  td.m_imageWidth = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[0] / scale;
                  td.m_imageHeight = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[1] / scale;

                  FloatVectorImageType::PointType origin = reader->GetOutput()->GetOrigin();
                  ImageType::SpacingType spacing = reader->GetOutput()->GetSpacing();
                  td.m_imageOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale - 1.0);
                  td.m_imageOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale - 1.0);

                  break;
              }

          }else if (auto meta = spot4MetadataReader->ReadMetadata(desc)) {
              // check if the raster corresponds to the main mission
              if (meta->Header.Ident.find(m_mission) != std::string::npos) {
                  // get the root foloder from the descriptor file name
                  std::string rootFolder = extractFolder(desc);
                  std::string imageFile = rootFolder + meta->Files.OrthoSurfCorrPente;
                  ImageReaderType::Pointer reader = getReader(imageFile);
                  reader->UpdateOutputInformation();
                  float curRes = reader->GetOutput()->GetSpacing()[0];


                  const float scale = (float)m_pixSize / curRes;
                  td.m_imageWidth = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[0] / scale;
                  td.m_imageHeight = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[1] / scale;

                  FloatVectorImageType::PointType origin = reader->GetOutput()->GetOrigin();
                  ImageType::SpacingType spacing = reader->GetOutput()->GetSpacing();
                  td.m_imageOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale - 1.0);
                  td.m_imageOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale - 1.0);

                  break;
              }

          } else {
            itkExceptionMacro("Unable to read metadata from " << desc);
          }
      }
  }

  // Process a SPOT4 metadata structure and extract the needed bands and masks.
  void ProcessSpot4Metadata(const SPOT4Metadata& meta, const std::string& filename, ImageDescriptor& descriptor, const TileData& td) {

      // Extract the raster date
      descriptor.aquisitionDate = formatSPOT4Date(meta.Header.DatePdv);

      // Save the descriptor fiel path
      descriptor.filename = filename;

      // save the mission
      descriptor.mission = SPOT;
      descriptor.isMain = m_mission.compare(descriptor.mission) == 0;

      // get the root foloder from the descriptor file name
      std::string rootFolder = extractFolder(filename);

      // Get the spot bands
      descriptor.bands = getSpotBands(meta, rootFolder, td);

      // The mask is built with the SpotMaskFilter
      descriptor.mask = getSpotMask(meta, rootFolder, td);
  }

  // Process a LANDSAT8 metadata structure and extract the needed bands and masks.
  void ProcessLandsat8Metadata(const MACCSFileMetadata& meta, const std::string& filename, ImageDescriptor& descriptor, const TileData& td) {

      // Extract the raster date
      descriptor.aquisitionDate = meta.InstanceId.AcquisitionDate;

      // Save the descriptor fiel path
      descriptor.filename = filename;

      // save the mission
      descriptor.mission = LANDSAT;
      descriptor.isMain = m_mission.compare(descriptor.mission) == 0;

      // get the root foloder from the descriptor file name
      std::string rootFolder = extractFolder(filename);

      // Get the bands
      descriptor.bands = getLandsatBands(meta, rootFolder,td);

      // Get the mask
      descriptor.mask = getLandsatMask(meta, rootFolder,td);
  }


  // Process a SENTINEL2 metadata structure and extract the needed bands and masks.
  void ProcessSentinel2Metadata(const MACCSFileMetadata& meta, const std::string& filename, ImageDescriptor& descriptor, const TileData& td) {

      // Extract the raster date
      descriptor.aquisitionDate = meta.InstanceId.AcquisitionDate;

      // Save the descriptor fiel path
      descriptor.filename = filename;

      // save the mission
      descriptor.mission = SENTINEL;
      descriptor.isMain = m_mission.compare(descriptor.mission) == 0;

      // get the root foloder from the descriptor file name
      std::string rootFolder = extractFolder(filename);

      // Get the bands
      descriptor.bands = getLandsatBands(meta, rootFolder,td);

      // Get the mask
      descriptor.mask = getLandsatMask(meta, rootFolder,td);
  }

  // Sort the descriptors based on the aquisition date
  static bool SortUnmergedMetadata(const ImageDescriptor& o1, const ImageDescriptor& o2) {
      if (o1.isMain && !o2.isMain) {
          return true;
      }
      if (o2.isMain && !o1.isMain) {
          return false;
      }
      int missionComp = o1.mission.compare(o2.mission);
      if (missionComp == 0) {
          return o1.aquisitionDate.compare(o2.aquisitionDate) < 0;
      } else {
          return missionComp < 0;
      }
  }


  // Get the id of the band. Return -1 if band not found.
  int getBandIndex(const std::vector<MACCSBand>& bands, const std::string& name) {
      for (const MACCSBand& band : bands) {
          if (band.Name == name) {
              return std::stoi(band.Id);
          }
      }
      return -1;
  }

  // get a reader from the file path
  ImageReaderType::Pointer getReader(const std::string& filePath) {
      ImageReaderType::Pointer reader = ImageReaderType::New();

      // set the file name
      reader->SetFileName(filePath);

      // add it to the list and return
      m_ImageReaderList->PushBack(reader);
      return reader;
  }

  // Return the path to a file for which the name end in the ending
  std::string getMACCSRasterFileName(const std::string& rootFolder, const std::vector<MACCSFileInformation>& imageFiles, const std::string& ending) {

      for (const MACCSFileInformation& fileInfo : imageFiles) {
          if (fileInfo.LogicalName.length() >= ending.length() &&
                  0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
              return rootFolder + fileInfo.FileLocation.substr(0, fileInfo.FileLocation.find_last_of('.')) + ".DBL.TIF";
          }

      }
      return "";
  }


  // Return the path to a file for which the name end in the ending
  std::string getMACCSMaskFileName(const std::string& rootFolder, const std::vector<MACCSAnnexInformation>& maskFiles, const std::string& ending) {

      for (const MACCSAnnexInformation& fileInfo : maskFiles) {
          if (fileInfo.File.LogicalName.length() >= ending.length() &&
                  0 == fileInfo.File.LogicalName.compare (fileInfo.File.LogicalName.length() - ending.length(), ending.length(), ending)) {
              return rootFolder + fileInfo.File.FileLocation.substr(0, fileInfo.File.FileLocation.find_last_of('.')) + ".DBL.TIF";
          }
      }
      return "";
  }

  // Extract the folder from a given path.
  std::string extractFolder(const std::string& filename) {
      size_t pos = filename.find_last_of("/\\");
      if (pos == std::string::npos) {
          return "";
      }

      return filename.substr(0, pos) + "/";
  }

  // Get the path to the Spot4 raster
  std::string getSPOT4RasterFileName(const SPOT4Metadata & desc, const std::string& folder) {
      return folder + desc.Files.OrthoSurfCorrPente;
  }

  // Return the path to a SPOT4 mask file
  std::string getSPOT4MaskFileName(const SPOT4Metadata & desc, const std::string& rootFolder, const unsigned char maskType) {

      std::string file;
      switch (maskType) {
      case MASK_TYPE_NUA:
          file = desc.Files.MaskNua;
          break;
      case MASK_TYPE_DIV:
          file = desc.Files.MaskDiv;
          break;
      case MASK_TYPE_SAT:
      default:
          file = desc.Files.MaskSaturation;
          break;
      }

      return rootFolder + file;
  }

  ImageType::Pointer getSpotBands(const SPOT4Metadata& meta, const std::string& rootFolder, const TileData& td) {
      // Return the bands associated with a SPOT product
      // get the bands
      std::string imageFile = rootFolder + meta.Files.OrthoSurfCorrPente;
      ImageReaderType::Pointer reader = getReader(imageFile);
      reader->UpdateOutputInformation();
      float curRes = reader->GetOutput()->GetSpacing()[0];

      // For SPOT series the bands are already in order. Only a resampling might be required
      return getResampledBands(reader->GetOutput(), td, curRes, false);
  }

  ImageType::Pointer getSpotMask(const SPOT4Metadata& meta, const std::string& rootFolder, const TileData& td) {
      // Return the mask associated with a SPOT product
      // Get the validity mask
      std::string maskFileDiv = rootFolder + meta.Files.MaskDiv;
      ImageReaderType::Pointer maskReaderDiv = getReader(maskFileDiv);
      maskReaderDiv->UpdateOutputInformation();
      float curRes = maskReaderDiv->GetOutput()->GetSpacing()[0];

      // Get the saturation mask
      std::string maskFileSat = rootFolder + meta.Files.MaskSaturation;
      ImageReaderType::Pointer maskReaderSat = getReader(maskFileSat);

      // Get the clouds mask
      std::string maskFileNua = rootFolder + meta.Files.MaskNua;
      ImageReaderType::Pointer maskReaderNua = getReader(maskFileNua);
\
      // Build the SpotMaskFilter
      SpotMaskFilterType::Pointer spotMaskFilter = SpotMaskFilterType::New();
      spotMaskFilter->SetInputValidityMask(maskReaderDiv->GetOutput());
      spotMaskFilter->SetInputSaturationMask(maskReaderSat->GetOutput());
      spotMaskFilter->SetInputCloudsMask(maskReaderNua->GetOutput());

      // Add the filter to the list
      m_SpotMaskFilters->PushBack(spotMaskFilter);

      // Return the mask resampled to the required value
      return getResampledBands(spotMaskFilter->GetOutput(), td, curRes, true);
  }

  ImageType::Pointer getLandsatBands(const MACCSFileMetadata& meta, const std::string& rootFolder, const TileData& td) {
      // Return the bands associated with a LANDSAT product
      // get the bands
      std::string imageFile = getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE");
      ImageReaderType::Pointer reader = getReader(imageFile);
      reader->UpdateOutputInformation();
      float curRes = reader->GetOutput()->GetSpacing()[0];

      // Extract the bands from 3 to 6
      MultiChannelExtractROIType::Pointer chExtractor = MultiChannelExtractROIType::New();
      chExtractor->SetInput(reader->GetOutput());
      chExtractor->SetFirstChannel(3);
      chExtractor->SetLastChannel(6);
      m_ChannelExtractors->PushBack(chExtractor);

      // A resampling might be required
      return getResampledBands(chExtractor->GetOutput(), td, curRes, false);
  }

  ImageType::Pointer getLandsatMask(const MACCSFileMetadata& meta, const std::string& rootFolder, const TileData& td) {
      // Return the mask associated with a LANDSAT product
      // Get the quality mask
      std::string maskFileQuality = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_QLT");
      ImageReaderType::Pointer maskReaderQuality = getReader(maskFileQuality);
      maskReaderQuality->UpdateOutputInformation();
      float curRes = maskReaderQuality->GetOutput()->GetSpacing()[0];

       // Get the cloud mask
      std::string maskFileCloud = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_CLD");
      ImageReaderType::Pointer maskReaderCloud = getReader(maskFileCloud);

      // Build the LandsatMaskFilter
      LandsatMaskFilterType::Pointer landsatMaskFilter = LandsatMaskFilterType::New();
      landsatMaskFilter->SetInputQualityMask(maskReaderQuality->GetOutput());
      landsatMaskFilter->SetInputCloudsMask(maskReaderCloud->GetOutput());

      // Add the filter to the list
      m_LandsatMaskFilters->PushBack(landsatMaskFilter);

      // Resample if needed
      return getResampledBands(landsatMaskFilter->GetOutput(), td, curRes, true);
  }

  ImageType::Pointer getSentinelBands(const MACCSFileMetadata& meta, const std::string& rootFolder, const TileData& td) {
      // Return the bands associated with a SENTINEL product
      // get the bands
      //Extract the first 3 bands form the first file.
      std::string imageFile1 = getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE_R1");
      ImageReaderType::Pointer reader1 = getReader(imageFile1);
      reader1->UpdateOutputInformation();
      float curRes1 = reader1->GetOutput()->GetSpacing()[0];

      // Get the index of the green band
      int gIndex = getBandIndex(meta.ImageInformation.Resolutions[0].Bands, "B3");

      // Get the index of the red band
      int rIndex = getBandIndex(meta.ImageInformation.Resolutions[0].Bands, "B4");

      // Get the index of the NIR band
      int nirIndex = getBandIndex(meta.ImageInformation.Resolutions[0].Bands, "B8");

      MultiChannelExtractROIType::Pointer chExtractor1 = MultiChannelExtractROIType::New();
      chExtractor1->SetInput(reader1->GetOutput());
      chExtractor1->SetChannel(gIndex);
      chExtractor1->SetChannel(rIndex);
      chExtractor1->SetChannel(nirIndex);
      m_ChannelExtractors->PushBack(chExtractor1);


      //Extract the last band form the second file.
      std::string imageFile2 = getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE_R2");
      ImageReaderType::Pointer reader2 = getReader(imageFile2);
      reader2->UpdateOutputInformation();
      float curRes2 = reader2->GetOutput()->GetSpacing()[0];

      // Get the index of the SWIR band
      int swirIndex = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B11");
      MultiChannelExtractROIType::Pointer chExtractor2 = MultiChannelExtractROIType::New();
      chExtractor2->SetInput(reader2->GetOutput());
      chExtractor2->SetChannel(swirIndex);
      m_ChannelExtractors->PushBack(chExtractor2);


      // Concatenate the previous results resampling if needed
      ConcatenateVectorImagesFilterType::Pointer concat = ConcatenateVectorImagesFilterType::New();
      concat->SetInput(1, getResampledBands(chExtractor1->GetOutput(), td, curRes1, false));
      concat->SetInput(2, getResampledBands(chExtractor2->GetOutput(), td, curRes2, false));
      m_ImageMergers->PushBack(concat);

      // Return the concatenation result
      return concat->GetOutput();
  }

  ImageType::Pointer getSentinelMask(const MACCSFileMetadata& meta, const std::string& rootFolder, const TileData& td) {
      // Return the mask associated with a SENTINEL product

      // Get the quality mask
      std::string maskFileQuality = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_QLT_R1");
      ImageReaderType::Pointer maskReaderQuality = getReader(maskFileQuality);
      maskReaderQuality->UpdateOutputInformation();
      float curRes = maskReaderQuality->GetOutput()->GetSpacing()[0];

       // Get the cloud mask
      std::string maskFileCloud = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_CLD_R1");
      ImageReaderType::Pointer maskReaderCloud = getReader(maskFileCloud);

      // Build the SentinelMaskFilter
      SentinelMaskFilterType::Pointer sentinelMaskFilter = SentinelMaskFilterType::New();
      sentinelMaskFilter->SetInputQualityMask(maskReaderQuality->GetOutput());
      sentinelMaskFilter->SetInputCloudsMask(maskReaderCloud->GetOutput());

      // Add the filter to the list
      m_SentinelMaskFilters->PushBack(sentinelMaskFilter);

      // Resample if needed
      return getResampledBands(sentinelMaskFilter->GetOutput(), td, curRes, true);
  }

  ImageType::Pointer getResampledBands(const ImageType::Pointer& image, const TileData& td, const float curRes, const bool isMask) {
       const float invRatio = static_cast<float>(m_pixSize) / curRes;

       // Scale Transform
       OutputVectorType scale;
       scale[0] = invRatio;
       scale[1] = invRatio;

       image->UpdateOutputInformation();
       auto imageSize = image->GetLargestPossibleRegion().GetSize();

       // Evaluate size
       ResampleFilterType::SizeType recomputedSize;
       if (td.m_imageWidth != 0 && td.m_imageHeight != 0) {
           recomputedSize[0] = td.m_imageWidth;
           recomputedSize[1] = td.m_imageHeight;
       } else {
           recomputedSize[0] = imageSize[0] / scale[0];
           recomputedSize[1] = imageSize[1] / scale[1];
       }

       if (imageSize == recomputedSize)
           return image;

       ResampleFilterType::Pointer resampler = ResampleFilterType::New();
       resampler->SetInput(image);

       // Set the interpolator
       if (isMask) {
           NearestNeighborInterpolationType::Pointer interpolator = NearestNeighborInterpolationType::New();
           resampler->SetInterpolator(interpolator);
       } else {
           BicubicInterpolationType::Pointer interpolator = BicubicInterpolationType::New();
           resampler->SetInterpolator(interpolator);
       }

//       IdentityTransformType::Pointer transform = IdentityTransformType::New();

       resampler->SetOutputParametersFromImage( image );

       // Evaluate spacing
       ImageType::SpacingType spacing = image->GetSpacing();
       ImageType::SpacingType OutputSpacing;
       OutputSpacing[0] = spacing[0] * scale[0];
       OutputSpacing[1] = spacing[1] * scale[1];

       resampler->SetOutputSpacing(OutputSpacing);

//       FloatVectorImageType::PointType origin = image->GetOrigin();
//       FloatVectorImageType::PointType outputOrigin;
//       outputOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale[0] - 1.0);
//       outputOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale[1] - 1.0);

       resampler->SetOutputOrigin(td.m_imageOrigin);
       resampler->SetCheckOutputBounds(false);

//       resampler->SetTransform(transform);

       resampler->SetOutputSize(recomputedSize);

       // Padd with nodata
       ImageType::PixelType defaultValue;
       itk::NumericTraits<ImageType::PixelType>::SetLength(defaultValue, image->GetNumberOfComponentsPerPixel());
       if(!isMask) {
           defaultValue = -10000;
       }
       resampler->SetEdgePaddingValue(defaultValue);


       m_ResamplersList->PushBack(resampler);
       return resampler->GetOutput();
  }


  // build the date for spot products as YYYYMMDD
  inline std::string formatSPOT4Date(const std::string& date) {
      return date.substr(0,4) + date.substr(5,2) + date.substr(8,2);
  }


  std::string                           m_mission;
  float                                 m_pixSize;

  ImageReaderListType::Pointer          m_ImageReaderList;
  ResampleFilterListType::Pointer       m_ResamplersList;

  SpotMaskFilterListType::Pointer                   m_SpotMaskFilters;
  LandsatMaskFilterListType::Pointer                m_LandsatMaskFilters;
  SentinelMaskFilterListType::Pointer               m_SentinelMaskFilters;
  MultiChannelExtractROIListType::Pointer           m_ChannelExtractors;
  ConcatenateVectorImagesFilterListType::Pointer    m_ImageMergers;
  TemporalResamplingFilterListType::Pointer         m_TempResamplers;
  CropTypeFeatureExtractionFilterListType::Pointer  m_FeatureExtrators;
  StreamingStatisticsVImageFilterListType::Pointer  m_Statistics;

};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::CropTypeStatistics)
//  Software Guide :EndCodeSnippet


