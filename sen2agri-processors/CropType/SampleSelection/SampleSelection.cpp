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
class SampleSelection : public Application
//  Software Guide : EndCodeSnippet
{
public:
  //  Software Guide : BeginLatex
  // The \code{ITK} public types for the class, the superclass and smart pointers.
  // Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  typedef SampleSelection Self;
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

  itkTypeMacro(SampleSelection, otb::Application)
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
      SetName("SampleSelection");
      SetDescription("Split the reference data into 2 disjoint sets, the training set and the validation set.");

      SetDocName("SampleSelection");
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
    AddParameter(ParameterType_Float, "ratio", "Sample Ratio");
    AddParameter(ParameterType_Int, "seed", "Seed for the random number generation");
    AddParameter(ParameterType_Empty, "nofilter", "Do not filter the polygons based on Crop/No crop");
    MandatoryOff("nofilter");

    AddParameter(ParameterType_OutputFilename, "tp", "Training Polygons");
    AddParameter(ParameterType_OutputFilename, "vp", "Validation Polygons");
    AddParameter(ParameterType_OutputFilename, "lut", "The lookup table based on the CODE field. Used only for Crop Type");
    MandatoryOff("lut");

    // Set default value for parameters
    SetDefaultParameterFloat("ratio", 0.75);
    SetDefaultParameterInt("seed", std::time(0));
     //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
    // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("ref", "reference_polygons.shp");
    SetDocExampleParameterValue("ratio", "0.75");
    SetDocExampleParameterValue("seed", "0");
    SetDocExampleParameterValue("tp", "training_polygons.shp");
    SetDocExampleParameterValue("vp", "validation_polygons.shp");
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
      otb::ogr::DataSource::Pointer ogrTp;
      otb::ogr::DataSource::Pointer ogrVp;

      std::multimap<int, ogr::Feature> featuresMap;

      // Create the reader over the reference file
      ogrRef = otb::ogr::DataSource::New(GetParameterString("ref"), otb::ogr::DataSource::Modes::Read);
      if (ogrRef->GetLayersCount() < 1) {
          itkExceptionMacro("The source file must contain at least one layer");
      }

      // get the required sampling ratio
      const float ratio = GetParameterFloat("ratio");
      // get the random seed
      const int seed = GetParameterInt("seed");

      // Create the writers over the outut files
      ogrTp = otb::ogr::DataSource::New(GetParameterString("tp"), otb::ogr::DataSource::Modes::Overwrite);
      ogrVp = otb::ogr::DataSource::New(GetParameterString("vp"), otb::ogr::DataSource::Modes::Overwrite);

      // read the layer of the reference file
      otb::ogr::Layer sourceLayer = ogrRef->GetLayer(0);
      if (sourceLayer.GetGeomType() != wkbPolygon) {
          itkExceptionMacro("The first layer must contain polygons!");
      }
      auto filter = !GetParameterEmpty("nofilter");
      if (filter) {
          std::cout << "Excluding non-crop features\n";
          auto ret = sourceLayer.ogr().SetAttributeFilter("CROP=1");
          if (ret != OGRERR_NONE) {
              std::cerr << "SetAttributeFilter() failed: " << ret << '\n';
          }
      }

      int featureCount = sourceLayer.GetFeatureCount(true);



      // read all features from the source field and add them to the multimap
      for (ogr::Feature& feature : sourceLayer) {
          if (!filter || feature.ogr().GetFieldAsInteger("CROP")) {
            featuresMap.insert(std::pair<int, ogr::Feature>(feature.ogr().GetFieldAsInteger("CODE"), feature.Clone()));
          }
      }
      std::cerr << '\n';


      // create the layers for the target files
      otb::ogr::Layer tpLayer = ogrTp->CreateLayer(sourceLayer.GetName(), sourceLayer.GetSpatialRef()->Clone(), sourceLayer.GetGeomType());
      otb::ogr::Layer vpLayer = ogrVp->CreateLayer(sourceLayer.GetName(), sourceLayer.GetSpatialRef()->Clone(), sourceLayer.GetGeomType());

      // add the fields
      OGRFeatureDefn &layerDefn = sourceLayer.GetLayerDefn();

      for (int i = 0; i < layerDefn.GetFieldCount(); i++ ) {
          OGRFieldDefn &fieldDefn = *layerDefn.GetFieldDefn(i);
          tpLayer.CreateField(fieldDefn);
          vpLayer.CreateField(fieldDefn);
      }

      OGRFeatureDefn &tpLayerDefn = tpLayer.GetLayerDefn();
      OGRFeatureDefn &vpLayerDefn = vpLayer.GetLayerDefn();

      std::ofstream outLUTFile;

      if (HasValue("lut")) {
          // create the LUT file
          outLUTFile.open(GetParameterString("lut"));
          if (!outLUTFile.is_open()) {
              itkExceptionMacro("Can't open LUT file for writting!");
          }
          outLUTFile << "# LUT table for Crop Type " << std::endl;

          // append the lines for -10000 (nodata) and 0 (no crop)
          outLUTFile << "-10000 245 245 245" << std::endl;
          outLUTFile << "0 0 0 0" << std::endl;
      }

      int index = 0;
      int lastcode = -1;
      int featTrainingTarget = 0;
      int featValidationTarget = 0;
      int featTrainingCount = 0;
      int featValidationCount = 0;

      // initialise the random number generator
      std::srand(seed);
      // Loop through the entries
      std::multimap<int,ogr::Feature>::iterator it;
      for (it=featuresMap.begin(); it!=featuresMap.end(); ++it) {
          // get the feature
          ogr::Feature &f = it->second;

          if (it->first != lastcode) {
              if (HasValue("lut")) {
                  // Adda new color to the LUT file
                  outLUTFile << it->first << " " << colors[index] << std::endl;
                  index = (index + 1) % 20;
              }
              lastcode = it->first;

              // get the number of features with the same code
              int featCount = featuresMap.count(lastcode);

              // compute the target training features
              featTrainingTarget = std::max(1, (int)std::round((float)featCount * ratio));
              featValidationTarget = featCount - featTrainingTarget;
              featTrainingCount = 0;
              featValidationCount = 0;

              // Add info message to log
              std::ostringstream strLog;
              strLog << "Found " << featCount << " features with CODE = " << lastcode << ". ";
              strLog << "Using " << featTrainingTarget << " for training and " << featValidationTarget << " for validation. " << std::endl;
              GetLogger()->Info(strLog.str());
          }

          // generate a random number and convert it to the [0..1] interval
          float random = (float) std::rand() / (float)RAND_MAX;

          // select the target file for this feature
          if ((random <= ratio && featTrainingCount < featTrainingTarget) || featValidationCount == featValidationTarget) {
              ogr::Feature feat(tpLayerDefn);
              // Add field values from input Layer
              for (int i = 0; i < tpLayerDefn.GetFieldCount(); i++) {
                  OGRFieldDefn* fieldDefn = tpLayerDefn.GetFieldDefn(i);
                  feat.ogr().SetField(fieldDefn->GetNameRef(), f.ogr().GetRawFieldRef(i));
              }

              // Set the geometry
              feat.SetGeometry(f.GetGeometry()->clone());

              tpLayer.CreateFeature(feat);
              featTrainingCount++;
          } else {
              ogr::Feature feat(vpLayerDefn);
              // Add field values from input Layer
              for (int i = 0; i < vpLayerDefn.GetFieldCount(); i++) {
                  OGRFieldDefn* fieldDefn = vpLayerDefn.GetFieldDefn(i);

                  feat.ogr().SetField(fieldDefn->GetNameRef(), f.ogr().GetRawFieldRef(i));
              }

              // Set the geometry
              feat.SetGeometry(f.GetGeometry()->clone());

              vpLayer.CreateFeature(feat);
              featValidationCount++;
          }
      }

      ogrTp->SyncToDisk();
      ogrVp->SyncToDisk();

      // Write the LUT file
      if (HasValue("lut")) {
          outLUTFile.close();
      }

      std::ostringstream strLog;
      strLog << "Total features: " << featureCount;
      strLog << ", Training features: " << tpLayer.GetFeatureCount(true);
      strLog << ", Validation features: " << vpLayer.GetFeatureCount(true) << std::endl;
      GetLogger()->Info(strLog.str());
  }
  //  Software Guide :EndCodeSnippet

  std::string colors[20] = {
      std::string("255 0 0"), // Red
      std::string("0 255 0"), // Green
      std::string("0 0 255"), // Blue
      std::string("255 255 0"), // Yellow
      std::string("255 0 255"), // Fucsia
      std::string("0 255 255"), // Cyan
      std::string("255 165 0"), // Orange
      std::string("128 0 128"), // Purple
      std::string("51 161 201"), //Peacock
      std::string("189 252 201"), //Mint
      std::string("128 0 0"),//Maroon
      std::string("245 222 179"),//Wheat
      std::string("142 56 142"), //sgi beet
      std::string("113 113 198"), //sgi slateblue
      std::string("125 158 192"), //sgi lightblue
      std::string("56 142 142"), //sgi teal
      std::string("113 198 113"), //sgi chartreuse
      std::string("142 142 56"), //sgi olivedrab
      std::string("197 193 170"), //sgi brightgray
      std::string("198 113 113") //sgi salmon
  };

};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::SampleSelection)
//  Software Guide :EndCodeSnippet


