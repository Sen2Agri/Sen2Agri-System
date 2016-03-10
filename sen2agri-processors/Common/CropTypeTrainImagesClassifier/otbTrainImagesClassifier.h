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
#include "otbConfigure.h"

#include "otbWrapperApplicationFactory.h"

#include <iostream>

//Image
#include "otbVectorImage.h"
#include "otbVectorData.h"
#include "otbListSampleGenerator.h"
#include "otbListSampleGeneratorRaster.h"

// ListSample
#include "itkVariableLengthVector.h"

//Estimator
#include "otbMachineLearningModelFactory.h"

#ifdef OTB_USE_OPENCV
# include "otbKNearestNeighborsMachineLearningModel.h"
# include "otbRandomForestsMachineLearningModel.h"
# include "otbSVMMachineLearningModel.h"
# include "otbBoostMachineLearningModel.h"
# include "otbDecisionTreeMachineLearningModel.h"
# include "otbGradientBoostedTreeMachineLearningModel.h"
# include "otbNormalBayesMachineLearningModel.h"
# include "otbNeuralNetworkMachineLearningModel.h"
#endif

#ifdef OTB_USE_LIBSVM 
#include "otbLibSVMMachineLearningModel.h"
#endif

// Statistic XML Reader
#include "otbStatisticsXMLFileReader.h"

// Validation
#include "otbConfusionMatrixCalculator.h"

#include "itkTimeProbe.h"
#include "otbStandardFilterWatcher.h"

// Normalize the samples
#include "otbShiftScaleSampleListFilter.h"

// List sample concatenation
#include "otbConcatenateSampleListFilter.h"

// Balancing ListSample
#include "otbListSampleToBalancedListSampleFilter.h"

// VectorData projection filter

// Extract a ROI of the vectordata
#include "otbVectorDataIntoImageProjectionFilter.h"

// Elevation handler
#include "otbWrapperElevationParametersHandler.h"



//Transform
#include "itkScalableAffineTransform.h"
#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"
#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"

// Filters
#include "otbMultiChannelExtractROI.h"
#include "otbStreamingResampleImageFilter.h"

#include "../Filters/otbConcatenateVectorImagesFilter.h"
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
typedef otb::ObjectList<ImageType>                                 ImageListType;

typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;

typedef itk::MACCSMetadataReader                                   MACCSMetadataReaderType;
typedef itk::SPOT4MetadataReader                                   SPOT4MetadataReaderType;

typedef otb::StreamingResampleImageFilter<ImageType, ImageType, double>    ResampleFilterType;
typedef otb::ObjectList<ResampleFilterType>                           ResampleFilterListType;
typedef otb::BCOInterpolateImageFunction<ImageType,
                                            double>          BicubicInterpolationType;
typedef itk::LinearInterpolateImageFunction<ImageType,
                                            double>          LinearInterpolationType;
typedef itk::NearestNeighborInterpolateImageFunction<ImageType, double>             NearestNeighborInterpolationType;

typedef itk::IdentityTransform<double, ImageType::ImageDimension>      IdentityTransformType;
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

namespace otb
{
namespace Wrapper
{

class TrainImagesClassifier: public Application
{
public:
  /** Standard class typedefs. */
  typedef TrainImagesClassifier Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self)

  itkTypeMacro(TrainImagesClassifier, otb::Application)

  typedef FloatVectorImageType::PixelType         PixelType;
  typedef FloatVectorImageType::InternalPixelType InternalPixelType;

  // Training vectordata
  typedef itk::VariableLengthVector<InternalPixelType> MeasurementType;

  // SampleList manipulation
  typedef otb::ListSampleGenerator<FloatVectorImageType, VectorDataType> ListSampleGeneratorType;
  typedef otb::ListSampleGeneratorRaster<FloatVectorImageType, Int32ImageType> ListSampleGeneratorRasterType;

  typedef ListSampleGeneratorType::ListSampleType ListSampleType;
  typedef ListSampleGeneratorType::LabelType LabelType;
  typedef ListSampleGeneratorType::ListLabelType LabelListSampleType;
  typedef otb::Statistics::ConcatenateSampleListFilter<ListSampleType> ConcatenateListSampleFilterType;
  typedef otb::Statistics::ConcatenateSampleListFilter<LabelListSampleType> ConcatenateLabelListSampleFilterType;

  // Statistic XML file Reader
  typedef otb::StatisticsXMLFileReader<MeasurementType> StatisticsReader;

  // Enhance List Sample  typedef otb::Statistics::ListSampleToBalancedListSampleFilter<ListSampleType, LabelListSampleType>      BalancingListSampleFilterType;
  typedef otb::Statistics::ShiftScaleSampleListFilter<ListSampleType, ListSampleType> ShiftScaleFilterType;

  // Machine Learning models
  typedef otb::MachineLearningModelFactory<InternalPixelType, ListSampleGeneratorType::ClassLabelType> MachineLearningModelFactoryType;
  typedef MachineLearningModelFactoryType::MachineLearningModelTypePointer ModelPointerType;
  
#ifdef OTB_USE_OPENCV
  typedef otb::RandomForestsMachineLearningModel<InternalPixelType, ListSampleGeneratorType::ClassLabelType> RandomForestType;
  typedef otb::KNearestNeighborsMachineLearningModel<InternalPixelType, ListSampleGeneratorType::ClassLabelType> KNNType;
  typedef otb::SVMMachineLearningModel<InternalPixelType, ListSampleGeneratorType::ClassLabelType> SVMType;
  typedef otb::BoostMachineLearningModel<InternalPixelType, ListSampleGeneratorType::ClassLabelType> BoostType;
  typedef otb::DecisionTreeMachineLearningModel<InternalPixelType, ListSampleGeneratorType::ClassLabelType> DecisionTreeType;
  typedef otb::GradientBoostedTreeMachineLearningModel<InternalPixelType, ListSampleGeneratorType::ClassLabelType> GradientBoostedTreeType;
  typedef otb::NeuralNetworkMachineLearningModel<InternalPixelType, ListSampleGeneratorType::ClassLabelType> NeuralNetworkType;
  typedef otb::NormalBayesMachineLearningModel<InternalPixelType, ListSampleGeneratorType::ClassLabelType> NormalBayesType;
#endif

#ifdef OTB_USE_LIBSVM 
  typedef otb::LibSVMMachineLearningModel<InternalPixelType, ListSampleGeneratorType::ClassLabelType> LibSVMType;
#endif
 
  // Estimate performance on validation sample
  typedef otb::ConfusionMatrixCalculator<LabelListSampleType, LabelListSampleType> ConfusionMatrixCalculatorType;
  typedef ConfusionMatrixCalculatorType::ConfusionMatrixType ConfusionMatrixType;
  typedef ConfusionMatrixCalculatorType::MapOfIndicesType MapOfIndicesType;
  typedef ConfusionMatrixCalculatorType::ClassLabelType ClassLabelType;


  // VectorData projection filter
  typedef otb::VectorDataProjectionFilter<VectorDataType, VectorDataType> VectorDataProjectionFilterType;

  // Extract ROI
  typedef otb::VectorDataIntoImageProjectionFilter<VectorDataType, FloatVectorImageType> VectorDataReprojectionType;

protected:
  using Superclass::AddParameter;
  friend void InitSVMParams(TrainImagesClassifier & app);

private:
  void DoInit();

  void DoUpdateParameters();

  void LogConfusionMatrix(ConfusionMatrixCalculatorType* confMatCalc);

#ifdef OTB_USE_LIBSVM 
  void InitLibSVMParams();
#endif  
  
#ifdef OTB_USE_OPENCV
  void InitBoostParams();
  void InitSVMParams();
  void InitDecisionTreeParams();
  void InitGradientBoostedTreeParams();
  void InitNeuralNetworkParams();
  void InitNormalBayesParams();
  void InitRandomForestsParams();
  void InitKNNParams();
#endif

#ifdef OTB_USE_LIBSVM 
  void TrainLibSVM(ListSampleType::Pointer trainingListSample, LabelListSampleType::Pointer trainingLabeledListSample);
#endif 
  
#ifdef OTB_USE_OPENCV
  void TrainBoost(ListSampleType::Pointer trainingListSample, LabelListSampleType::Pointer trainingLabeledListSample);
  void TrainSVM(ListSampleType::Pointer trainingListSample, LabelListSampleType::Pointer trainingLabeledListSample);
  void TrainDecisionTree(ListSampleType::Pointer trainingListSample, LabelListSampleType::Pointer trainingLabeledListSample);
  void TrainGradientBoostedTree(ListSampleType::Pointer trainingListSample, LabelListSampleType::Pointer trainingLabeledListSample);
  void TrainNeuralNetwork(ListSampleType::Pointer trainingListSample, LabelListSampleType::Pointer trainingLabeledListSample);
  void TrainNormalBayes(ListSampleType::Pointer trainingListSample, LabelListSampleType::Pointer trainingLabeledListSample);
  void TrainRandomForests(ListSampleType::Pointer trainingListSample, LabelListSampleType::Pointer trainingLabeledListSample);
  void TrainKNN(ListSampleType::Pointer trainingListSample, LabelListSampleType::Pointer trainingLabeledListSample);
#endif

  void Classify(ListSampleType::Pointer validationListSample, LabelListSampleType::Pointer predictedList);

  void DoExecute();

  VectorDataReprojectionType::Pointer vdreproj;
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

       IdentityTransformType::Pointer transform = IdentityTransformType::New();

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

       resampler->SetTransform(transform);

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
  ImageListType::Pointer                            m_InputImages;
};

}
}


