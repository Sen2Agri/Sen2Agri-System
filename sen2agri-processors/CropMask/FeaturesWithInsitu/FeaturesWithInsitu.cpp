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
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbImageList.h"
#include "FeaturesWithInsitu.hxx"

typedef otb::ImageFileReader<ImageType> ReaderType;
typedef FeaturesWithInsituFunctor<ImageType::PixelType> FeaturesWithInsituFunctorType;
typedef FeaturesWithInsituBMFunctor<ImageType::PixelType> FeaturesWithInsituBMFunctorType;

typedef TernaryFunctorImageFilterWithNBands<FeaturesWithInsituFunctorType>
TernaryFunctorImageFilterWithNBandsType;
typedef TernaryFunctorImageFilterWithNBands<FeaturesWithInsituBMFunctorType>
TernaryFunctorImageFilterBMWithNBandsType;

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
class FeaturesWithInsitu : public Application
//  Software Guide : EndCodeSnippet
{
public:
    //  Software Guide : BeginLatex
    // The \code{ITK} public types for the class, the superclass and smart pointers.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    typedef FeaturesWithInsitu Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    // Software Guide : EndCodeSnippet

    //  Software Guide : BeginLatex
    //  Invoke the macros necessary to respect ITK object factory mechanisms.
    //  Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    itkNewMacro(Self);

    itkTypeMacro(FeaturesWithInsitu, otb::Application);
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
        SetName("FeaturesWithInsitu");
        SetDescription(
            "The feature extraction step produces the relevant features for the classication.");

        SetDocName("FeaturesWithInsitu");
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
        AddParameter(ParameterType_InputImage, "ndvi", "NDVI time series");
        AddParameter(ParameterType_InputImage, "ndwi", "NDWI time series");
        AddParameter(ParameterType_InputImage, "brightness", "Brightness time series");
        AddParameter(ParameterType_InputFilename,
                     "dates",
                     "The dates for the input series, expressed as days from epoch");

        AddParameter(ParameterType_Int, "window", "The number of dates in the temporal window");
        SetDefaultParameterInt("window", 2);

        AddParameter(ParameterType_Empty,
                     "bm",
                     "If set use the features from Benchmarking instead of the features from ATBD");
        MandatoryOff("bm");

        AddParameter(ParameterType_OutputImage, "out", "Temporal and Statistic features");

        //  Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        // An example commandline is automatically generated. Method
        // \code{SetDocExampleParameterValue()} is
        // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet
        SetDocExampleParameterValue("ndvi", "ndvi.tif");
        SetDocExampleParameterValue("ndwi", "ndwi.tif");
        SetDocExampleParameterValue("brightness", "brightness.tif");
        SetDocExampleParameterValue("dates", "dates.txt");
        SetDocExampleParameterValue("window", "2");
        SetDocExampleParameterValue("out", "features_insitu.tif");
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
        // define all needed types
        m_bands = 27;
        m_w = 2;
        m_delta = 0.05f;
        m_tsoil = 0.2f;
        m_bm = true;

        m_ndviReader = ReaderType::New();
        m_ndwiReader = ReaderType::New();
        m_brightnessReader = ReaderType::New();
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
        // Read the parameters
        m_w = GetParameterInt("window");
        if (HasValue("bm")) {
            m_bm = GetParameterEmpty("bm");
        }
        if (m_bm) {
            m_bands = 26;
        }
        otbAppLogINFO("Computing " << m_bands << " temporal features!");

        // Get the file that contains the dates
        m_inDates.clear();
        std::string datesFileName = GetParameterString("dates");
        std::ifstream datesFile;
        datesFile.open(datesFileName);
        if (!datesFile.is_open()) {
            itkExceptionMacro("Can't open dates file for reading!");
        }

        // read the file and save the dates as second from Epoch to a vector
        std::string value;
        std::string date;
        std::vector<std::string> sensors;
        while (std::getline(datesFile, value)) {
            std::istringstream ss(value);
            ss >> date;
            m_inDates.emplace_back(std::stoi(date));
        }

        // close the file
        datesFile.close();

        // Read the input files
        m_ndviReader->SetFileName(GetParameterString("ndvi"));
        m_ndviReader->UpdateOutputInformation();

        m_ndwiReader->SetFileName(GetParameterString("ndwi"));
        m_ndwiReader->UpdateOutputInformation();

        m_brightnessReader->SetFileName(GetParameterString("brightness"));
        m_brightnessReader->UpdateOutputInformation();

        // connect the functor based filter
        if (m_bm) {
            m_filterBM = TernaryFunctorImageFilterBMWithNBandsType::New();
            m_filterBM->SetNumberOfOutputBands(m_bands);
            m_filterBM->SetFunctor(
                FeaturesWithInsituBMFunctorType(m_bands,
                                                m_w,
                                                m_delta,
                                                m_inDates,
                                                m_tsoil));
        } else {
            m_filter = TernaryFunctorImageFilterWithNBandsType::New();
            m_filter->SetNumberOfOutputBands(m_bands);
            m_filter->SetFunctor(
                FeaturesWithInsituFunctorType(m_bands,
                                              m_w,
                                              m_delta,
                                              m_inDates,
                                              m_tsoil));
        }

        if (m_bm) {
            m_filterBM->SetInput(0, m_ndviReader->GetOutput());
            m_filterBM->SetInput(1, m_ndwiReader->GetOutput());
            m_filterBM->SetInput(2, m_brightnessReader->GetOutput());
        } else {
            m_filter->SetInput(0, m_ndviReader->GetOutput());
            m_filter->SetInput(1, m_ndwiReader->GetOutput());
            m_filter->SetInput(2, m_brightnessReader->GetOutput());
        }

        if (m_bm) {
            SetParameterOutputImage("out", m_filterBM->GetOutput());
        } else {
            SetParameterOutputImage("out", m_filter->GetOutput());
        }
    }
    //  Software Guide :EndCodeSnippet

    // The number of bands per output
    int m_bands;
    // The slice size
    int m_w;
    // Delta
    float m_delta;
    // the dates for the images
    std::vector<int> m_inDates;
    // T_soil
    float m_tsoil;
    // Use benchmarking features
    bool m_bm;

    ReaderType::Pointer m_ndviReader;
    ReaderType::Pointer m_ndwiReader;
    ReaderType::Pointer m_brightnessReader;
    TernaryFunctorImageFilterWithNBandsType::Pointer m_filter;
    TernaryFunctorImageFilterBMWithNBandsType::Pointer m_filterBM;
};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::FeaturesWithInsitu)
//  Software Guide :EndCodeSnippet
