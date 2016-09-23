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
#include "../Filters/otbTemporalResamplingFilter.h"
#include "../Filters/otbTemporalMergingFilter.h"

#include "../Filters/otbSpotMaskFilter.h"
#include "../Filters/otbSentinelMaskFilter.h"

#include "../Filters/CropMaskPreprocessing.h"

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
class CropMaskImageClassifier : public Application
//  Software Guide : EndCodeSnippet
{
public:
  //  Software Guide : BeginLatex
  // The \code{ITK} public types for the class, the superclass and smart pointers.
  // Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  typedef CropMaskImageClassifier Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
  // Software Guide : EndCodeSnippet

  //  Software Guide : BeginLatex
  //  Invoke the macros necessary to respect ITK object factory mechanisms.
  //  Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  itkNewMacro(Self)

  itkTypeMacro(CropMaskImageClassifier, otb::Application)
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
      SetName("CropMaskImageClassifier");
      SetDescription("Build the statistics from a set of tiles");

      SetDocName("CropMaskImageClassifier");
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

    AddParameter(ParameterType_OutputFilename, "out", "Output Image");
    SetParameterDescription( "out", "Output image" );

    AddParameter(ParameterType_Float, "bv", "Background Value");
    SetParameterDescription( "bv", "Background value to ignore in statistics computation." );
    MandatoryOff("bv");

    AddParameter(ParameterType_StringList, "sp", "Temporal sampling rate");

    AddParameter(ParameterType_Float, "pixsize", "The size of a pixel, in meters");
    SetDefaultParameterFloat("pixsize", 10.0); // The default value is 10 meters
    SetMinimumParameterFloatValue("pixsize", 1.0);
    MandatoryOff("pixsize");

    AddParameter(ParameterType_String, "mission", "The main raster series that will be used. By default SPOT is used");
    MandatoryOff("mission");

    AddParameter(ParameterType_Int, "window", "The number of dates in the temporal window");
    SetDefaultParameterInt("window", 2);

    AddParameter(ParameterType_Empty,
                 "bm",
                 "If set use the features from Benchmarking instead of the features from ATBD");
    MandatoryOff("bm");

    AddParameter(ParameterType_OutputFilename, "outstat", "Statistics file");
    SetParameterDescription("outstat", "Statistics file");
    MandatoryOff("outstat");

    AddParameter(ParameterType_OutputFilename, "indays", "Resampled input days");
    SetParameterDescription("indays", "The output days after temporal resampling.");

    AddParameter(ParameterType_InputImage, "mask", "Input mask");
    SetParameterDescription("mask", "The mask allows to restrict classification of the input image to the area where mask pixel values are greater than 0");
    MandatoryOff("mask");

    AddParameter(ParameterType_InputFilename, "model", "Model file");
    SetParameterDescription("model", "A model file (maximum class label = 65535)");

    AddParameter(ParameterType_Empty, "singletile", "Single tile mode");
    SetParameterDescription("singletile", "Reuses image statistics from the training step");

     //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
    // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("il", "image1.xml image2.xml");
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
      }

      // Get the list of input files
      const std::vector<std::string> &descriptors = this->GetParameterStringList("il");
      if( descriptors.size()== 0 )
        {
        itkExceptionMacro("No input file set...");
        }


      // get the required pixel size
      auto pixSize = this->GetParameterFloat("pixsize");
      // get the main mission
      std::string mission = SPOT;
      if (HasValue("mission")) {
          mission = this->GetParameterString("mission");
      }


      auto bm = GetParameterEmpty("bm");
      auto window = GetParameterInt("window");

      TileData td;

      auto preprocessor = CropMaskPreprocessing::New();
      preprocessor->SetPixelSize(pixSize);
      preprocessor->SetMission(mission);

      preprocessor->SetBM(bm);
      preprocessor->SetW(window);
      preprocessor->SetDelta(0.05f);
      preprocessor->SetTSoil(0.2f);

      // compute the desired size of the processed rasters
      preprocessor->updateRequiredImageSize(descriptors, 0, descriptors.size(), td);
      preprocessor->Build(descriptors.begin(), descriptors.end(), td);

      const auto &sensorOutDays = readOutputDays(GetParameterString("indays"));
      preprocessor->SetSensorOutDays(sensorOutDays);
      auto output = preprocessor->GetOutput();

      // Samples
      typedef double ValueType;
      typedef itk::VariableLengthVector<ValueType> MeasurementType;

      // Build a Measurement Vector of mean
      MeasurementType mean;

      // Build a MeasurementVector of variance
      MeasurementType variance;
      Application::Pointer app;

      if (!GetParameterEmpty("singletile") && HasValue("outstat")) {
          app = ApplicationRegistry::CreateApplication("ComputeImagesStatistics");
          if (!app) {
              itkExceptionMacro("Unable to load the ComputeImagesStatistics application");
          }
          if (HasValue("bv")) {
              app->EnableParameter("bv");
              app->SetParameterFloat("bv", GetParameterFloat("bv"));
          }

          app->EnableParameter("out");
          app->SetParameterString("out", GetParameterString("outstat"));

          app->EnableParameter("il");
          auto imageList = dynamic_cast<InputImageListParameter *>(app->GetParameterByKey("il"));

          imageList->AddImage(output);

          app->UpdateParameters();

          otbAppLogINFO("Computing statistics");
          app->ExecuteAndWriteOutput();
          otbAppLogINFO("Statistics written");
      } else {
          otbAppLogINFO("Skipping statistics");
      }

      app = otb::Wrapper::ApplicationRegistry::CreateApplication("ImageClassifier");
      if (!app) {
          itkExceptionMacro("Unable to load the ImageClassifier application");
      }

      app->EnableParameter("in");
      auto inputParameter = dynamic_cast<InputImageParameter *>(app->GetParameterByKey("in"));
      inputParameter->SetImage(output);

      if (HasValue("mask")) {
          app->EnableParameter("mask");
          app->SetParameterString("mask", GetParameterString("mask"));
      }

      app->EnableParameter("model");
      app->SetParameterString("model", GetParameterString("model"));

      if (HasValue("outstat")) {
        app->EnableParameter("imstat");
        app->SetParameterString("imstat", GetParameterString("outstat"));
      }

      app->EnableParameter("out");
      app->SetParameterString("out", GetParameterString("out"));
      app->SetParameterOutputImagePixelType("out", ImagePixelType_uint16);

      app->UpdateParameters();

      otbAppLogINFO("Performing classification");
      app->ExecuteAndWriteOutput();
      otbAppLogINFO("Classification done");
  }

  //  Software Guide :EndCodeSnippet

};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::CropMaskImageClassifier)
//  Software Guide :EndCodeSnippet
