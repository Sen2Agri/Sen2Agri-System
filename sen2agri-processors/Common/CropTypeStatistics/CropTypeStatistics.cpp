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
#include "otbWrapperApplicationRegistry.h"
#include "otbWrapperInputImageListParameter.h"

#include "otbVectorImage.h"
#include "otbImageList.h"

#include "otbStreamingResampleImageFilter.h"
#include "otbGridResampleImageFilter.h"

//Transform
#include "itkScalableAffineTransform.h"
//#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

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

#include "../Filters/CropTypePreprocessing.h"

#define TYPE_MACCS  0
#define TYPE_SPOT4  1

#define EPSILON     1e-6


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

  itkTypeMacro(CropTypeStatistics, otb::Application)
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
      const std::vector<std::string> &descriptors = this->GetParameterStringList("il");
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
                  itkExceptionMacro("Invalid number of products " << ppts)
              }
              prodPerTile.push_back(ppt);
              numDesc += ppt;
          }
      } else {
          // All products belong to the same tile
          prodPerTile.push_back(descriptors.size());
          numDesc = descriptors.size();
      }

      // get the required pixel size
      auto pixSize = this->GetParameterFloat("pixsize");
      // get the main mission
      std::string mission = SPOT;
      if (HasValue("mission")) {
          mission = this->GetParameterString("mission");
      }

      if( descriptors.size() != numDesc )
        {
        itkExceptionMacro("The number of descriptors (" << descriptors.size() << ") is not consistent with the sum of products per tile (" << numDesc << ")")
        }


      auto preprocessors = CropTypePreprocessingList::New();

      // Loop through the sets of products
      int startIndex = 0;
      int endIndex;
      for (size_t i = 0; i < prodPerTile.size(); i ++) {
          int sz = prodPerTile[i];
          endIndex = startIndex + sz;
          TileData td;


          auto preprocessor = CropTypePreprocessing::New();
          preprocessors->PushBack(preprocessor);
          preprocessor->SetPixelSize(pixSize);
          preprocessor->SetMission(mission);

          // compute the desired size of the processed rasters
          preprocessor->updateRequiredImageSize(descriptors, startIndex, endIndex, td);
          preprocessor->Build(descriptors.begin() + startIndex, descriptors.begin() + endIndex, td);
      }

      const auto &sensorOutDays = getOutputDays(preprocessors, resample, sp);

      // Samples
      typedef double ValueType;
      typedef itk::VariableLengthVector<ValueType> MeasurementType;

      // Build a Measurement Vector of mean
      MeasurementType mean;

      // Build a MeasurementVector of variance
      MeasurementType variance;

      auto app = otb::Wrapper::ApplicationRegistry::CreateApplication("ComputeImagesStatistics");
      if (!app) {
          itkExceptionMacro("Unable to load the ComputeImagesStatistics application");
      }
      if (HasValue("bv")) {
          app->EnableParameter("bv");
          app->SetParameterFloat("bv", GetParameterFloat("bv"));
      }

      app->EnableParameter("out");
      app->SetParameterString("out", GetParameterString("out"));

      app->EnableParameter("il");
      auto imageList = dynamic_cast<InputImageListParameter *>(app->GetParameterByKey("il"));

      for (size_t i = 0; i < prodPerTile.size(); i++) {
          auto preprocessor = preprocessors->GetNthElement(i);
          preprocessor->SetSensorOutDays(sensorOutDays);
          auto output = preprocessor->GetOutput();
          imageList->AddImage(output);
      }
      app->UpdateParameters();
      app->ExecuteAndWriteOutput();
  }
  //  Software Guide :EndCodeSnippet

};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::CropTypeStatistics)
//  Software Guide :EndCodeSnippet
