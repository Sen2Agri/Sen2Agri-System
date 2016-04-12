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
// The sample selection consists in splitting the reference data into 2 disjoint sets, the training
// set and the validation set.
// These sets are composed of polygons, not individual pixels.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
#include <algorithm>

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbConcatenateVectorImageFilter.h"
#include "itkBinaryFunctorImageFilter.h"

#include "DataSmoothing.hxx"

typedef otb::ImageFileReader<ImageType> ReaderType;
typedef DataSmoothingFunctor<ImageType::PixelType, MaskImageType::PixelType> DataSmoothingFunctorType;
typedef itk::BinaryFunctorImageFilter<ImageType, MaskImageType, ImageType, DataSmoothingFunctorType>
DataSmoothingFilterType;
typedef otb::ConcatenateVectorImageFilter<ImageType, ImageType, ImageType>
ConcatenateVectorImageFilterType;
//  Software Guide : EndCodeSnippet

void sortImages(std::vector<ImageInfo> &images)
{
    std::sort(
        images.begin(), images.end(), [](const ImageInfo & img1, const ImageInfo & img2) noexcept {
            return std::tie(img1.day, img1.priority) < std::tie(img2.day, img2.priority);
        });
}

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
class DataSmoothing : public Application
//  Software Guide : EndCodeSnippet
{
public:
    //  Software Guide : BeginLatex
    // The \code{ITK} public types for the class, the superclass and smart pointers.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    typedef DataSmoothing Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    // Software Guide : EndCodeSnippet

    //  Software Guide : BeginLatex
    //  Invoke the macros necessary to respect ITK object factory mechanisms.
    //  Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    itkNewMacro(Self)
    itkTypeMacro(DataSmoothing, otb::Application)
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
        SetName("DataSmoothing");
        SetDescription(
            "The feature extraction step produces the relevant features for the classication.");

        SetDocName("DataSmoothing");
        SetDocLongDescription("The feature extraction step produces the relevant features for the "
                              "classication. The features are computed"
                              "for each date of the resampled and gaplled time series and "
                              "concatenated together into a single multi-channel"
                              "image file. The selected features are the surface reflectances, the "
                              "NDVI, the NDWI and the brightness.");
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
        AddParameter(ParameterType_InputImage, "ts", "The input time series");
        AddParameter(ParameterType_InputImage, "mask", "The input mask series");
        AddParameter(ParameterType_InputFilename, "dates", "The image dates file");
        AddParameter(ParameterType_Float, "lambda", "Smoothing parameter of the Whitaker function");

        AddParameter(ParameterType_OutputImage, "sts", "The smoothed time series");
        AddParameter(
            ParameterType_OutputFilename, "outdays", "The output series dates, in days from epoch");
        MandatoryOff("outdays");

        SetDefaultParameterFloat("lambda", 2);

        //  Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        // An example commandline is automatically generated. Method
        // \code{SetDocExampleParameterValue()} is
        // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet
        SetDocExampleParameterValue("ts", "tocr.tif");
        SetDocExampleParameterValue("mask", "mask.tif");
        SetDocExampleParameterValue("dates", "dates.txt");
        SetDocExampleParameterValue("lambda", "2");
        SetDocExampleParameterValue("sts", "rtocrs.tif");
        SetDocExampleParameterValue("outdays", "days.txt");
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

        m_tsReader = ReaderType::New();

        m_smoothFilter = DataSmoothingFilterType::New();
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
        std::string dateFile = GetParameterString("dates");
        std::ifstream file(dateFile);
        if (!file) {
            itkExceptionMacro("Unable to open the date file");
        }

        std::vector<std::string> sensors = { "SENTINEL", "SPOT", "LANDSAT" };

        std::vector<ImageInfo> images;

        std::string sensor;
        std::string cnt;
        std::string dateStr;
        int index = 0;
        while (file >> sensor >> cnt) {
            auto it = std::find(sensors.begin(), sensors.end(), sensor);
            if (it == sensors.end()) {
                itkExceptionMacro("Unknown sensor " << sensor);
            }
            int priority = static_cast<int>(std::distance(sensors.begin(), it));
            int count = std::stoi(cnt);
            for (int i = 0; i < count; i++) {
                file >> dateStr;
                std::tm tm = {};
                if (!strptime(dateStr.c_str(), "%Y%m%d", &tm)) {
                    itkExceptionMacro("Invalid date: " << dateStr);
                }
                images.emplace_back(index++, mktime(&tm) / (24 * 3600), priority);
            }
        }

        // Get the input parameters
        double lambda = GetParameterFloat("lambda");

        // Read the input file
        m_tsReader->SetFileName(GetParameterString("ts"));
        m_tsReader->UpdateOutputInformation();

        sortImages(images);

        std::ofstream outFile;
        if (HasValue("outdays")) {
            outFile.open(GetParameterString("outdays"));
            if (!outFile) {
                itkExceptionMacro("Unable to open output days file");
            }
        }
        auto prevDay = -1;
        std::vector<int> outputDates;
        for (const auto &img : images) {
            if (img.day != prevDay) {
                outputDates.emplace_back(img.day);
                prevDay = img.day;
                if (outFile) {
                    outFile << img.day << '\n';
                }
            }
        }

        int bands = m_tsReader->GetOutput()->GetNumberOfComponentsPerPixel() / images.size();

        // connect the functor based filter
        m_smoothFilter->SetFunctor(DataSmoothingFunctorType(bands, lambda, outputDates, images));
        m_smoothFilter->SetInput1(m_tsReader->GetOutput());
        m_smoothFilter->SetInput2(GetParameterInt16VectorImage("mask"));
        m_smoothFilter->UpdateOutputInformation();
        m_smoothFilter->GetOutput()->SetNumberOfComponentsPerPixel(outputDates.size() * bands);
        SetParameterOutputImage("sts", m_smoothFilter->GetOutput());
    }
    //  Software Guide :EndCodeSnippet

    ReaderType::Pointer m_tsReader;
    DataSmoothingFilterType::Pointer m_smoothFilter;
};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::DataSmoothing)
//  Software Guide :EndCodeSnippet
