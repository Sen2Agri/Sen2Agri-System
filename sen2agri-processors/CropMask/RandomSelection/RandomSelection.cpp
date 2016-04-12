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
#include "otbOGRDataSourceToLabelImageFilter.h"
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
class RandomSelection : public Application
//  Software Guide : EndCodeSnippet
{
public:
  //  Software Guide : BeginLatex
  // The \code{ITK} public types for the class, the superclass and smart pointers.
  // Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  typedef RandomSelection Self;
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

  itkTypeMacro(RandomSelection, otb::Application)
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
      SetName("RandomSelection");
      SetDescription("Split the reference data into 2 disjoint sets, the training set and the validation set.");

      SetDocName("RandomSelection");
      SetDocLongDescription("The sample selection consists in splitting the reference data into 2 disjoint sets, "
                            "the training set and the validation set. "
                            "These sets are composed of polygons, not individual pixels.");
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
    AddParameter(ParameterType_InputFilename, "ref", "Reference Polygons");
    AddParameter(ParameterType_Float, "nbtrsample", "The number of training samples");
    AddParameter(ParameterType_Int, "seed", "Seed for the random number generation");

    AddParameter(ParameterType_OutputFilename, "trp", "Training Polygons");
    AddParameter(ParameterType_OutputFilename, "tsp", "Testing Polygons");


    // Set default value for parameters
    SetDefaultParameterFloat("nbtrsample", 1000);
    SetDefaultParameterInt("seed", std::time(0));
     //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
    // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("ref", "reference_polygons.shp");
    SetDocExampleParameterValue("nbtrsample", "1000");
    SetDocExampleParameterValue("seed", "0");
    SetDocExampleParameterValue("trp", "training_polygons.shp");
    SetDocExampleParameterValue("tsp", "testing_polygons.shp");
    //  Software Guide : EndCodeSnippet
  }

  // Software Guide : BeginLatex
  // \code{DoUpdateParameters()} is called as soon as a parameter value change. Section \ref{sec:appDoUpdateParameters}
  // gives a complete description of this method.
  // Software Guide : EndLatex
  //  Software Guide :BeginCodeSnippet
  void DoUpdateParameters()
  {
      // Nothing to do.
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
      // Internal variables for accessing the files
      otb::ogr::DataSource::Pointer ogrRef;
      otb::ogr::DataSource::Pointer ogrTrp;
      otb::ogr::DataSource::Pointer ogrTsp;

      std::multimap<int, ogr::Feature> featuresMap;

      // Create the reader over the reference file
      ogrRef = otb::ogr::DataSource::New(GetParameterString("ref"), otb::ogr::DataSource::Modes::Read);
      if (ogrRef->GetLayersCount() < 1) {
          itkExceptionMacro("The source file must contain at least one layer");
      }

      // get the required sampling ratio
      int nbtrsample = GetParameterInt("nbtrsample");
      // get the random seed
      const int seed = GetParameterInt("seed");

      // Create the writers over the outut files
      ogrTrp = otb::ogr::DataSource::New(GetParameterString("trp"), otb::ogr::DataSource::Modes::Overwrite);
      ogrTsp = otb::ogr::DataSource::New(GetParameterString("tsp"), otb::ogr::DataSource::Modes::Overwrite);

      // read the layer of the reference file
      otb::ogr::Layer sourceLayer = ogrRef->GetLayer(0);
      if (sourceLayer.GetGeomType() != wkbPolygon) {
          itkExceptionMacro("The first layer must contain polygons!");
      }

      int featureCount = sourceLayer.GetFeatureCount(true);
      int cropNumber = 0;
      int noCropNumber = 0;

      // read all features from the source file and add them to the multimap
      for (ogr::Feature& feature : sourceLayer) {
          featuresMap.insert(std::pair<int, ogr::Feature>(feature.ogr().GetFieldAsInteger("CODE"), feature.Clone()));
          if (feature.ogr().GetFieldAsInteger("CROP") == 1) {
              cropNumber++;
          } else {
              noCropNumber++;
          }
      }

      // update the number of features so that it will not be bigger than the crop or nocrop number
      nbtrsample = std::min(nbtrsample, std::min(cropNumber, noCropNumber));

      // compute the ratio for the features.
      float cropRatio = (float) nbtrsample / cropNumber;
      float noCropRatio = (float) nbtrsample / noCropNumber;

      // create the layers for the target files
      otb::ogr::Layer trpLayer = ogrTrp->CreateLayer(sourceLayer.GetName(), sourceLayer.GetSpatialRef()->Clone(), sourceLayer.GetGeomType());
      otb::ogr::Layer tspLayer = ogrTsp->CreateLayer(sourceLayer.GetName(), sourceLayer.GetSpatialRef()->Clone(), sourceLayer.GetGeomType());

      // add the fields
      OGRFeatureDefn* layerDefn = sourceLayer.ogr().GetLayerDefn();

      for (int i = 0; i < layerDefn->GetFieldCount(); i++ ) {
          OGRFieldDefn* fieldDefn = layerDefn->GetFieldDefn(i);
          trpLayer.ogr().CreateField(fieldDefn);
          tspLayer.ogr().CreateField(fieldDefn);
      }

      OGRFeatureDefn* trpLayerDefn = trpLayer.ogr().GetLayerDefn();
      OGRFeatureDefn* tspLayerDefn = tspLayer.ogr().GetLayerDefn();

      // initialise the random number generator
      std::srand(seed);
      // Loop through the key set
      std::multimap<int,ogr::Feature>::iterator it;
      for (it=featuresMap.begin(); it!=featuresMap.end(); it = featuresMap.upper_bound(it->first)) {
          // get the number of polygons from the feature
          int count = featuresMap.count(it->first);

          // compute the number of training polygons based on the polygon type
          float ratio;
          if (it->second.ogr().GetFieldAsInteger("CROP") == 1) {
              ratio = cropRatio;
          } else {

              ratio = noCropRatio;
          }
          int trCount = std::round(ratio * count);
          if (trCount == 0) {
              trCount = 1;
          }

          // loop through the features belonging to the current key
          std::multimap<int,ogr::Feature>::iterator featuresIt;
          for (featuresIt = featuresMap.lower_bound(it->first); featuresIt != featuresMap.upper_bound(it->first); ++featuresIt) {
              // get the feature
              ogr::Feature &f = featuresIt->second;

              // generate a random number and convert it to the [0..1] interval
              float random = (float) std::rand() / (float)RAND_MAX;

              if ((trCount != 0) && (trCount == count || random <= ratio)) {
                  // add feature to training set
                  ogr::Feature feat(*trpLayerDefn);
                  // Add field values from input Layer
                  for (int i = 0; i < trpLayerDefn->GetFieldCount(); i++) {
                      OGRFieldDefn* fieldDefn = trpLayerDefn->GetFieldDefn(i);
                      feat.ogr().SetField(fieldDefn->GetNameRef(), f.ogr().GetRawFieldRef(i));
                  }

                  // Set the geometry
                  feat.ogr().SetGeometry(f.ogr().GetGeometryRef()->clone());

                  trpLayer.CreateFeature(feat);

                  // decrement the number of training features
                  trCount--;
              } else {
                  // add feature to testing set
                  ogr::Feature feat(*tspLayerDefn);
                  // Add field values from input Layer
                  for (int i = 0; i < tspLayerDefn->GetFieldCount(); i++) {
                      OGRFieldDefn* fieldDefn = tspLayerDefn->GetFieldDefn(i);
                      feat.ogr().SetField(fieldDefn->GetNameRef(), f.ogr().GetRawFieldRef(i));
                  }

                  // Set the geometry
                  feat.ogr().SetGeometry(f.ogr().GetGeometryRef()->clone());

                  tspLayer.CreateFeature(feat);
              }

              // decrement the count
              count--;
          }
      }

      // save the output files
      trpLayer.ogr().CommitTransaction();
      tspLayer.ogr().CommitTransaction();

      ogrTrp->SyncToDisk();
      ogrTsp->SyncToDisk();

      std::cout << "total features: " << featureCount << ", Training features: " << trpLayer.GetFeatureCount(true) << ", Testing features: " << tspLayer.GetFeatureCount(true) << std::endl;
  }
  //  Software Guide :EndCodeSnippet


};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::RandomSelection)
//  Software Guide :EndCodeSnippet


