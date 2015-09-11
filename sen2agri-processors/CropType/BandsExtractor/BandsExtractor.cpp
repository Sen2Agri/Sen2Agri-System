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

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"
#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"

#include "otbVectorImage.h"
#include "otbImageList.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbBandMathImageFilter.h"

#include "otbStreamingResampleImageFilter.h"
#include "otbStreamingStatisticsImageFilter.h"
#include "otbLabelImageToVectorDataFilter.h"

//Transform
#include "itkScalableAffineTransform.h"
#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

#include "otbOGRIOHelper.h"


#define LANDSAT    "LANDSAT"
#define SENTINEL   "SENTINEL"

#define TYPE_MACCS  0
#define TYPE_SPOT4  1

#define MASK_TYPE_NUA   0
#define MASK_TYPE_SAT   1
#define MASK_TYPE_DIV   2


typedef otb::VectorImage<short, 2>                                 ImageType;
typedef otb::Image<short, 2>                                       InternalImageType;

typedef otb::ImageList<ImageType>                                  ImageListType;
typedef otb::ImageList<InternalImageType>                          InternalImageListType;

typedef otb::ImageListToVectorImageFilter<InternalImageListType,
                                     ImageType >                   ListConcatenerFilterType;
typedef otb::MultiToMonoChannelExtractROI<ImageType::InternalPixelType,
                                     InternalImageType::PixelType> ExtractROIFilterType;
typedef otb::ObjectList<ExtractROIFilterType>                      ExtractROIFilterListType;

typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;

typedef itk::MACCSMetadataReader                                   MACCSMetadataReaderType;
typedef itk::SPOT4MetadataReader                                   SPOT4MetadataReaderType;

typedef otb::StreamingResampleImageFilter<InternalImageType, InternalImageType, double>    ResampleFilterType;
typedef otb::ObjectList<ResampleFilterType>                           ResampleFilterListType;
typedef itk::LinearInterpolateImageFunction<InternalImageType,
                                            double>          LinearInterpolationType;
typedef itk::IdentityTransform<double, InternalImageType::ImageDimension>      IdentityTransformType;
typedef itk::ScalableAffineTransform<double, InternalImageType::ImageDimension> ScalableTransformType;
typedef ScalableTransformType::OutputVectorType                         OutputVectorType;

typedef otb::BandMathImageFilter<InternalImageType>                 BandMathImageFilterType;
typedef otb::ObjectList<BandMathImageFilterType>                    BandMathImageFilterListType;

typedef otb::StreamingStatisticsImageFilter<InternalImageType> StreamingStatisticsImageFilterType;
typedef otb::LabelImageToVectorDataFilter<InternalImageType>   LabelImageToVectorDataFilterType;

struct SourceImageMetadata {
    MACCSFileMetadata  msccsFileMetadata;
    SPOT4Metadata      spot4Metadata;
};

struct ImageDescriptor {
    // the descriptor file
    std::string filename;
    // the aquisition date in format YYYYMMDD
    std::string aquisitionDate;

    // The Green band
    InternalImageType::Pointer imgG;
    // The Red band
    InternalImageType::Pointer imgR;
    // The NIR band
    InternalImageType::Pointer imgNIR;
    // The SWIR band
    InternalImageType::Pointer imgSWIR;

    // The Validity mask (0 - invalid, 1 - valid)
    InternalImageType::Pointer maskValid;
    // The Saturation mask (0 - no saturation)
    InternalImageType::Pointer maskSat;
    // The Cloud mask (0 - no cloud)
    InternalImageType::Pointer maskCloud;
    // The Cloud mask (0 - no water)
    InternalImageType::Pointer maskWater;
    // The Cloud mask (0 - no snow)
    InternalImageType::Pointer maskSnow;

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
class BandsExtractor : public Application
//  Software Guide : EndCodeSnippet
{
public:
  //  Software Guide : BeginLatex
  // The \code{ITK} public types for the class, the superclass and smart pointers.
  // Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  typedef BandsExtractor Self;
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

  itkTypeMacro(BandsExtractor, otb::Application)
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
      SetName("BandsExtractor");
      SetDescription("Extract only the needed bands from a temporal series of images.");

      SetDocName("BandsExtractor");
      SetDocLongDescription("Extract only the four needed bands (R, G, NIR and SWIR) from a list of images and concatenate them into one image.");
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

    m_Concatener = ListConcatenerFilterType::New();
    m_Masks = ListConcatenerFilterType::New();
    m_AllMasks = ListConcatenerFilterType::New();
    m_ImageList = InternalImageListType::New();
    m_MasksList = InternalImageListType::New();
    m_AllMasksList = InternalImageListType::New();
    m_ResamplersList = ResampleFilterListType::New();
    m_ExtractorList = ExtractROIFilterListType::New();
    m_ImageReaderList = ImageReaderListType::New();
    m_BandMathList = BandMathImageFilterListType::New();    
    //m_VldMaskList = InternalImageListType::New();
    m_borderMask = BandMathImageFilterType::New();
    m_ShapeBuilder = LabelImageToVectorDataFilterType::New();

#ifdef OTB_MUPARSER_HAS_CXX_LOGICAL_OPERATORS
      m_maskExpression = "(b3 == 1) && (b1 == 0) && (b2 == 0) ? 0 : 1";
#else
      m_maskExpression = "if((b1 == 0) and (b2 == 0) and (b3 == 1), 0, 1)";
#endif

    //  Software Guide : BeginCodeSnippet
    AddParameter(ParameterType_InputFilenameList, "il", "The xml files");

    AddParameter(ParameterType_OutputImage, "out", "The concatenated images");
    AddParameter(ParameterType_OutputImage, "mask", "The concatenated masks");
    MandatoryOff("mask");

    AddParameter(ParameterType_OutputImage, "allmasks", "The concatenated masks for cloud, water, snow, saturation, etc.");
    MandatoryOff("allmasks");

    AddParameter(ParameterType_OutputFilename, "outdate", "The file containing the dates for the images");
    AddParameter(ParameterType_OutputVectorData, "shape", "The file containing the border shape");
    MandatoryOff("shape");


     //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
    // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("il", "image1.xml image2.xml");
    SetDocExampleParameterValue("out", "fts.tif");
    SetDocExampleParameterValue("mask", "mask.tif");
    SetDocExampleParameterValue("allmasks", "allmasks.tif");
    SetDocExampleParameterValue("outdate", "dates.txt");
    SetDocExampleParameterValue("shape", "shape.shp");
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
      m_Concatener = ListConcatenerFilterType::New();
      m_Masks = ListConcatenerFilterType::New();
      m_AllMasks = ListConcatenerFilterType::New();
      m_ImageList = InternalImageListType::New();
      m_MasksList = InternalImageListType::New();
      m_AllMasksList = InternalImageListType::New();
      m_ExtractorList = ExtractROIFilterListType::New();
      m_ResamplersList = ResampleFilterListType::New();
      m_ImageReaderList = ImageReaderListType::New();
      m_BandMathList = BandMathImageFilterListType::New();
      //m_VldMaskList = InternalImageListType::New();
      m_borderMask = BandMathImageFilterType::New();
      m_ShapeBuilder = LabelImageToVectorDataFilterType::New();

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
      // Get the list of input files
      std::vector<std::string> descriptors = this->GetParameterStringList("il");

      if( descriptors.size()== 0 )
        {
        itkExceptionMacro("No input file set...");
        }

      // load all descriptors

      MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
      SPOT4MetadataReaderType::Pointer spot4MetadataReader = SPOT4MetadataReaderType::New();
      for (const std::string& desc : descriptors) {
          ImageDescriptor id;
          if (auto meta = maccsMetadataReader->ReadMetadata(desc)) {
              // add the information to the list
              if (meta->Header.FixedHeader.Mission.find(LANDSAT) != std::string::npos) {
                  // Interpret landsat product
                  ProcessLandsat8Metadata(*meta, desc, id);
              } else if (meta->Header.FixedHeader.Mission.find(SENTINEL) != std::string::npos) {
                  // Interpret sentinel product
                  ProcessSentinel2Metadata(*meta, desc, id);
              } else {
                  itkExceptionMacro("Unknown mission: " + meta->Header.FixedHeader.Mission);
              }

              m_DescriptorsList.push_back(id);
          }else if (auto meta = spot4MetadataReader->ReadMetadata(desc)) {
              // add the information to the list
              ProcessSpot4Metadata(*meta, desc, id);

              m_DescriptorsList.push_back(id);
          } else {
              itkExceptionMacro("Unable to read metadata from " << desc);
          }
      }

      // sort the descriptors after the aquisition date
      std::sort(m_DescriptorsList.begin(), m_DescriptorsList.end(), BandsExtractor::SortMetadata);

      // Build the Border Shape
      if(HasValue("shape"))
          BuildBorderShape();

      // Construct output image and mask
      BuildRasterAndMask();
  }
  //  Software Guide :EndCodeSnippet


  // Process a SPOT4 metadata structure and extract the needed bands and masks.
  void ProcessSpot4Metadata(const SPOT4Metadata& meta, const std::string& filename, ImageDescriptor& descriptor) {

      // Extract the raster date
      descriptor.aquisitionDate = formatSPOT4Date(meta.Header.DatePdv);

      // Save the descriptor fiel path
      descriptor.filename = filename;

      // get the root foloder from the descriptor file name
      std::string rootFolder = extractFolder(filename);
      // get the bands
      std::string imageFile = rootFolder + meta.Files.OrthoSurfCorrPente;
      ImageReaderType::Pointer reader = getReader(imageFile);
      int curRes = reader->GetOutput()->GetSpacing()[0];
      const int wantedRes = 10;
      ExtractROIFilterType::Pointer extractor;
      ResampleFilterType::Pointer resampler;

      // Extract the green band
      extractor = getExtractor(reader->GetOutput(), 1);
      // resample from 20m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.imgG = resampler->GetOutput();

      // Extract the red band
      extractor = getExtractor(reader->GetOutput(), 2);
      // resample from 20m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.imgR = resampler->GetOutput();

      // Extract the NIR band
      extractor = getExtractor(reader->GetOutput(), 3);
      // resample from 20m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.imgNIR = resampler->GetOutput();

      // Extract the SWIR band
      extractor = getExtractor(reader->GetOutput(), 4);
      // resample from 20m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.imgSWIR = resampler->GetOutput();

      // Get the validity mask
      std::string maskFileDiv = rootFolder + meta.Files.MaskDiv;
      ImageReaderType::Pointer maskReaderDiv = getReader(maskFileDiv);
      curRes = maskReaderDiv->GetOutput()->GetSpacing()[0];

      extractor = getExtractor(maskReaderDiv->GetOutput(), 1);

      BandMathImageFilterType::Pointer maskMath = BandMathImageFilterType::New();
      maskMath->SetNthInput(0, extractor->GetOutput());
      maskMath->SetExpression("((b1 == 0) || (b1-(rint(b1/2-0.01)*2) == 0)) ? 1 : 0 ");
      maskMath->UpdateOutputInformation();
      m_BandMathList->PushBack(maskMath);
      // resample from 20m to 10m
      resampler = getResampler(maskMath->GetOutput(), curRes, wantedRes);
      descriptor.maskValid = resampler->GetOutput();

      // Get the saturation mask
      std::string maskFileSat = rootFolder + meta.Files.MaskSaturation;
      ImageReaderType::Pointer maskReaderSat = getReader(maskFileSat);
      curRes = maskReaderSat->GetOutput()->GetSpacing()[0];

      extractor = getExtractor(maskReaderSat->GetOutput(), 1);
      // resample from 20m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.maskSat = resampler->GetOutput();

      // Get the clouds mask
      std::string maskFileNua = rootFolder + meta.Files.MaskNua;
      ImageReaderType::Pointer maskReaderNua = getReader(maskFileNua);
      curRes = maskReaderNua->GetOutput()->GetSpacing()[0];

      extractor = getExtractor(maskReaderNua->GetOutput(), 1);
      // resample from 20m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.maskCloud = resampler->GetOutput();

      // Get the water mask

      extractor = getExtractor(maskReaderDiv->GetOutput(), 1);

      BandMathImageFilterType::Pointer maskWaterMath = BandMathImageFilterType::New();
      maskWaterMath->SetNthInput(0, extractor->GetOutput());
      maskWaterMath->SetExpression("((b1 == 2) || (rint(b1/2 - 0,01)-(rint(rint(b1/2 - 0,01)/2-0.01) * 2) == 0)) ? 1 : 0 ");
      maskWaterMath->UpdateOutputInformation();
      m_BandMathList->PushBack(maskWaterMath);
      // resample from 20m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.maskWater = resampler->GetOutput();

      // Get the snow mask

      extractor = getExtractor(maskReaderDiv->GetOutput(), 1);

      BandMathImageFilterType::Pointer maskSnowMath = BandMathImageFilterType::New();
      maskSnowMath->SetNthInput(0, extractor->GetOutput());
      maskSnowMath->SetExpression("((b1 == 4) || (rint(b1/4 - 0,01)-(rint(rint(b1/4 - 0,01)/2-0.01) * 2) == 0)) ? 1 : 0 ");
      maskSnowMath->UpdateOutputInformation();
      m_BandMathList->PushBack(maskSnowMath);
      // resample from 20m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.maskSnow = resampler->GetOutput();

  }

  // Process a LANDSAT8 metadata structure and extract the needed bands and masks.
  void ProcessLandsat8Metadata(const MACCSFileMetadata& meta, const std::string& filename, ImageDescriptor& descriptor) {

      // Extract the raster date
      descriptor.aquisitionDate = meta.InstanceId.AcquisitionDate;

      // Save the descriptor fiel path
      descriptor.filename = filename;

      // get the root foloder from the descriptor file name
      std::string rootFolder = extractFolder(filename);
      // get the bands
      std::string imageFile = getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE");
      ImageReaderType::Pointer reader = getReader(imageFile);
      int curRes = reader->GetOutput()->GetSpacing()[0];
      const int wantedRes = 10;
      ExtractROIFilterType::Pointer extractor;
      ResampleFilterType::Pointer resampler;

      // Extract the green band
      extractor = getExtractor(reader->GetOutput(), 3);
      // resample from 30m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.imgG = resampler->GetOutput();

      // Extract the red band
      extractor = getExtractor(reader->GetOutput(), 4);
      // resample from 30m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.imgR = resampler->GetOutput();

      // Extract the NIR band
      extractor = getExtractor(reader->GetOutput(), 5);
      // resample from 30m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.imgNIR = resampler->GetOutput();

      // Extract the SWIR band
      extractor = getExtractor(reader->GetOutput(), 6);
      // resample from 30m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.imgSWIR = resampler->GetOutput();

      // Get the validity mask
      std::string maskFileQuality = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_QLT");
      ImageReaderType::Pointer maskReaderQuality = getReader(maskFileQuality);
      curRes = maskReaderQuality->GetOutput()->GetSpacing()[0];

      extractor = getExtractor(maskReaderQuality->GetOutput(), 3);

      BandMathImageFilterType::Pointer maskMath = BandMathImageFilterType::New();
      maskMath->SetNthInput(0, extractor->GetOutput());
      maskMath->SetExpression("((b1 == 0) || (b1-(rint(b1/2-0.01)*2) == 0)) ? 1 : 0 ");
      maskMath->UpdateOutputInformation();
      m_BandMathList->PushBack(maskMath);
      // resample from 30m to 10m
      resampler = getResampler(maskMath->GetOutput(), curRes, wantedRes);
      descriptor.maskValid = resampler->GetOutput();

      // Get the saturation mask
      extractor = getExtractor(maskReaderQuality->GetOutput(), 1);

      // resample from 30m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.maskSat = resampler->GetOutput();

      // Get the cloud mask
      std::string maskFileCloud = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_CLD");
      ImageReaderType::Pointer maskReaderCloud = getReader(maskFileCloud);
      curRes = maskReaderCloud->GetOutput()->GetSpacing()[0];

      extractor = getExtractor(maskReaderCloud->GetOutput(), 1);

      // resample from 30m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.maskCloud = resampler->GetOutput();

      // Get the water mask
      std::string maskFileWaterSnow = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_MSK");
      ImageReaderType::Pointer maskReaderWaterSnow = getReader(maskFileWaterSnow);
      curRes = maskReaderWaterSnow->GetOutput()->GetSpacing()[0];

      extractor = getExtractor(maskReaderWaterSnow->GetOutput(), 1);

      BandMathImageFilterType::Pointer maskWaterMath = BandMathImageFilterType::New();
      maskWaterMath->SetNthInput(0, extractor->GetOutput());
      maskWaterMath->SetExpression("((b1 == 0) || (b1-(rint(b1/2-0.01)*2) == 0)) ? 1 : 0 ");
      maskWaterMath->UpdateOutputInformation();
      m_BandMathList->PushBack(maskWaterMath);

      // resample from 30m to 10m
      resampler = getResampler(maskWaterMath->GetOutput(), curRes, wantedRes);
      descriptor.maskWater = resampler->GetOutput();

      // Get the snow mask
      extractor = getExtractor(maskReaderWaterSnow->GetOutput(), 1);

      BandMathImageFilterType::Pointer maskSnowMath = BandMathImageFilterType::New();
      maskSnowMath->SetNthInput(0, extractor->GetOutput());
      maskSnowMath->SetExpression("((b1 == 32) || (rint(b1/32 - 0,01)-(rint(rint(b1/32 - 0,01)/2-0.01) * 2) == 0)) ? 1 : 0 ");
      maskSnowMath->UpdateOutputInformation();
      m_BandMathList->PushBack(maskSnowMath);

      // resample from 30m to 10m
      resampler = getResampler(maskSnowMath->GetOutput(), curRes, wantedRes);
      descriptor.maskSnow = resampler->GetOutput();

  }


  // Process a SENTINEL2 metadata structure and extract the needed bands and masks.
  void ProcessSentinel2Metadata(const MACCSFileMetadata& meta, const std::string& filename, ImageDescriptor& descriptor) {

      // Extract the raster date
      descriptor.aquisitionDate = meta.InstanceId.AcquisitionDate;

      // Save the descriptor fiel path
      descriptor.filename = filename;

      // get the root foloder from the descriptor file name
      std::string rootFolder = extractFolder(filename);

      ExtractROIFilterType::Pointer extractor;
      ResampleFilterType::Pointer resampler;

      //Extract the first 3 bands form the first file. No resampling needed
      std::string imageFile1 = getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE_R1");
      ImageReaderType::Pointer reader1 = getReader(imageFile1);

      // Extract Green band
      int gIndex = getBandIndex(meta.ImageInformation.Resolutions[0].Bands, "B3");
      extractor = getExtractor(reader1->GetOutput(), gIndex);
      descriptor.imgG = extractor->GetOutput();

      // Extract Red band
      int rIndex = getBandIndex(meta.ImageInformation.Resolutions[0].Bands, "B4");
      extractor = getExtractor(reader1->GetOutput(), rIndex);
      descriptor.imgR = extractor->GetOutput();

      // Extract NIR band
      int nirIndex = getBandIndex(meta.ImageInformation.Resolutions[0].Bands, "B8");
      extractor = getExtractor(reader1->GetOutput(), nirIndex);
      descriptor.imgNIR = extractor->GetOutput();

      //Extract the last band form the second file. Resampling needed.
      std::string imageFile2 = getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE_R2");
      ImageReaderType::Pointer reader2 = getReader(imageFile2);
      int curRes = reader2->GetOutput()->GetSpacing()[0];
      const int wantedRes = 10;

      // Extract SWIR band
      int swirIndex = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B11");
      extractor = getExtractor(reader2->GetOutput(), swirIndex);
      // resample from 20m to 10m
      resampler = getResampler(extractor->GetOutput(), curRes, wantedRes);
      descriptor.imgSWIR = resampler->GetOutput();

      // Get the validity mask
      std::string maskFileQuality = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_QLT_R1");
      ImageReaderType::Pointer maskReaderQuality = getReader(maskFileQuality);

      extractor = getExtractor(maskReaderQuality->GetOutput(), 3);

      BandMathImageFilterType::Pointer maskMath = BandMathImageFilterType::New();
      maskMath->SetNthInput(0, extractor->GetOutput());
      maskMath->SetExpression("((b1 == 0) || (b1-(rint(b1/2-0.01)*2) == 0)) ? 1 : 0");
      maskMath->UpdateOutputInformation();
      m_BandMathList->PushBack(maskMath);
      descriptor.maskValid = maskMath->GetOutput();

      // Get the saturation mask
      extractor = getExtractor(maskReaderQuality->GetOutput(), 1);
      descriptor.maskSat = extractor->GetOutput();

      // Get the cloud mask
      std::string maskFileCloud = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_CLD_R1");
      ImageReaderType::Pointer maskReaderCloud = getReader(maskFileCloud);
      extractor = getExtractor(maskReaderCloud->GetOutput(), 1);
      descriptor.maskCloud = extractor->GetOutput();

      // Get the water mask
      std::string maskFileWaterSnow = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_MSK_R1");
      ImageReaderType::Pointer maskReaderWaterSnow = getReader(maskFileWaterSnow);
      extractor = getExtractor(maskReaderWaterSnow->GetOutput(), 1);

      BandMathImageFilterType::Pointer maskWaterMath = BandMathImageFilterType::New();
      maskWaterMath->SetNthInput(0, extractor->GetOutput());
      maskWaterMath->SetExpression("((b1 == 0) || (b1-(rint(b1/2-0.01)*2) == 0)) ? 1 : 0 ");
      maskWaterMath->UpdateOutputInformation();
      m_BandMathList->PushBack(maskWaterMath);
      descriptor.maskWater = maskWaterMath->GetOutput();

      // Get the snow mask
      extractor = getExtractor(maskReaderWaterSnow->GetOutput(), 1);

      BandMathImageFilterType::Pointer maskSnowMath = BandMathImageFilterType::New();
      maskSnowMath->SetNthInput(0, extractor->GetOutput());
      maskSnowMath->SetExpression("((b1 == 32) || (rint(b1/32 - 0,01)-(rint(rint(b1/32 - 0,01)/2-0.01) * 2) == 0)) ? 1 : 0 ");
      maskSnowMath->UpdateOutputInformation();
      m_BandMathList->PushBack(maskSnowMath);
      descriptor.maskSnow = maskSnowMath->GetOutput();
  }

  // Get all validity masks and build a validity shape which will be comon for all rasters
  void BuildBorderShape() {
      float sum = 0.0;
      std::string expression = "(0";
      int counter = 0;

      for (const ImageDescriptor& desc : m_DescriptorsList) {
          // compute mean value
          StreamingStatisticsImageFilterType::Pointer statsEstimator = StreamingStatisticsImageFilterType::New();
          statsEstimator->SetInput(desc.maskValid);
          statsEstimator->Update();

          sum += statsEstimator->GetMean();
          m_borderMask->SetNthInput(counter, desc.maskValid);
          expression += "+b" + std::to_string(++counter);
      }

      expression += ") >= " + std::to_string(sum) + " ? 1 : 0";

      m_borderMask->SetExpression(expression);

      // polygonize the border mask
      m_ShapeBuilder->SetInput(m_borderMask->GetOutput());
      m_ShapeBuilder->SetInputMask(m_borderMask->GetOutput());

      SetParameterOutputVectorData("shape", m_ShapeBuilder->GetOutput());

      //      m_ShapeBuilder->Update();

      //      LabelImageToVectorDataFilterType::VectorDataPointerType vd = m_ShapeBuilder->GetOutput();

      //      // Get the projection ref of the current VectorData
      //      std::string projectionRefWkt = vd->GetProjectionRef();
      //      bool        projectionInformationAvailable = !projectionRefWkt.empty();
      //      OGRSpatialReference * oSRS = NULL;

      //      if (projectionInformationAvailable)
      //        {
      //        oSRS = static_cast<OGRSpatialReference *>(OSRNewSpatialReference(projectionRefWkt.c_str()));
      //        }
      //      else
      //        {
      //        otbMsgDevMacro(<< "Projection information unavailable");
      //        }

      //      // Retrieving root node
      //      auto tree = vd->GetDataTree();

      //      // Get the input tree root
      //      OGRIOHelper::InternalTreeNodeType * inputRoot = const_cast<OGRIOHelper::InternalTreeNodeType *>(tree->GetRoot());

      //      // Iterative method to build the layers from a VectorData
      //      OGRRegisterAll();
      //      OGRLayer *   ogrCurrentLayer = NULL;
      //      std::vector<OGRLayer *> ogrLayerVector;
      //      otb::OGRIOHelper::Pointer IOConversion = otb::OGRIOHelper::New();

      //      // The method ConvertDataTreeNodeToOGRLayers create the
      //      // OGRDataSource but don t release it. Destruction is done in the
      //      // desctructor
      //      OGRDataSource* OGRDataSourcePointer = NULL;
      //      ogrLayerVector = IOConversion->ConvertDataTreeNodeToOGRLayers(inputRoot,
      //                                                                    OGRDataSourcePointer,
      //                                                                    ogrCurrentLayer,
      //                                                                    oSRS);

      //      //get the extent of the first layer
      //      OGREnvelope extent;
      //      ogrLayerVector[0]->GetExtent(&extent);

      //      // get the name of the file where the dates are written
      //      std::string datesFileName = GetParameterString("extent");

      //      //open the file
      //      std::ofstream extentFile;
      //      extentFile.open(datesFileName);
      //      if (!extentFile.is_open()) {
      //          itkExceptionMacro("Can't open extent file for writing!");
      //      }

      //      // write the data
      //      extentFile << extent.MinX << " " << extent.MinY<< " " << extent.MaxX << " " << extent.MaxY << " " << std::endl;

      //      // close the file
      //      extentFile.close();

      //      // destroy the data source
      //      OGRDataSource::DestroyDataSource(OGRDataSourcePointer);
  }

  void BuildRasterAndMask() {
      // get the name of the file where the dates are written
      std::string datesFileName = GetParameterString("outdate");

      //open the file
      std::ofstream datesFile;
      datesFile.open(datesFileName);
      if (!datesFile.is_open()) {
          itkExceptionMacro("Can't open dates file for writing!");
      }


      // interpret the descriptors and extract the required bands from the atached images
      for ( const ImageDescriptor& desc : m_DescriptorsList) {
          // write the date to the output file
          datesFile << desc.aquisitionDate << std::endl;

          // add the bans to the image list
          m_ImageList->PushBack(desc.imgG);
          m_ImageList->PushBack(desc.imgR);
          m_ImageList->PushBack(desc.imgNIR);
          m_ImageList->PushBack(desc.imgSWIR);

           if(HasValue("mask")) {
              // build the corresponding mask
              BandMathImageFilterType::Pointer maskMath = BandMathImageFilterType::New();
              maskMath->SetNthInput(0, desc.maskCloud);
              maskMath->SetNthInput(1, desc.maskSat);
              maskMath->SetNthInput(2, desc.maskValid);
              maskMath->SetNthInput(3, m_borderMask->GetOutput());

#ifdef OTB_MUPARSER_HAS_CXX_LOGICAL_OPERATORS
              m_maskExpression = "(b1 == 0) && (b2 == 0) && (b3 == 1) && (b4 == 1) ? 0 : 1";
#else
              m_maskExpression = "if((b1 == 0) and (b2 == 0) and (b3 == 1) and (b4 == 1), 0, 1)";
#endif

              maskMath->SetExpression(m_maskExpression);

              m_BandMathList->PushBack(maskMath);

              m_MasksList->PushBack(maskMath->GetOutput());
           }

          if(HasValue("allmasks")) {
              // build the general mask

              BandMathImageFilterType::Pointer allMaskMath = BandMathImageFilterType::New();
              allMaskMath->SetNthInput(0, desc.maskCloud);
              allMaskMath->SetNthInput(1, desc.maskSat);
              allMaskMath->SetNthInput(2, desc.maskValid);
              allMaskMath->SetNthInput(3, desc.maskWater);
              allMaskMath->SetNthInput(4, desc.maskSnow);

#ifdef OTB_MUPARSER_HAS_CXX_LOGICAL_OPERATORS
              m_allMaskExpression = "(b1 == 0) && (b2 == 0) && (b3 == 1) && (b4 == 0) && (b5 == 0) ? 0 : 1";
#else
              m_allMaskExpression = "if((b1 == 0) and (b2 == 0) and (b3 == 1) and && (b4 == 0) and (b5 == 0) , 0, 1)";
#endif

              allMaskMath->SetExpression(m_allMaskExpression);

              m_BandMathList->PushBack(allMaskMath);

              m_AllMasksList->PushBack(allMaskMath->GetOutput());
          }
      }

      // close the dates file
      datesFile.close();

      m_Concatener->SetInput( m_ImageList );
      if(HasValue("mask"))
        m_Masks->SetInput(m_MasksList);
      if(HasValue("allmasks"))
        m_AllMasks->SetInput(m_AllMasksList);

      SetParameterOutputImage("out", m_Concatener->GetOutput());
      SetParameterOutputImage("mask", m_Masks->GetOutput());
      if(HasValue("allmasks"))
          SetParameterOutputImage("allmasks", m_AllMasks->GetOutput());
  }

//  void ExtractMasks() {

//      // Extract the validity masks
//      ExtractValidityMasks();

//      // Compute the border mask
//      float sum = 0.0;
//      std::string expression = "(0";
//      int counter = 0;
//      for (float i : m_MeanPixels) {
//          sum += i;
//          m_borderMask->SetNthInput(counter, m_VldMaskList->GetNthElement(counter));
//          expression += "+b" + std::to_string(++counter);
//      }

//      int threshold = (int)sum;
//      expression += ") >= " + std::to_string(threshold) + " ? 1 : 0";

//      m_borderMask->SetExpression(expression);

//      // polygonize the border mask
//      m_ShapeBuilder->SetInput(m_borderMask->GetOutput());
//      m_ShapeBuilder->SetInputMask(m_borderMask->GetOutput());



//      // interpret the descriptors and extract the required bands from the atached images
//      for (const ImageDescriptor& desc : m_DescriptorsList) {

//          if (desc.type == TYPE_MACCS) {
//              if (desc.maccsDescriptor.Header.FixedHeader.Mission.find(LANDSAT) != std::string::npos) {
//                  // Interpret landsat masks
//                  PocessLandsatMasks(desc);
//              } else if (desc.maccsDescriptor.Header.FixedHeader.Mission.find(SENTINEL) != std::string::npos) {
//                  // Interpret sentinel masks
//                  PocessSentinelMasks(desc);
//              } else {
//                  itkExceptionMacro("Unknown mission: " + desc.maccsDescriptor.Header.FixedHeader.Mission);
//              }
//          } else if (desc.type == TYPE_SPOT4) {
//              PocessSpot4Masks(desc);
//          }
//      }
//  }

//  // Loop through all descriptors and extract the pixel validity masks as 0 for invalid pixel and 1 for valid pixel
//  void ExtractValidityMasks() {
//      for (const ImageDescriptor& desc : m_DescriptorsList) {

//          if (desc.type == TYPE_MACCS) {
//              if (desc.maccsDescriptor.Header.FixedHeader.Mission.find(LANDSAT) != std::string::npos) {
//                  // Interpret landsat masks
//                  ExtractLandsatValidityMask(desc);
//              } else if (desc.maccsDescriptor.Header.FixedHeader.Mission.find(SENTINEL) != std::string::npos) {
//                  // Interpret sentinel masks
//                  ExtractSentinelValidityMask(desc);
//              } else {
//                  itkExceptionMacro("Unknown mission: " + desc.maccsDescriptor.Header.FixedHeader.Mission);
//              }
//          } else if (desc.type == TYPE_SPOT4) {
//              ExtractSpot4ValidityMask(desc);
//          }
//      }
//  }

//  void ExtractLandsatValidityMask(const ImageDescriptor& meta) {
//      //TODO: To implement
//  }

//  void ExtractSentinelValidityMask(const ImageDescriptor& meta) {
//      //TODO: To implement
//  }

//  void ExtractSpot4ValidityMask(const ImageDescriptor& meta) {
//      std::string maskFileDiv = getSPOT4MaskFileName(meta, MASK_TYPE_DIV);
//      ImageReaderType::Pointer maskReaderDiv = getReader(maskFileDiv);

//      ExtractROIFilterType::Pointer divMaskExtractor = ExtractROIFilterType::New();
//      divMaskExtractor->SetInput( maskReaderDiv->GetOutput() );
//      divMaskExtractor->SetChannel( 1 );
//      divMaskExtractor->UpdateOutputInformation();
//      m_ExtractorList->PushBack( divMaskExtractor );

//      BandMathImageFilterType::Pointer maskMath = BandMathImageFilterType::New();
//      maskMath->SetNthInput(0, divMaskExtractor->GetOutput());
//      maskMath->SetExpression("((b1 == 0) || (b1-rint(b1/2-0.01) == 0)) ? 1 : 0 ");
//      m_BandMathList->PushBack(maskMath);

//      // compute mean value
//      StreamingStatisticsImageFilterType::Pointer statsEstimator = StreamingStatisticsImageFilterType::New();
//      statsEstimator->SetInput(maskMath->GetOutput());
//      statsEstimator->Update();

//      m_MeanPixels.push_back(statsEstimator->GetMean());

//      m_VldMaskList->PushBack(maskMath->GetOutput());

//  }

//  void PocessSpot4Masks(const ImageDescriptor& meta) {
//      // create the mask
//      std::string maskFileNua = getSPOT4MaskFileName(meta, MASK_TYPE_NUA);
//      ImageReaderType::Pointer maskReaderNua = getReader(maskFileNua);
//      std::string maskFileSat = getSPOT4MaskFileName(meta, MASK_TYPE_SAT);
//      ImageReaderType::Pointer maskReaderSat = getReader(maskFileSat);
//      std::string maskFileDiv = getSPOT4MaskFileName(meta, MASK_TYPE_DIV);
//      ImageReaderType::Pointer maskReaderDiv = getReader(maskFileDiv);

//      BandMathImageFilterType::Pointer maskMath = BandMathImageFilterType::New();

//      ExtractROIFilterType::Pointer nuaMaskExtractor = ExtractROIFilterType::New();
//      nuaMaskExtractor->SetInput( maskReaderNua->GetOutput() );
//      nuaMaskExtractor->SetChannel( 1 );
//      nuaMaskExtractor->UpdateOutputInformation();
//      m_ExtractorList->PushBack( nuaMaskExtractor );

//      ExtractROIFilterType::Pointer satMaskExtractor = ExtractROIFilterType::New();
//      satMaskExtractor->SetInput( maskReaderSat->GetOutput() );
//      satMaskExtractor->SetChannel( 1 );
//      satMaskExtractor->UpdateOutputInformation();
//      m_ExtractorList->PushBack( satMaskExtractor );

//      ExtractROIFilterType::Pointer divMaskExtractor = ExtractROIFilterType::New();
//      divMaskExtractor->SetInput( maskReaderDiv->GetOutput() );
//      divMaskExtractor->SetChannel( 1 );
//      divMaskExtractor->UpdateOutputInformation();
//      m_ExtractorList->PushBack( divMaskExtractor );

//      maskMath->SetNthInput(0, nuaMaskExtractor->GetOutput());
//      maskMath->SetNthInput(1, satMaskExtractor->GetOutput());
//      maskMath->SetNthInput(2, divMaskExtractor->GetOutput());
//      maskMath->SetNthInput(3, m_borderMask->GetOutput());

//#ifdef OTB_MUPARSER_HAS_CXX_LOGICAL_OPERATORS
//      m_maskExpression = "(b1 == 0) && (b2 == 0) && (b3 == 0) && (b4 == 1) ? 0 : 1";
//#else
//      m_maskExpression = "if((b1 == 0) and (b2 == 0) and (b3 == 0) and (b4 == 1), 0, 1)";
//#endif

//      maskMath->SetExpression(m_maskExpression);

//      m_BandMathList->PushBack(maskMath);

//      m_MasksList->PushBack(maskMath->GetOutput());
//  }

//  void PocessLandsatMasks(const ImageDescriptor& meta) {
//      //TODO: To implement
//      // create the mask
//      std::string maskFile = getMACCSImageFileName(meta.filename, meta.maccsDescriptor.ProductOrganization.AnnexFiles, "_MSK");
//      ImageReaderType::Pointer maskReader = getReader(maskFile);
//      ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
//      extractor->SetInput( maskReader->GetOutput() );
//      extractor->SetChannel( 1 );
//      extractor->UpdateOutputInformation();
//      m_ExtractorList->PushBack( extractor );


//      // resample from 30m to 10m
//      ResampleFilterType::Pointer maskResampler = getResampler(extractor->GetOutput(), 3.0);
//      m_MasksList->PushBack(maskResampler->GetOutput());

//  }

//  void PocessSentinelMasks(const ImageDescriptor& meta) {
//      //TODO: To implement
//      // create the mask
//      std::string maskFile = getMACCSImageFileName(meta.filename, meta.maccsDescriptor.ProductOrganization.AnnexFiles, "_MSK");
//      ImageReaderType::Pointer maskReader = getReader(maskFile);
//      ExtractROIFilterType::Pointer maskExtractor = ExtractROIFilterType::New();
//      maskExtractor->SetInput( maskReader->GetOutput() );
//      maskExtractor->SetChannel( 1 );
//      maskExtractor->UpdateOutputInformation();
//      m_ExtractorList->PushBack( maskExtractor );

//      m_MasksList->PushBack(maskExtractor->GetOutput());
//  }

  // Sort the descriptors based on the aquisition date
  static bool SortMetadata(const ImageDescriptor& o1, const ImageDescriptor& o2) {
      return o1.aquisitionDate.compare(o2.aquisitionDate) < 0;
  }

//  // Process a Landsat8 image.
//  void PocessLandsatImage(const ImageDescriptor& meta) {
//      // Only MACCS format is supported
//      if (meta.type != TYPE_MACCS) {
//          itkExceptionMacro("Unsupported LANDSAT8 image format for descritor: " + meta.filename);
//      }
//      // The Landsat8 images contains only one resolution described in one file.
//      // load the file in a reader
//      std::string imageFile = getMACCSImageFileName(meta.filename, meta.maccsDescriptor.ProductOrganization.ImageFiles, "_FRE");
//      ImageReaderType::Pointer reader = getReader(imageFile);

//      // the required bands are:
//      // b3 - G (Name B3)
//      // b4 - R (Name B4)
//      // b5 - NIR (Name B5)
//      // b6 - SWIR (Name B6)
//      for (int i = 3; i <= 6; i++) {
//          std::string bandName = "B" + std::to_string(i);
//          int index = getBandIndex(meta.maccsDescriptor.ImageInformation.Bands, bandName);

//          ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
//          extractor->SetInput( reader->GetOutput() );
//          extractor->SetChannel( index );
//          extractor->UpdateOutputInformation();
//          m_ExtractorList->PushBack( extractor );

//          // resample from 30m to 10m
//          ResampleFilterType::Pointer resampler = getResampler(extractor->GetOutput(), 3.0);

//          m_ImageList->PushBack( resampler->GetOutput() );
//      }
//  }


//  // Process a Spot4 image
//  void PocessSpot4Image(const ImageDescriptor& meta) {
//      // The Spot4 images contains only one resolution described in one file.
//      // No filtering is required.
//      // load the file in a reader
//      std::string imageFile = getSPOT4ImageFileName(meta);
//      ImageReaderType::Pointer reader = getReader(imageFile);

//      // the required bands are:
//      // b1 - G (Name B3)
//      // b2 - R (Name B4)
//      // b3 - NIR (Name B5)
//      // b4 - SWIR (Name B6)
//      for (int i = 1; i <= 4; i++) {

//          ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
//          extractor->SetInput( reader->GetOutput() );
//          extractor->SetChannel( i );
//          extractor->UpdateOutputInformation();
//          m_ExtractorList->PushBack( extractor );

//          m_ImageList->PushBack( extractor->GetOutput() );
//      }
//  }

//  // Process a Sentinel2 image
//  void PocessSentinelImage(const ImageDescriptor& meta) {
//      // Only MACCS format is supported
//      if (meta.type != TYPE_MACCS) {
//          itkExceptionMacro("Unsupported Sentinel2 image format for descritor: " + meta.filename);
//      }
//      // The Sentinel2 images are described in 2 files, one with 10m resolution and one with 20m resolution
//      // The 10m resolution file contains the G, R and NIR bands which can be used unmodified
//      // The 20m resolution file contains the SWIR band which must be resampled.

//      //Extract the first 3 bands form the first file. No resampling needed
//      std::string imageFile1 = getMACCSImageFileName(meta.filename, meta.maccsDescriptor.ProductOrganization.ImageFiles, "_FRE_R1");
//      ImageReaderType::Pointer reader1 = getReader(imageFile1);

//      // Extract Green band
//      int gIndex = getBandIndex(meta.maccsDescriptor.ImageInformation.Resolutions[0].Bands, "B3");

//      ExtractROIFilterType::Pointer gExtractor = ExtractROIFilterType::New();
//      gExtractor->SetInput( reader1->GetOutput() );
//      gExtractor->SetChannel( gIndex );
//      gExtractor->UpdateOutputInformation();
//      m_ExtractorList->PushBack( gExtractor );

//      m_ImageList->PushBack( gExtractor->GetOutput() );

//      // Extract Red band
//      int rIndex = getBandIndex(meta.maccsDescriptor.ImageInformation.Resolutions[0].Bands, "B4");

//      ExtractROIFilterType::Pointer rExtractor = ExtractROIFilterType::New();
//      rExtractor->SetInput( reader1->GetOutput() );
//      rExtractor->SetChannel( rIndex );
//      rExtractor->UpdateOutputInformation();
//      m_ExtractorList->PushBack( rExtractor );

//      m_ImageList->PushBack( rExtractor->GetOutput() );

//      // Extract NIR band
//      int nirIndex = getBandIndex(meta.maccsDescriptor.ImageInformation.Resolutions[0].Bands, "B8");

//      ExtractROIFilterType::Pointer nirExtractor = ExtractROIFilterType::New();
//      nirExtractor->SetInput( reader1->GetOutput() );
//      nirExtractor->SetChannel( nirIndex );
//      nirExtractor->UpdateOutputInformation();
//      m_ExtractorList->PushBack( nirExtractor );

//      m_ImageList->PushBack( nirExtractor->GetOutput() );

//      //Extract the last band form the second file. Resampling needed.
//      std::string imageFile2 = getMACCSImageFileName(meta.filename, meta.maccsDescriptor.ProductOrganization.ImageFiles, "_FRE_R2");
//      ImageReaderType::Pointer reader2 = getReader(imageFile2);

//      // Extract SWIR band
//      int swirIndex = getBandIndex(meta.maccsDescriptor.ImageInformation.Resolutions[1].Bands, "B11");

//      ExtractROIFilterType::Pointer swirExtractor = ExtractROIFilterType::New();
//      swirExtractor->SetInput( reader2->GetOutput() );
//      swirExtractor->SetChannel( swirIndex );
//      swirExtractor->UpdateOutputInformation();
//      m_ExtractorList->PushBack( swirExtractor );

//      // resample from 20m to 10m
//      ResampleFilterType::Pointer resampler = getResampler(swirExtractor->GetOutput(), 2.0);

//      m_ImageList->PushBack( resampler->GetOutput() );

//  }

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

  ExtractROIFilterType::Pointer getExtractor (const ImageType::Pointer& image, const unsigned int chanel) {
      ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
      extractor->SetInput( image );
      extractor->SetChannel( chanel );
      extractor->UpdateOutputInformation();
      m_ExtractorList->PushBack( extractor );
      return extractor;
  }

  ResampleFilterType::Pointer getResampler(const InternalImageType::Pointer& image, const int curRes, const int wantedRes) {
       ResampleFilterType::Pointer resampler = ResampleFilterType::New();
       resampler->SetInput(image);
       if(curRes == wantedRes) {
           m_ResamplersList->PushBack(resampler);
           return resampler;
       }
       const float ratio = curRes / wantedRes;

       // Set the interpolator
       LinearInterpolationType::Pointer interpolator = LinearInterpolationType::New();
       resampler->SetInterpolator(interpolator);

       IdentityTransformType::Pointer transform = IdentityTransformType::New();

       resampler->SetOutputParametersFromImage( image );
       // Scale Transform
       OutputVectorType scale;
       scale[0] = 1.0 / ratio;
       scale[1] = 1.0 / ratio;

       // Evaluate spacing
       InternalImageType::SpacingType spacing = image->GetSpacing();
       InternalImageType::SpacingType OutputSpacing;
       OutputSpacing[0] = spacing[0] * scale[0];
       OutputSpacing[1] = spacing[1] * scale[1];

       resampler->SetOutputSpacing(OutputSpacing);

       FloatVectorImageType::PointType origin = image->GetOrigin();
       FloatVectorImageType::PointType outputOrigin;
       outputOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale[0] - 1.0);
       outputOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale[1] - 1.0);

       resampler->SetOutputOrigin(outputOrigin);

       resampler->SetTransform(transform);

       // Evaluate size
       ResampleFilterType::SizeType recomputedSize;
       recomputedSize[0] = image->GetLargestPossibleRegion().GetSize()[0] / scale[0];
       recomputedSize[1] = image->GetLargestPossibleRegion().GetSize()[1] / scale[1];

       resampler->SetOutputSize(recomputedSize);

       m_ResamplersList->PushBack(resampler);
       return resampler;
  }



  inline std::string formatSPOT4Date(const std::string& date) {
      return date.substr(0,4) + date.substr(5,2) + date.substr(8,2);
  }

  ListConcatenerFilterType::Pointer     m_Concatener;
  ListConcatenerFilterType::Pointer     m_Masks;
  ListConcatenerFilterType::Pointer     m_AllMasks;
  ExtractROIFilterListType::Pointer     m_ExtractorList;
  ResampleFilterListType::Pointer       m_ResamplersList;
  InternalImageListType::Pointer        m_ImageList;
  InternalImageListType::Pointer        m_MasksList;
  InternalImageListType::Pointer        m_AllMasksList;
  ImageReaderListType::Pointer          m_ImageReaderList;
  std::vector<ImageDescriptor>          m_DescriptorsList;
  BandMathImageFilterListType::Pointer  m_BandMathList;  
  std::string                           m_maskExpression;
  std::string                           m_allMaskExpression;

//  InternalImageListType::Pointer        m_VldMaskList;
//  std::vector<float>                    m_MeanPixels;
  BandMathImageFilterType::Pointer      m_borderMask;

  LabelImageToVectorDataFilterType::Pointer m_ShapeBuilder;
};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::BandsExtractor)
//  Software Guide :EndCodeSnippet


