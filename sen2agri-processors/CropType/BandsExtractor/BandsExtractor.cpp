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

#include "otbVectorImage.h"
#include "otbImageList.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"

#define LANDSAT    "LANDSAT"
#define SENTINEL   "SENTINEL"


typedef otb::VectorImage<float, 2>                                 ImageType;
typedef otb::Image<float, 2>                                       InternalImageType;

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
typedef std::pair<std::string, MACCSFileMetadata>                  ImageDescriptorPairType;


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
    m_ImageList = InternalImageListType::New();
    m_ExtractorList = ExtractROIFilterListType::New();
    m_ImageReaderList = ImageReaderListType::New();

    //  Software Guide : BeginCodeSnippet
    AddParameter(ParameterType_InputFilenameList, "il", "The xml files");

    AddParameter(ParameterType_OutputImage, "out", "The concatenated images");

     //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
    // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("il", "image1.xml image2.xml");
    SetDocExampleParameterValue("out", "fts.tif");
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
      m_ImageList = InternalImageListType::New();
      m_ExtractorList = ExtractROIFilterListType::New();
      m_ImageReaderList = ImageReaderListType::New();
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
      std::vector<std::string>::iterator descIt;
      for (descIt = descriptors.begin(); descIt != descriptors.end(); ++descIt) {
          MACCSFileMetadata meta = maccsMetadataReader->ReadMetadata(*descIt);
          // add the information to the list
          m_DescriptorsList.push_back(ImageDescriptorPairType(*descIt, meta));
      }

      // sort the descriptors after the aquisition date
      std::sort(m_DescriptorsList.begin(), m_DescriptorsList.end(), BandsExtractor::SortMetadata);


      // interpret the descriptors and extract the required bands from the atached images
      std::vector<ImageDescriptorPairType>::iterator it;
      for (it = m_DescriptorsList.begin(); it != m_DescriptorsList.end(); ++it) {
          ImageDescriptorPairType metaPair = *it;
          if (metaPair.second.Header.FixedHeader.Mission.find(LANDSAT) != std::string::npos) {
              // Interpret landsat image
              PocessLandsatImage(metaPair);
          } else if (metaPair.second.Header.FixedHeader.Mission.find(SENTINEL) != std::string::npos) {
              // Interpret sentinel image
              PocessSentinelImage(metaPair);
          } else {
              itkExceptionMacro("Unknown mission: " + metaPair.second.Header.FixedHeader.Mission);
          }

      }

/*
      inList->GetNthElement(0)->UpdateOutputInformation();
      FloatVectorImageType::SizeType size = inList->GetNthElement(0)->GetLargestPossibleRegion().GetSize();

      // Split each input vector image into image
      // and generate an mono channel image list
      for( unsigned int i=0; i<inList->Size(); i++ )
        {
        FloatVectorImageType::Pointer vectIm = inList->GetNthElement(i);
        vectIm->UpdateOutputInformation();
        if( size != vectIm->GetLargestPossibleRegion().GetSize() )
          {
          itkExceptionMacro("Input Image size mismatch...");
          }

        // get the number of bands
        int numBands = vectIm->GetNumberOfComponentsPerPixel();
        int minBand = numBands == 7 ? 2 : 0;
        int maxBand = numBands == 7 ? 6 : numBands;

        int chNum = 1;
        for( unsigned int j=minBand; j<maxBand; j++)
          {
          ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
          extractor->SetInput( vectIm );
          extractor->SetChannel( chNum++ );
          extractor->UpdateOutputInformation();
          m_ExtractorList->PushBack( extractor );
          m_ImageList->PushBack( extractor->GetOutput() );
          }
        }
*/

      m_Concatener->SetInput( m_ImageList );

      SetParameterOutputImage("out", m_Concatener->GetOutput());
  }
  //  Software Guide :EndCodeSnippet

  // Sort the descriptors based on the aquisition date
  static bool SortMetadata(ImageDescriptorPairType o1, ImageDescriptorPairType o2) {
      return o1.second.InstanceId.AcquisitionDate.compare(o2.second.InstanceId.AcquisitionDate) < 0 ? true : false;
  }

  // Process a Landsat8 image
  void PocessLandsatImage(ImageDescriptorPairType meta) {
      // The Landsat8 images contains only one resolution described in one file.
      // load the file in a reader
      std::string imageFile = getImageFileName(meta.first, meta.second.ImageInformation.ProductOrganization.ImageFiles, "_SRE");
      ImageReaderType::Pointer reader = getReader(imageFile);

      // the required bands are:
      // b3 - G (Name B3)
      // b4 - R (Name B4)
      // b5 - NIR (Name B5)
      // b6 - SWIR (Name B6)
      for (int i = 3; i <= 6; i++) {
          std::string bandName = "B" + std::to_string(i);
          int index = getBandIndex(meta.second.ImageInformation.Bands, bandName);

          ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
          //TODO: resample bands !
          extractor->SetInput( reader->GetOutput() );
          extractor->SetChannel( index );
          extractor->UpdateOutputInformation();
          m_ExtractorList->PushBack( extractor );
          m_ImageList->PushBack( extractor->GetOutput() );
      }
  }

  // Process a Sentinel2 image
  void PocessSentinelImage(ImageDescriptorPairType meta) {
    //todo: process sentinel
  }

  // Get the id of the band. Return -1 if band not found.
  int getBandIndex(std::vector<MACCSBand> bands, std::string name) {
      std::vector<MACCSBand>::iterator it;
      for (it = bands.begin(); it != bands.end(); ++it) {
          if (it->Name == name) {
              return std::stoi(it->Id);
          }
      }
      return -1;
  }

  // get a reader from the file path
  ImageReaderType::Pointer getReader(std::string filePath) {
      ImageReaderType::Pointer reader = ImageReaderType::New();

      // set the file name
      reader->SetFileName(filePath);

      // add it to the list and return
      m_ImageReaderList->PushBack(reader);
      return reader;
  }

  // Return the path to a file for which the name end in the ending
  std::string getImageFileName(std::string descriptor, std::vector<MACCSFileInformation> imageFiles, std::string ending) {

      std::string folder;
      int pos = descriptor.find_last_of("/\\");
      if (pos == std::string::npos) {
          folder = "";
      }

      folder = descriptor.substr(0, pos);

      std::vector<MACCSFileInformation>::iterator it;

      for (it = imageFiles.begin(); it != imageFiles.end(); ++it) {
          if (it->LogicalName.length() >= ending.length() &&
                  0 == it->LogicalName.compare (it->LogicalName.length() - ending.length(), ending.length(), ending)) {
              return folder + it->FileLocation;
          }

      }
      return "";
  }

  ListConcatenerFilterType::Pointer     m_Concatener;
  ExtractROIFilterListType::Pointer     m_ExtractorList;
  InternalImageListType::Pointer        m_ImageList;
  ImageReaderListType::Pointer          m_ImageReaderList;
  std::vector<ImageDescriptorPairType>  m_DescriptorsList;


};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::BandsExtractor)
//  Software Guide :EndCodeSnippet


