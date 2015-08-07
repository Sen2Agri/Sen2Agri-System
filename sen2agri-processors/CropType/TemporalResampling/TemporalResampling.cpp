
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
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "TemporalResampling.hxx"
//  Software Guide : EndCodeSnippet

// define all needed types
typedef otb::ImageFileReader<ImageType>  ReaderType;
typedef otb::ImageFileWriter<ImageType>  WriterType;


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
class TemporalResampling : public Application
//  Software Guide : EndCodeSnippet
{
public:
  //  Software Guide : BeginLatex
  // The \code{ITK} public types for the class, the superclass and smart pointers.
  // Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  typedef TemporalResampling Self;
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

  itkTypeMacro(TemporalResampling, otb::Application)
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
      SetName("TemporalResampling");
      SetDescription("Resample a list of images to a fixed step time interval.");

      SetDocName("TemporalResampling");
      SetDocLongDescription("Resample a list of images to a fixed step time interval.");
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

    AddParameter(ParameterType_InputImage, "tocr", "S2 L2A surface reflectances");
    AddParameter(ParameterType_InputImage, "mask", "Validity masks for each acquisition date");
    AddParameter(ParameterType_InputFilename, "ind", "Dates of each image acquisition");
    AddParameter(ParameterType_Int, "sp", "Temporal sampling rate");
    AddParameter(ParameterType_String, "t0", "Starting sampling date");
    AddParameter(ParameterType_String, "tend", "Last date");
    AddParameter(ParameterType_Int, "radius", "Radius of the temporal window ");

    AddParameter(ParameterType_OutputImage, "rtocr", "Resampled S2 L2A surface reflectances");


    // Set default value for parameters
    SetDefaultParameterInt("sp", 5);
    SetDefaultParameterInt("radius", 15);
     //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
    // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("tocr", "tocr.tif");
    SetDocExampleParameterValue("mask", "mask.tif");
    SetDocExampleParameterValue("ind", "dates.txt");
    SetDocExampleParameterValue("sp", "5");
    SetDocExampleParameterValue("t0", "20130501");
    SetDocExampleParameterValue("tend", "20130601");
    SetDocExampleParameterValue("radius", "15");
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

      //Read all input parameters
      imgReader = ReaderType::New();
      imgReader->SetFileName(GetParameterString("tocr"));

      maskReader = ReaderType::New();
      maskReader->SetFileName(GetParameterString("mask"));

      imgReader->GetOutput()->UpdateOutputInformation();
      maskReader->GetOutput()->UpdateOutputInformation();

      // Get the file that contains the dates
      std::string datesFileName = GetParameterString("ind");
      std::ifstream datesFile;
      datesFile.open(datesFileName);
      if (!datesFile.is_open()) {
          itkExceptionMacro("Can't open dates file for reading!");
      }

      // read the file and save the dates as second from Epoch to a vector
      std::vector<int> inDates;
      std::string value;
      while (std::getline(datesFile, value)) {
          inDates.push_back(getDaysFromEpoch(value));
      }
      // close the file
      datesFile.close();

      // get the interval used for the output images
      int sp = GetParameterInt("sp");
      int t0 = getDaysFromEpoch(GetParameterString("t0"));
      int tend = getDaysFromEpoch(GetParameterString("tend"));
      int radius = GetParameterInt("radius");

      // compute the output dates vector
      std::vector<int> outDates;
      for( int i = t0; i <= tend; i += sp) {
          outDates.push_back(i);
      }

      // The number of image bands can be computed as the ratio between the bands in the image and the bands in the mask
      int imageBands = imgReader->GetOutput()->GetNumberOfComponentsPerPixel() / maskReader->GetOutput()->GetNumberOfComponentsPerPixel();


      // Create the instance of the filter which will perform all computations
      BinaryFunctorImageFilterWithNBands::Pointer filter = BinaryFunctorImageFilterWithNBands::New();
      filter->SetNumberOfOutputBands(imageBands * outDates.size());
      filter->SetFunctor(GapFillingFunctor<ImageType::PixelType>(inDates, outDates, radius, imageBands));


      filter->SetInput(0, imgReader->GetOutput());
      filter->SetInput(1, maskReader->GetOutput());

      SetParameterOutputImage("rtocr", filter->GetOutput());
  }
  //  Software Guide :EndCodeSnippet
private:
  ReaderType::Pointer imgReader;
  ReaderType::Pointer maskReader;

  inline int getDaysFromEpoch(const std::string& date) {
      struct tm tm;
      if (strptime(date.c_str(), "%Y%m%d", &tm) == NULL) {
          itkExceptionMacro("Invalid value for a date: " + date);
      }
      return mktime(&tm) / 86400;
  }
};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::TemporalResampling)
//  Software Guide :EndCodeSnippet


