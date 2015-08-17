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

    AddParameter(ParameterType_OutputFilename, "tp", "Training Polygons");
    AddParameter(ParameterType_OutputFilename, "vp", "Validation Polygons");


    // Set default value for parameters
    SetDefaultParameterFloat("ratio", 0.75);
     //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
    // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("ref", "reference_polygons.shp");
    SetDocExampleParameterValue("ratio", "0.75");
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

      // Create the writers over the outut files
      ogrTp = otb::ogr::DataSource::New(GetParameterString("tp"), otb::ogr::DataSource::Modes::Overwrite);
      ogrVp = otb::ogr::DataSource::New(GetParameterString("vp"), otb::ogr::DataSource::Modes::Overwrite);

      // read the layer of the reference file
      otb::ogr::Layer sourceLayer = ogrRef->GetLayer(0);
      if (sourceLayer.GetGeomType() != wkbPolygon) {
          itkExceptionMacro("The first layer must contain polygons!");
      }

      sourceLayer.ogr().SetAttributeFilter("CROP=1");

      int featureCount = sourceLayer.GetFeatureCount(true);

      // read all features frm the source fiel and add them to the multimap
      for (ogr::Feature& feature : sourceLayer) {
          featuresMap.insert(std::pair<int, ogr::Feature>(feature.ogr().GetFieldAsInteger("CODE"), feature.Clone()));
      }

      // create the layers for the target files
      otb::ogr::Layer tpLayer = ogrTp->CreateLayer(sourceLayer.GetName(), sourceLayer.GetSpatialRef()->Clone(), sourceLayer.GetGeomType());
      otb::ogr::Layer vpLayer = ogrVp->CreateLayer(sourceLayer.GetName(), sourceLayer.GetSpatialRef()->Clone(), sourceLayer.GetGeomType());

      // add the fields
      OGRFeatureDefn* layerDefn = sourceLayer.ogr().GetLayerDefn();

      for (int i = 0; i < layerDefn->GetFieldCount(); i++ ) {
          OGRFieldDefn* fieldDefn = layerDefn->GetFieldDefn(i);
          tpLayer.ogr().CreateField(fieldDefn);
          vpLayer.ogr().CreateField(fieldDefn);
      }

      OGRFeatureDefn* tpLayerDefn = tpLayer.ogr().GetLayerDefn();
      OGRFeatureDefn* vpLayerDefn = sourceLayer.ogr().GetLayerDefn();

      int lastKey = -1;
      // Loop through the entries
      std::multimap<int,ogr::Feature>::iterator it;
      for (it=featuresMap.begin(); it!=featuresMap.end(); ++it) {
          // check if the key is changed
          if (lastKey != it->first) {
              lastKey = it->first;
              // initialise the random number generator
              std::srand(std::time(0) + lastKey);
          }

          // get the feature
          ogr::Feature &f = it->second;

          // generate a random number and convert it to the [0..1] interval
          float random = (float) std::rand() / (float)RAND_MAX;

          // select the target file for this feature
          if (random <= ratio) {
              ogr::Feature feat(*tpLayerDefn);
              // Add field values from input Layer
              for (int i = 0; i < tpLayerDefn->GetFieldCount(); i++) {
                  OGRFieldDefn* fieldDefn = tpLayerDefn->GetFieldDefn(i);
                  feat.ogr().SetField(fieldDefn->GetNameRef(), f.ogr().GetRawFieldRef(i));
              }

              // Set the geometry
              feat.ogr().SetGeometry(f.ogr().GetGeometryRef()->clone());

              tpLayer.CreateFeature(feat);
          } else {
              ogr::Feature feat(*vpLayerDefn);
              // Add field values from input Layer
              for (int i = 0; i < vpLayerDefn->GetFieldCount(); i++) {
                  OGRFieldDefn* fieldDefn = vpLayerDefn->GetFieldDefn(i);
                  feat.ogr().SetField(fieldDefn->GetNameRef(), f.ogr().GetRawFieldRef(i));
              }

              // Set the geometry
              feat.ogr().SetGeometry(f.ogr().GetGeometryRef()->clone());

              vpLayer.CreateFeature(feat);
          }
      }

      // save the output files
      tpLayer.ogr().CommitTransaction();
      vpLayer.ogr().CommitTransaction();

      ogrTp->SyncToDisk();
      ogrVp->SyncToDisk();

      std::cout << "total features: " << featureCount << ", Training features: " << tpLayer.GetFeatureCount(true) << ", Validation features: " << vpLayer.GetFeatureCount(true) << std::endl;
  }
  //  Software Guide :EndCodeSnippet


};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::SampleSelection)
//  Software Guide :EndCodeSnippet


