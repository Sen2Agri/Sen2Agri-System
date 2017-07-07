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

#include "otbVectorImage.h"
#include "otbVectorData.h"

#include "otbImage.h"

#include "otbVectorDataFileReader.h"
#include "otbImageFileReader.h"

//UsingOPENCV
# include "otbRandomForestsMachineLearningModel.h"

#include "ShpFileSampleGeneratorFast.hpp"
#include "ListSampleClassStatistics.hpp"

typedef short                                 PixelValueType;
typedef otb::VectorImage<PixelValueType, 2>   ImageType;
typedef otb::VectorData<double, 2>            VectorDataType;


typedef otb::ImageFileReader<ImageType>             ReaderType;
typedef otb::VectorDataFileReader<VectorDataType>   ShapeReaderType;

typedef int ClassLabelType;
typedef typename ImageType::PixelType SampleType;
typedef itk::Statistics::ListSample<SampleType> ListSampleType;
typedef itk::FixedArray<ClassLabelType, 1> LabelType; //note could be templated by an std:::string
typedef itk::Statistics::ListSample<LabelType> ListLabelType;

typedef itk::FixedArray<unsigned int, 2>  CoordinateType; //note could be templated by an std:::string
typedef itk::Statistics::ListSample<CoordinateType> CoordinateList;

typedef otb::RandomForestsMachineLearningModel<PixelValueType, ClassLabelType> RandomForestType;

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
class RandomForestTraining : public Application
//  Software Guide : EndCodeSnippet
{
public:
  //  Software Guide : BeginLatex
  // The \code{ITK} public types for the class, the superclass and smart pointers.
  // Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  typedef RandomForestTraining Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
  // Software Guide : EndCodeSnippet

  //  Software Guide : BeginLatex
  //  Invoke the macros necessary to respect ITK object factory mechanisms.
  //  Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  itkNewMacro(Self)
  itkTypeMacro(RandomForestTraining, otb::Application)
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
      SetName("RandomForestTraining");
      SetDescription("The feature extraction step produces the relevant features for the classication.");

      SetDocName("RandomForestTraining");
      SetDocLongDescription("The feature extraction step produces the relevant features for the classication. The features are computed"
                            "for each date of the resampled and gaplled time series and concatenated together into a single multi-channel"
                            "image file. The selected features are the surface reflectances, the NDVI, the NDWI and the brightness.");
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
    AddParameter(ParameterType_InputImage, "fvi", "The feature vector image");
    AddParameter(ParameterType_InputVectorData, "shp", "The training polygons.");

    AddParameter(ParameterType_Int, "seed", "The seed used for random selection");
    SetDefaultParameterInt("seed", std::time(0));
    MandatoryOff("seed");

    AddParameter(ParameterType_Int, "nbsamples", "The maximum number of training samples.");
    SetDefaultParameterInt("nbsamples", 1000);
    SetMinimumParameterIntValue("nbsamples",1000);
    SetMinimumParameterIntValue("nbsamples",5000);
    MandatoryOff("nbsamples");

    AddParameter(ParameterType_Int, "rfnbtrees", "The maximum number of random trees");
    SetDefaultParameterInt("rfnbtrees", 100);
    MandatoryOff("rfnbtrees");

    AddParameter(ParameterType_Int, "rfmin", "The minimum number of samples in each node");
    SetDefaultParameterInt("rfmin", 5);
    MandatoryOff("rfmin");

    AddParameter(ParameterType_Int, "rfmax", "The maximum number of samples in each node");
    SetDefaultParameterInt("rfmax", 25);
    MandatoryOff("rfmax");


    AddParameter(ParameterType_String, "out", "The output model.");

     //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
    // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("fvi", "fvi.tif");
    SetDocExampleParameterValue("shp", "shape.shp");
    SetDocExampleParameterValue("seed", "0");
    SetDocExampleParameterValue("nbsamples", "1000");
    SetDocExampleParameterValue("out", "model.txt");
    //  Software Guide : EndCodeSnippet
  }

  // Software Guide : BeginLatex
  // \code{DoUpdateParameters()} is called as soon as a parameter value change. Section \ref{sec:appDoUpdateParameters}
  // gives a complete description of this method.
  // Software Guide : EndLatex
  //  Software Guide :BeginCodeSnippet
  void DoUpdateParameters()
  {

      m_imgReader = ReaderType::New();
      m_shpReader = ShapeReaderType::New();
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
      // Get the paremeters
      unsigned int seed = (unsigned int)GetParameterInt("seed");
      unsigned int nbsamples = (unsigned int)GetParameterInt("nbsamples");
      int rfnbtrees = GetParameterInt("rfnbtrees");
      int rfmin = GetParameterInt("rfmin");
      int rfmax = GetParameterInt("rfmax");

      m_imgReader->SetFileName(GetParameterString("fvi"));
      m_imgReader->GetOutput()->UpdateOutputInformation();


      m_shpReader->SetFileName( GetParameterString("shp"));
      m_shpReader->Update( );

      ShpFileSampleGeneratorFast<ImageType, VectorDataType>::Pointer  SampleGenerator;
      SampleGenerator = ShpFileSampleGeneratorFast<ImageType, VectorDataType>::New();
      SampleGenerator->SetInputImage(m_imgReader->GetOutput());
      SampleGenerator->SetInputVectorData(m_shpReader->GetOutput());

      SampleGenerator->SetSeed(seed);

      SampleGenerator->SetLimitationofMaximumTrainingSamples(nbsamples);

      SampleGenerator->GenerateClassStatistics( );

      std::cout<<" Class stats are computed " << std::endl;

      SampleGenerator->GenerateSampleLists( );

      std::cout<<" Split is performed " << std::endl;

      auto NbClasses = SampleGenerator->GetNumberOfClasses();

      std::cout<<" The NbClasses is  " << NbClasses << std::endl;

      SampleGenerator->PrintInformation( );

      ListSampleType::Pointer trainingListSample   = SampleGenerator->GetTrainingListSample();
      ListLabelType::Pointer  trainingListLabelCrop    = SampleGenerator->GetTrainingListLabelCrop();
      ListSampleType::Pointer validationListSample = SampleGenerator->GetValidationListSample();
      ListLabelType::Pointer  validationListLabel  = SampleGenerator->GetValidationListLabel();
      ListLabelType::Pointer  validationListLabelCrop  = SampleGenerator->GetValidationListLabelCrop();

      ListSampleClassStatistics ListStatistics(trainingListLabelCrop,trainingListSample,m_imgReader->GetOutput()->GetNumberOfComponentsPerPixel());
      ListStatistics.writeResultsMean("./mean_file" );
      ListStatistics.writeResultsVariance("./variance_file");

      CoordinateList::Pointer TrainingCoordinates = SampleGenerator->GetTrainingCoordinatesList();
      CoordinateList::Pointer ValidationCoordinates = SampleGenerator->GetValidationCoordinatesList();

      std::cout<<" Nb Samples for training is  " <<  trainingListSample->Size() << std::endl;

      std::cout<<" Nb Samples for testing is  " <<  validationListSample->Size() << std::endl;



        RandomForestType::Pointer classifier = RandomForestType::New();
        classifier->SetInputListSample(trainingListSample);
        classifier->SetTargetListSample( trainingListLabelCrop );
        classifier->SetMaxDepth(rfmax);
        classifier->SetMinSampleCount(rfmin);
        classifier->SetRegressionAccuracy(0.);
        classifier->SetMaxNumberOfCategories(10);
        classifier->SetMaxNumberOfVariables(0);
        classifier->SetMaxNumberOfTrees(rfnbtrees);
        classifier->SetForestAccuracy(0.01);
        classifier->SetCalculateVariableImportance(true);
        std::cout << "STARTING LEARNING"  <<std::endl;

        classifier->Train();
        std::cout << "LEARNING DONE"  <<std::endl;
        classifier->Save(GetParameterString("out"));
  }
  //  Software Guide :EndCodeSnippet

  ReaderType::Pointer                     m_imgReader;
  ShapeReaderType::Pointer                m_shpReader;

};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::RandomForestTraining)
//  Software Guide :EndCodeSnippet



