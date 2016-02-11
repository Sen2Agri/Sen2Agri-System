
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
// The sample selection consists in splitting the reference data into 2 disjoint sets, the training
// set and the validation set.
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
typedef otb::ImageFileReader<ImageType> ReaderType;

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
    itkNewMacro(Self);

    itkTypeMacro(TemporalResampling, otb::Application);
    //  Software Guide : EndCodeSnippet

private:
    //  Software Guide : BeginLatex
    //  \code{DoInit()} method contains class information and description, parameter set up, and
    // example values.
    //  Software Guide : EndLatex

    void DoInit()
    {

        // Software Guide : BeginLatex
        // Application name and description are set using following methods :
        // \begin{description}
        // \item[\code{SetName()}] Name of the application.
        // \item[\code{SetDescription()}] Set the short description of the class.
        // \item[\code{SetDocName()}] Set long name of the application (that can be displayed
        // \dots).
        // \item[\code{SetDocLongDescription()}] This methods is used to describe the class.
        // \item[\code{SetDocLimitations()}] Set known limitations (threading, invalid pixel type
        // \dots) or bugs.
        // \item[\code{SetDocAuthors()}] Set the application Authors. Author List. Format : "John
        // Doe, Winnie the Pooh" \dots
        // \item[\code{SetDocSeeAlso()}] If the application is related to one another, it can be
        // mentioned.
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
        // \code{Code/ApplicationEngine/otbWrapperTags.h} contains some predefined tags defined in
        // \code{Tags} namespace.
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet
        AddDocTag(Tags::Vector);
        //  Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        // The input parameters:
        // - ref: Vector file containing reference data
        // - ratio: Ratio between the number of training and validation polygons per class (dafault:
        // 0.75)
        // The output parameters:
        // - tp: Vector file containing reference data for training
        // - vp: Vector file containing reference data for validation
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet

        AddParameter(ParameterType_InputImage, "tocr", "S2 L2A surface reflectances");
        AddParameter(ParameterType_InputImage, "mask", "Validity masks for each acquisition date");
        AddParameter(ParameterType_InputFilename, "ind", "Dates of each image acquisition");
        AddParameter(ParameterType_StringList, "sp", "Temporal sampling rate");

        AddParameter(ParameterType_Choice, "mode", "Mode");
        SetParameterDescription("mode", "Specifies the choice of output dates (default: resample)");
        AddChoice("mode.resample", "Specifies the temporal resampling mode");
        AddChoice("mode.gapfill", "Specifies the gapfilling mode");
        SetParameterString("mode", "resample");

        AddParameter(ParameterType_OutputImage, "rtocr", "Resampled S2 L2A surface reflectances");

        AddParameter(ParameterType_OutputFilename,
                     "outdays",
                     "The file containing the days from epoch for the resampled time series");
        MandatoryOff("outdays");

        // Set default value for parameters
        //  Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        // An example commandline is automatically generated. Method
        // \code{SetDocExampleParameterValue()} is
        // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet
        SetDocExampleParameterValue("tocr", "tocr.tif");
        SetDocExampleParameterValue("mask", "mask.tif");
        SetDocExampleParameterValue("ind", "dates.txt");
        SetDocExampleParameterValue("sp", "SENTINEL 5 SPOT 5 LANDSAT 16");
        SetDocExampleParameterValue("mode", "resample");
        //  Software Guide : EndCodeSnippet
    }

    // Software Guide : BeginLatex
    // \code{DoUpdateParameters()} is called as soon as a parameter value change. Section
    // \ref{sec:appDoUpdateParameters}
    // gives a complete description of this method.
    // Software Guide : EndLatex
    //  Software Guide :BeginCodeSnippet
    void DoUpdateParameters()
    {
        // Nothing to do.
    }
    //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // The algorithm consists in a random sampling without replacement of the polygons of each class
    // with
    // probability p = sample_ratio value for the training set and
    // 1 - p for the validation set.
    // Software Guide : EndLatex
    //  Software Guide :BeginCodeSnippet
    void DoExecute()
    {
        // Read all input parameters
        imgReader = ReaderType::New();
        imgReader->SetFileName(GetParameterString("tocr"));

        maskReader = ReaderType::New();
        maskReader->SetFileName(GetParameterString("mask"));

        imgReader->GetOutput()->UpdateOutputInformation();
        maskReader->GetOutput()->UpdateOutputInformation();

        // get the interval used for the output images
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

        // Get the file that contains the dates
        std::string datesFileName = GetParameterString("ind");
        std::ifstream datesFile;
        datesFile.open(datesFileName);
        if (!datesFile.is_open()) {
            itkExceptionMacro("Can't open dates file for reading!");
        }

        // build the input data vector
        std::vector<SensorData> inData;
        std::string value;
        while (std::getline(datesFile, value) && value.size() > 0) {
            // the first line is the mission name
            SensorData sd;
            sd.sensorName = value;

            // read the number of input dates
            if (!std::getline(datesFile, value)) {
                itkExceptionMacro("Invald dates file!. Cannot read the number of images for sensor "
                                  << sd.sensorName);
            }
            int numInput = std::stoi(value);
            // read the input dates
            for (int i = 0; i < numInput; i++) {
                if (!std::getline(datesFile, value)) {
                    itkExceptionMacro("Invald dates file! Cannot read date "
                                      << (i + 1) << " for sensor " << sd.sensorName);
                }
                sd.inDates.push_back(getDaysFromEpoch(value));
            }

            if (resample) {
                auto it = sp.find(sd.sensorName);
                if (it == sp.end()) {
                    itkExceptionMacro("Sampling rate required for sensor " << sd.sensorName);
                }
                auto rate = it->second;
                for (int date = sd.inDates.front(); date <= sd.inDates.back(); date += rate) {
                    sd.outDates.emplace_back(date);
                }
            } else {
                sd.outDates.insert(sd.outDates.end(), sd.inDates.begin(), sd.inDates.end());
            }

            // add the data to the vector
            inData.emplace_back(sd);
        }

        // close the file
        datesFile.close();

        if (HasValue("outdays")) {
            // create the output days file
            std::ofstream outDaysFile(GetParameterString("outdays"));
            if (!outDaysFile) {
                itkExceptionMacro("Can't open output days file for writing!");
            }

            for (const auto &sd : inData) {
                for (auto date : sd.outDates) {
                    outDaysFile << sd.sensorName << ' ' << date << '\n';
                }
            }
        }

        // The number of image bands can be computed as the ratio between the bands in the image and
        // the bands in the mask
        int imageBands = imgReader->GetOutput()->GetNumberOfComponentsPerPixel() /
                         maskReader->GetOutput()->GetNumberOfComponentsPerPixel();

        // Create the instance of the filter which will perform all computations
        filter = BinaryFunctorImageFilterWithNBands<ImageType, GapFillingFunctor<ImageType::PixelType>>::New();

        int dateCount = 0;
        for (const auto &sd : inData) {
            dateCount += sd.outDates.size();
        }
        filter->SetNumberOfOutputBands(imageBands * dateCount);
        filter->SetFunctor(GapFillingFunctor<ImageType::PixelType>(inData, 0, imageBands));

        filter->SetInput(0, imgReader->GetOutput());
        filter->SetInput(1, maskReader->GetOutput());

        SetParameterOutputImage("rtocr", filter->GetOutput());
    }
    //  Software Guide :EndCodeSnippet
private:
    ReaderType::Pointer imgReader;
    ReaderType::Pointer maskReader;
    BinaryFunctorImageFilterWithNBands<ImageType, GapFillingFunctor<ImageType::PixelType>>::Pointer filter;

    inline int getDaysFromEpoch(const std::string &date)
    {
        struct tm tm = {};
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
