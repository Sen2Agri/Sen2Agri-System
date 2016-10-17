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

#include "otbStatisticsXMLFileReader.h"
#include "otbShiftScaleVectorImageFilter.h"
#include "../MultiModelImageClassifier/otbMultiModelImageClassificationFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbImageToVectorImageCastFilter.h"
#include "otbMachineLearningModelFactory.h"
#include "otbObjectList.h"

#include "otbVectorImage.h"
#include "otbImageList.h"

#include "CropMaskPreprocessing.h"

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

  typedef UInt16ImageType                                                                                OutputImageType;
  typedef UInt8ImageType                                                                                 MaskImageType;
  typedef itk::VariableLengthVector<FloatVectorImageType::InternalPixelType>                             MeasurementType;
  typedef otb::StatisticsXMLFileReader<MeasurementType>                                                  StatisticsReader;
  typedef otb::ShiftScaleVectorImageFilter<FloatVectorImageType, FloatVectorImageType>                   RescalerType;
  typedef otb::MultiModelImageClassificationFilter<FloatVectorImageType, OutputImageType, MaskImageType> ClassificationFilterType;
  typedef ClassificationFilterType::Pointer                                                              ClassificationFilterPointerType;
  typedef ClassificationFilterType::ModelType                                                            ModelType;
  typedef ModelType::Pointer                                                                             ModelPointerType;
  typedef otb::ObjectList<ModelType>                                                                     ModelListType;
  typedef otb::ObjectList<ModelType>::Pointer                                                            ModelListPointerType;
  typedef ClassificationFilterType::ValueType                                                            ValueType;
  typedef ClassificationFilterType::LabelType                                                            LabelType;
  typedef otb::MachineLearningModelFactory<ValueType, LabelType>                                         MachineLearningModelFactoryType;

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

    AddParameter(ParameterType_OutputImage, "out", "Output Image");
    SetParameterDescription( "out", "Output image" );
    SetParameterOutputImagePixelType("out", ImagePixelType_uint16);

    AddParameter(ParameterType_Float, "bv", "Background Value");
    SetParameterDescription( "bv", "Background value to ignore in statistics computation." );
    MandatoryOff("bv");

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

    AddParameter(ParameterType_InputFilenameList, "indays", "Resampled input days for each stratum");
    SetParameterDescription("indays", "The output days after temporal resampling, one file per stratum.");

    AddParameter(ParameterType_InputImage, "mask", "Input mask");
    SetParameterDescription("mask", "The mask allows to restrict classification of the input image to the area where mask pixel values are greater than 0");
    MandatoryOff("mask");

    AddParameter(ParameterType_InputFilenameList, "model", "Model files");
    SetParameterDescription("model", "One or more model files (produced by TrainImagesClassifier application, maximal class label = 65535).");

    AddParameter(ParameterType_Int, "nodatalabel", "No data label");
    SetDefaultParameterInt("nodatalabel", 0);
    SetParameterDescription("nodatalabel", "The label to output for masked pixels.");

    AddParameter(ParameterType_Empty, "singletile", "Single tile mode");
    SetParameterDescription("singletile", "Reuses image statistics from the training step");

     //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
    // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("il", "image1.xml image2.xml");
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

      preprocessor = CropMaskPreprocessing::New();
      preprocessor->SetPixelSize(pixSize);
      preprocessor->SetMission(mission);

      preprocessor->SetBM(bm);
      preprocessor->SetW(window);
      preprocessor->SetDelta(0.05f);
      preprocessor->SetTSoil(0.2f);

      // compute the desired size of the processed rasters
      preprocessor->updateRequiredImageSize(descriptors, 0, descriptors.size(), td);
      preprocessor->Build(descriptors.begin(), descriptors.end(), td);

      const auto &inDays = GetParameterStringList("indays");

      // Samples
      typedef double ValueType;
      typedef itk::VariableLengthVector<ValueType> MeasurementType;

      // Build a Measurement Vector of mean
      MeasurementType mean;

      // Build a MeasurementVector of variance
      MeasurementType variance;
      Application::Pointer app;

//      if (!GetParameterEmpty("singletile") && HasValue("outstat")) {
//          app = ApplicationRegistry::CreateApplication("ComputeImagesStatistics");
//          if (!app) {
//              itkExceptionMacro("Unable to load the ComputeImagesStatistics application");
//          }
//          if (HasValue("bv")) {
//              app->EnableParameter("bv");
//              app->SetParameterFloat("bv", GetParameterFloat("bv"));
//          }

//          app->EnableParameter("out");
//          app->SetParameterString("out", GetParameterString("outstat"));

//          app->EnableParameter("il");
//          auto imageList = dynamic_cast<InputImageListParameter *>(app->GetParameterByKey("il"));

//          imageList->AddImage(output);

//          app->UpdateParameters();

//          otbAppLogINFO("Computing statistics");
//          app->ExecuteAndWriteOutput();
//          otbAppLogINFO("Statistics written");
//      } else {
//          otbAppLogINFO("Skipping statistics");
//      }

      // Load models
      otbAppLogINFO("Loading models");

      m_Models = ModelListType::New();
      const std::vector<std::string> &modelFiles = GetParameterStringList("model");
      m_Models->Reserve(modelFiles.size());

      for (std::vector<std::string>::const_iterator it = modelFiles.begin(), itEnd = modelFiles.end(); it != itEnd; ++it)
        {
        ModelPointerType model = MachineLearningModelFactoryType::CreateMachineLearningModel(*it,
                                                                              MachineLearningModelFactoryType::ReadMode);

        if (model.IsNull())
          {
          otbAppLogFATAL(<< "Error when loading model " << *it << " : unsupported model type");
          }

        model->Load(*it);

        m_Models->PushBack(model);
      }

      otbAppLogINFO("Models loaded");

      // Normalize input image (optional)
      StatisticsReader::Pointer  statisticsReader = StatisticsReader::New();
      MeasurementType  meanMeasurementVector;
      MeasurementType  stddevMeasurementVector;
      m_Rescaler = RescalerType::New();

      // Classify
      m_ClassificationFilter = ClassificationFilterType::New();
      m_ClassificationFilter->SetModels(m_Models);

      if(IsParameterEnabled("mask"))
        {
        otbAppLogINFO("Using model mask");
        // Load mask image and cast into LabeledImageType
        MaskImageType::Pointer inMask = GetParameterUInt8Image("mask");

        m_ClassificationFilter->SetUseModelMask(true);
        m_ClassificationFilter->SetModelMask(inMask);
        }

      // Normalize input image if asked
//      if(IsParameterEnabled("imstat")  )
//        {
//        otbAppLogINFO("Input image normalization activated.");
//        // Load input image statistics
//        statisticsReader->SetFileName(GetParameterString("imstat"));
//        meanMeasurementVector   = statisticsReader->GetStatisticVectorByName("mean");
//        stddevMeasurementVector = statisticsReader->GetStatisticVectorByName("stddev");
//        otbAppLogINFO( "mean used: " << meanMeasurementVector );
//        otbAppLogINFO( "standard deviation used: " << stddevMeasurementVector );
//        // Rescale vector image
//        m_Rescaler->SetScale(stddevMeasurementVector);
//        m_Rescaler->SetShift(meanMeasurementVector);
////        m_Rescaler->SetInput(output);

//        m_ClassificationFilter->SetInput(m_Rescaler->GetOutput());
//        }
//      else
        {
        otbAppLogINFO("Input image normalization deactivated.");
        if (m_Models->Size() == 1)
          {
          const auto &sensorOutDays = readOutputDays(inDays[0]);
          auto output = preprocessor->GetOutput(sensorOutDays);

          for (size_t i = 0; i < m_Models->Size(); i++)
            {
            m_ClassificationFilter->PushBackInput(output);
            }
          }
        else
          {
          for (size_t i = 0; i < m_Models->Size(); i++)
            {
            const auto &sensorOutDays = readOutputDays(inDays[i]);
            auto output = preprocessor->GetOutput(sensorOutDays);

            m_ClassificationFilter->PushBackInput(output);
            }
          }
        }


      if(HasValue("nodatalabel"))
        {
          m_ClassificationFilter->SetDefaultLabel(GetParameterInt("nodatalabel"));
        }

      SetParameterOutputImage<OutputImageType>("out", m_ClassificationFilter->GetOutput());

//      app = otb::Wrapper::ApplicationRegistry::CreateApplication("ImageClassifier");
//      if (!app) {
//          itkExceptionMacro("Unable to load the ImageClassifier application");
//      }

//      app->EnableParameter("in");
//      auto inputParameter = dynamic_cast<InputImageParameter *>(app->GetParameterByKey("in"));
//      inputParameter->SetImage(output);

//      if (HasValue("mask")) {
//          app->EnableParameter("mask");
//          app->SetParameterString("mask", GetParameterString("mask"));
//      }

//      app->EnableParameter("model");
//      app->SetParameterString("model", GetParameterString("model"));

//      if (HasValue("outstat")) {
//        app->EnableParameter("imstat");
//        app->SetParameterString("imstat", GetParameterString("outstat"));
//      }

//      app->EnableParameter("out");
//      app->SetParameterString("out", GetParameterString("out"));
//      app->SetParameterOutputImagePixelType("out", ImagePixelType_uint16);

//      app->UpdateParameters();

//      otbAppLogINFO("Performing classification");
//      app->ExecuteAndWriteOutput();
//      otbAppLogINFO("Classification done");
  }

  //  Software Guide :EndCodeSnippet

  ClassificationFilterType::Pointer m_ClassificationFilter;
  ModelListPointerType m_Models;
  RescalerType::Pointer m_Rescaler;
  CropMaskPreprocessing::Pointer                    preprocessor;
};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::CropMaskImageClassifier)
//  Software Guide :EndCodeSnippet
