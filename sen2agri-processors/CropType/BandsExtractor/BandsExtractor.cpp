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

//Transform
#include "itkScalableAffineTransform.h"
#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

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


struct SourceImageMetadata {
    MACCSFileMetadata  msccsFileMetadata;
    SPOT4Metadata      spot4Metadata;
};

struct ImageDescriptor {
    // the descriptor file
    std::string filename;
    // the aquisition date in format YYYYMMDD
    std::string aquisitionDate;
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
    m_ImageList = InternalImageListType::New();
    m_MasksList = InternalImageListType::New();
    m_ResamplersList = ResampleFilterListType::New();
    m_ExtractorList = ExtractROIFilterListType::New();
    m_ImageReaderList = ImageReaderListType::New();
    m_BandMathList = BandMathImageFilterListType::New();

    //  Software Guide : BeginCodeSnippet
    AddParameter(ParameterType_InputFilenameList, "il", "The xml files");

    AddParameter(ParameterType_OutputImage, "out", "The concatenated images");
    AddParameter(ParameterType_OutputImage, "mask", "The concatenated masks");
    AddParameter(ParameterType_OutputFilename, "outdate", "The file containing the dates for the images");

     //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
    // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("il", "image1.xml image2.xml");
    SetDocExampleParameterValue("out", "fts.tif");
    SetDocExampleParameterValue("mask", "mask.tif");
    SetDocExampleParameterValue("outdate", "dates.txt");
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
      m_ImageList = InternalImageListType::New();
      m_MasksList = InternalImageListType::New();
      m_ExtractorList = ExtractROIFilterListType::New();
      m_ResamplersList = ResampleFilterListType::New();
      m_ImageReaderList = ImageReaderListType::New();
      m_BandMathList = BandMathImageFilterListType::New();

#ifdef OTB_MUPARSER_HAS_CXX_LOGICAL_OPERATORS
      m_maskExpression = "((b3 != 1) && (b1 == 0) && (b2 == 0) ? 0 : 1)";
#else
      m_maskExpression = "if(b3==1,1,if(b1==0,if(b2==0,0,1),1))";
#endif
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
              id.filename = desc;
              id.type = TYPE_MACCS;
              id.aquisitionDate = meta->InstanceId.AcquisitionDate;
              id.maccsDescriptor = *meta;

              m_DescriptorsList.push_back(id);
          }else if (auto meta = spot4MetadataReader->ReadMetadata(desc)) {
              // add the information to the list
              id.filename = desc;
              id.type = TYPE_SPOT4;
              id.aquisitionDate = formatSPOT4Date(meta->Header.DatePdv);
              id.spot4Descriptor = *meta;

              m_DescriptorsList.push_back(id);
          } else {
              itkExceptionMacro("Unable to read metadata from " << desc);
          }
      }

      // sort the descriptors after the aquisition date
      std::sort(m_DescriptorsList.begin(), m_DescriptorsList.end(), BandsExtractor::SortMetadata);

      // get the name of the file where the dates are written
      std::string datesFileName = GetParameterString("outdate");

      //open the file
      std::ofstream datesFile;
      datesFile.open(datesFileName);
      if (!datesFile.is_open()) {
          itkExceptionMacro("Can't open dates file for writing!");
      }

      // interpret the descriptors and extract the required bands from the atached images
      for (const ImageDescriptor& desc : m_DescriptorsList) {
          // write the date to the output file
          datesFile << desc.aquisitionDate << std::endl;

          if (desc.type == TYPE_MACCS) {
              if (desc.maccsDescriptor.Header.FixedHeader.Mission.find(LANDSAT) != std::string::npos) {
                  // Interpret landsat image
                  PocessLandsatImage(desc);
              } else if (desc.maccsDescriptor.Header.FixedHeader.Mission.find(SENTINEL) != std::string::npos) {
                  // Interpret sentinel image
                  PocessSentinelImage(desc);
              } else {
                  itkExceptionMacro("Unknown mission: " + desc.maccsDescriptor.Header.FixedHeader.Mission);
              }
          } else if (desc.type == TYPE_SPOT4) {
              PocessSpot4Image(desc);
          }
      }

      // close the dates file
      datesFile.close();

      m_Concatener->SetInput( m_ImageList );
      m_Masks->SetInput(m_MasksList);

      SetParameterOutputImage("out", m_Concatener->GetOutput());
      SetParameterOutputImage("mask", m_Masks->GetOutput());
  }
  //  Software Guide :EndCodeSnippet

  // Sort the descriptors based on the aquisition date
  static bool SortMetadata(const ImageDescriptor& o1, const ImageDescriptor& o2) {
      return o1.aquisitionDate.compare(o2.aquisitionDate) < 0;
  }

  // Process a Landsat8 image.
  void PocessLandsatImage(const ImageDescriptor& meta) {
      // Only MACCS format is supported
      if (meta.type != TYPE_MACCS) {
          itkExceptionMacro("Unsupported LANDSAT8 image format for descritor: " + meta.filename);
      }
      // The Landsat8 images contains only one resolution described in one file.
      // load the file in a reader
      std::string imageFile = getMACCSImageFileName(meta.filename, meta.maccsDescriptor.ProductOrganization.ImageFiles, "_FRE");
      ImageReaderType::Pointer reader = getReader(imageFile);

      // the required bands are:
      // b3 - G (Name B3)
      // b4 - R (Name B4)
      // b5 - NIR (Name B5)
      // b6 - SWIR (Name B6)
      for (int i = 3; i <= 6; i++) {
          std::string bandName = "B" + std::to_string(i);
          int index = getBandIndex(meta.maccsDescriptor.ImageInformation.Bands, bandName);

          ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
          extractor->SetInput( reader->GetOutput() );
          extractor->SetChannel( index );
          extractor->UpdateOutputInformation();
          m_ExtractorList->PushBack( extractor );

          // resample from 30m to 10m
          ResampleFilterType::Pointer resampler = getResampler(extractor->GetOutput(), 3.0);

          m_ImageList->PushBack( resampler->GetOutput() );
      }

      // create the mask
      std::string maskFile = getMACCSImageFileName(meta.filename, meta.maccsDescriptor.ProductOrganization.AnnexFiles, "_MSK");
      ImageReaderType::Pointer maskReader = getReader(maskFile);
      ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
      extractor->SetInput( maskReader->GetOutput() );
      extractor->SetChannel( 1 );
      extractor->UpdateOutputInformation();
      m_ExtractorList->PushBack( extractor );


      // resample from 30m to 10m
      ResampleFilterType::Pointer maskResampler = getResampler(extractor->GetOutput(), 3.0);
      m_MasksList->PushBack(maskResampler->GetOutput());
  }


  // Process a Spot4 image
  void PocessSpot4Image(const ImageDescriptor& meta) {
      // The Spot4 images contains only one resolution described in one file.
      // No filtering is required.
      // load the file in a reader
      std::string imageFile = getSPOT4ImageFileName(meta);
      ImageReaderType::Pointer reader = getReader(imageFile);

      // the required bands are:
      // b1 - G (Name B3)
      // b2 - R (Name B4)
      // b3 - NIR (Name B5)
      // b4 - SWIR (Name B6)
      for (int i = 1; i <= 4; i++) {

          ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
          extractor->SetInput( reader->GetOutput() );
          extractor->SetChannel( i );
          extractor->UpdateOutputInformation();
          m_ExtractorList->PushBack( extractor );

          m_ImageList->PushBack( extractor->GetOutput() );
      }

      // create the mask
      std::string maskFileNua = getSPOT4MaskFileName(meta, MASK_TYPE_NUA);
      ImageReaderType::Pointer maskReaderNua = getReader(maskFileNua);
      std::string maskFileSat = getSPOT4MaskFileName(meta, MASK_TYPE_SAT);
      ImageReaderType::Pointer maskReaderSat = getReader(maskFileSat);
      std::string maskFileDiv = getSPOT4MaskFileName(meta, MASK_TYPE_DIV);
      ImageReaderType::Pointer maskReaderDiv = getReader(maskFileDiv);

      BandMathImageFilterType::Pointer maskMath = BandMathImageFilterType::New();

      ExtractROIFilterType::Pointer nuaMaskExtractor = ExtractROIFilterType::New();
      nuaMaskExtractor->SetInput( maskReaderNua->GetOutput() );
      nuaMaskExtractor->SetChannel( 1 );
      nuaMaskExtractor->UpdateOutputInformation();
      m_ExtractorList->PushBack( nuaMaskExtractor );

      ExtractROIFilterType::Pointer satMaskExtractor = ExtractROIFilterType::New();
      satMaskExtractor->SetInput( maskReaderSat->GetOutput() );
      satMaskExtractor->SetChannel( 1 );
      satMaskExtractor->UpdateOutputInformation();
      m_ExtractorList->PushBack( satMaskExtractor );

      ExtractROIFilterType::Pointer divMaskExtractor = ExtractROIFilterType::New();
      divMaskExtractor->SetInput( maskReaderDiv->GetOutput() );
      divMaskExtractor->SetChannel( 1 );
      divMaskExtractor->UpdateOutputInformation();
      m_ExtractorList->PushBack( divMaskExtractor );


      maskMath->SetNthInput(0, nuaMaskExtractor->GetOutput());
      maskMath->SetNthInput(1, satMaskExtractor->GetOutput());
      maskMath->SetNthInput(2, divMaskExtractor->GetOutput());
      maskMath->SetExpression(m_maskExpression);

      m_BandMathList->PushBack(maskMath);

//      ExtractROIFilterType::Pointer maskExtractor = ExtractROIFilterType::New();
//      maskExtractor->SetInput( maskMath->GetOutput() );
//      maskExtractor->SetChannel( 1 );
//      maskExtractor->UpdateOutputInformation();
//      m_ExtractorList->PushBack( maskExtractor );

      m_MasksList->PushBack(maskMath->GetOutput());
  }

  // Process a Sentinel2 image
  void PocessSentinelImage(const ImageDescriptor& meta) {
      // Only MACCS format is supported
      if (meta.type != TYPE_MACCS) {
          itkExceptionMacro("Unsupported Sentinel2 image format for descritor: " + meta.filename);
      }
      // The Sentinel2 images are described in 2 files, one with 10m resolution and one with 20m resolution
      // The 10m resolution file contains the G, R and NIR bands which can be used unmodified
      // The 20m resolution file contains the SWIR band which must be resampled.

      //Extract the first 3 bands form the first file. No resampling needed
      std::string imageFile1 = getMACCSImageFileName(meta.filename, meta.maccsDescriptor.ProductOrganization.ImageFiles, "_FRE_R1");
      ImageReaderType::Pointer reader1 = getReader(imageFile1);

      // Extract Green band
      int gIndex = getBandIndex(meta.maccsDescriptor.ImageInformation.Resolutions[0].Bands, "B3");

      ExtractROIFilterType::Pointer gExtractor = ExtractROIFilterType::New();
      gExtractor->SetInput( reader1->GetOutput() );
      gExtractor->SetChannel( gIndex );
      gExtractor->UpdateOutputInformation();
      m_ExtractorList->PushBack( gExtractor );

      m_ImageList->PushBack( gExtractor->GetOutput() );

      // Extract Red band
      int rIndex = getBandIndex(meta.maccsDescriptor.ImageInformation.Resolutions[0].Bands, "B4");

      ExtractROIFilterType::Pointer rExtractor = ExtractROIFilterType::New();
      rExtractor->SetInput( reader1->GetOutput() );
      rExtractor->SetChannel( rIndex );
      rExtractor->UpdateOutputInformation();
      m_ExtractorList->PushBack( rExtractor );

      m_ImageList->PushBack( rExtractor->GetOutput() );

      // Extract NIR band
      int nirIndex = getBandIndex(meta.maccsDescriptor.ImageInformation.Resolutions[0].Bands, "B8");

      ExtractROIFilterType::Pointer nirExtractor = ExtractROIFilterType::New();
      nirExtractor->SetInput( reader1->GetOutput() );
      nirExtractor->SetChannel( nirIndex );
      nirExtractor->UpdateOutputInformation();
      m_ExtractorList->PushBack( nirExtractor );

      m_ImageList->PushBack( nirExtractor->GetOutput() );

      //Extract the last band form the second file. Resampling needed.
      std::string imageFile2 = getMACCSImageFileName(meta.filename, meta.maccsDescriptor.ProductOrganization.ImageFiles, "_FRE_R2");
      ImageReaderType::Pointer reader2 = getReader(imageFile2);

      // Extract SWIR band
      int swirIndex = getBandIndex(meta.maccsDescriptor.ImageInformation.Resolutions[1].Bands, "B11");

      ExtractROIFilterType::Pointer swirExtractor = ExtractROIFilterType::New();
      swirExtractor->SetInput( reader2->GetOutput() );
      swirExtractor->SetChannel( swirIndex );
      swirExtractor->UpdateOutputInformation();
      m_ExtractorList->PushBack( swirExtractor );

      // resample from 20m to 10m
      ResampleFilterType::Pointer resampler = getResampler(swirExtractor->GetOutput(), 2.0);

      m_ImageList->PushBack( resampler->GetOutput() );

      // create the mask
      std::string maskFile = getMACCSImageFileName(meta.filename, meta.maccsDescriptor.ProductOrganization.AnnexFiles, "_MSK");
      ImageReaderType::Pointer maskReader = getReader(maskFile);
      ExtractROIFilterType::Pointer maskExtractor = ExtractROIFilterType::New();
      maskExtractor->SetInput( maskReader->GetOutput() );
      maskExtractor->SetChannel( 1 );
      maskExtractor->UpdateOutputInformation();
      m_ExtractorList->PushBack( maskExtractor );

      m_MasksList->PushBack(maskExtractor->GetOutput());
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
  std::string getMACCSImageFileName(const std::string& descriptor, const std::vector<MACCSFileInformation>& imageFiles, const std::string& ending) {

      std::string folder;
      size_t pos = descriptor.find_last_of("/\\");
      if (pos == std::string::npos) {
          folder = "";
      } else {
          folder = descriptor.substr(0, pos);
      }

      for (const MACCSFileInformation& fileInfo : imageFiles) {
          if (fileInfo.LogicalName.length() >= ending.length() &&
                  0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
              return folder + "/" + fileInfo.FileLocation.substr(0, fileInfo.FileLocation.find_last_of('.')) + ".DBL.TIF";
          }

      }
      return "";
  }


  // Return the path to a file for which the name end in the ending
  std::string getMACCSImageFileName(const std::string& descriptor, const std::vector<MACCSAnnexInformation>& maskFiles, const std::string& ending) {

      std::string folder;
      size_t pos = descriptor.find_last_of("/\\");
      if (pos == std::string::npos) {
          folder = "";
      } else {
          folder = descriptor.substr(0, pos);
      }

      for (const MACCSAnnexInformation& fileInfo : maskFiles) {
          if (fileInfo.File.LogicalName.length() >= ending.length() &&
                  0 == fileInfo.File.LogicalName.compare (fileInfo.File.LogicalName.length() - ending.length(), ending.length(), ending)) {
              return folder + "/" + fileInfo.File.FileLocation.substr(0, fileInfo.File.FileLocation.find_last_of('.')) + ".DBL.TIF";
          }

      }
      return "";
  }

  // Return the path to a file for which the name end in the ending
  std::string getSPOT4ImageFileName(const ImageDescriptor& desc) {

      std::string folder;
      size_t pos = desc.filename.find_last_of("/\\");
      if (pos == std::string::npos) {
          return desc.spot4Descriptor.Files.OrthoSurfCorrPente;
      }

      folder = desc.filename.substr(0, pos);
      return folder + "/" + desc.spot4Descriptor.Files.OrthoSurfCorrPente;
  }

  // Return the path to a file for which the name end in the ending
  std::string getSPOT4MaskFileName(const ImageDescriptor& desc, const unsigned char maskType) {

      std::string file;
      switch (maskType) {
      case MASK_TYPE_NUA:
          file = desc.spot4Descriptor.Files.MaskNua;
          break;
      case MASK_TYPE_DIV:
          file = desc.spot4Descriptor.Files.MaskDiv;
          break;
      case MASK_TYPE_SAT:
      default:
          file = desc.spot4Descriptor.Files.MaskSaturation;
          break;
      }

      std::string folder;
      size_t pos = desc.filename.find_last_of("/\\");
      if (pos == std::string::npos) {
          return file;
      }

      folder = desc.filename.substr(0, pos);
      return folder + "/" + file;
  }

  ResampleFilterType::Pointer getResampler(const InternalImageType::Pointer& image, const float& ratio) {
       ResampleFilterType::Pointer resampler = ResampleFilterType::New();
       resampler->SetInput(image);

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
  ExtractROIFilterListType::Pointer     m_ExtractorList;
  ResampleFilterListType::Pointer       m_ResamplersList;
  InternalImageListType::Pointer        m_ImageList;
  InternalImageListType::Pointer        m_MasksList;
  ImageReaderListType::Pointer          m_ImageReaderList;
  std::vector<ImageDescriptor>          m_DescriptorsList;
  BandMathImageFilterListType::Pointer  m_BandMathList;
  std::string                           m_maskExpression;
};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::BandsExtractor)
//  Software Guide :EndCodeSnippet


