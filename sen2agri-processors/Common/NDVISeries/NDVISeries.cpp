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

// Filters
#include "otbMultiChannelExtractROI.h"
#include "../Filters/otbTemporalResamplingFilter.h"

#include "../Filters/CropMaskNDVIPreprocessing.h"

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
class NDVISeries : public Application
        //  Software Guide : EndCodeSnippet
{
public:
    //  Software Guide : BeginLatex
    // The \code{ITK} public types for the class, the superclass and smart pointers.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    typedef NDVISeries Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    // Software Guide : EndCodeSnippet

    //  Software Guide : BeginLatex
    //  Invoke the macros necessary to respect ITK object factory mechanisms.
    //  Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    itkNewMacro(Self)

    itkTypeMacro(NDVISeries, otb::Application)
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
        SetName("NDVISeries");
        SetDescription("Extracts a temporally-resampled NDVI series");

        SetDocName("NDVISeries");
        SetDocLongDescription("Extracts a temporally-resampled NDVI series.");
        SetDocLimitations("None");
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

        AddParameter(ParameterType_StringList, "sp", "Temporal sampling rate");

        AddParameter(ParameterType_Choice, "mode", "Mode");
        SetParameterDescription("mode", "Specifies the choice of output dates (default: resample)");
        AddChoice("mode.resample", "Specifies the temporal resampling mode");
        AddChoice("mode.gapfill", "Specifies the gapfilling mode");
        AddChoice("mode.gapfillmain", "Specifies the gapfilling mode, but only use non-main series products to fill in the main one");
        SetParameterString("mode", "resample");

        AddParameter(ParameterType_Float, "pixsize", "The size of a pixel, in meters");
        SetDefaultParameterFloat("pixsize", 10.0); // The default value is 10 meters
        SetMinimumParameterFloatValue("pixsize", 1.0);
        MandatoryOff("pixsize");

        AddParameter(ParameterType_String, "mission", "The main raster series that will be used. By default SPOT is used");
        MandatoryOff("mission");

        //  Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
        // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet
        SetDocExampleParameterValue("il", "image1.xml image2.xml");
        SetDocExampleParameterValue("sp", "SENTINEL 10 LANDSAT 7");
        SetDocExampleParameterValue("out", "statistics.xml");
        SetDocExampleParameterValue("nbcomp", "6");
        //  Software Guide : EndCodeSnippet

        m_Preprocessor = CropMaskNDVIPreprocessing::New();
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
        std::vector<SensorPreferences> sp;
        if (HasValue("sp")) {
            const auto &spValues = GetParameterStringList("sp");
            sp = parseSensorPreferences(spValues);
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

        TemporalResamplingMode resamplingMode = TemporalResamplingMode::Resample;
        const auto &modeStr = GetParameterString("mode");
        if (modeStr == "gapfill") {
            resamplingMode = TemporalResamplingMode::GapFill;
        } else if (modeStr == "gapfillmain") {
            resamplingMode = TemporalResamplingMode::GapFillMainMission;
        }

        TileData td;

        m_Preprocessor = CropMaskNDVIPreprocessing::New();
        m_Preprocessor->SetPixelSize(pixSize);
        m_Preprocessor->SetMission(mission);

        // compute the desired size of the processed rasters
        m_Preprocessor->updateRequiredImageSize(descriptors, 0, descriptors.size(), td);
        m_Preprocessor->Build(descriptors.begin(), descriptors.end(), td);

        auto preprocessors = CropMaskNDVIPreprocessingList::New();
        preprocessors->PushBack(m_Preprocessor);
        const auto &sensorOutDays = getOutputDays(preprocessors, resamplingMode, mission, sp);

        auto output = m_Preprocessor->GetOutput(sensorOutDays);

        SetParameterOutputImage("out", output);
    }

    CropMaskNDVIPreprocessing::Pointer m_Preprocessor;
};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::NDVISeries)
//  Software Guide :EndCodeSnippet
