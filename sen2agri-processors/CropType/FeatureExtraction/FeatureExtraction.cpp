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

#include "FeatureExtraction.hxx"


typedef FeatureTimeSeriesFunctor<ImageType::PixelType>                    FeatureTimeSeriesFunctorType;
typedef NDVIFunctor<ImageType::PixelType>                                 NDVIFunctorType;
typedef NDWIFunctor<ImageType::PixelType>                                 NDWIFunctorType;
typedef BrightnessFunctor<ImageType::PixelType>                           BrightnessFunctorType;

typedef UnaryFunctorImageFilterWithNBands<FeatureTimeSeriesFunctorType>   FeatureTimeSeriesFilterType;
typedef UnaryFunctorImageFilterWithNBands<NDVIFunctorType>                NDVIFilterType;
typedef UnaryFunctorImageFilterWithNBands<NDWIFunctorType>                NDWIFilterType;
typedef UnaryFunctorImageFilterWithNBands<BrightnessFunctorType>          BrightnessFilterType;

typedef otb::ImageFileReader<ImageType>                                   ReaderType;


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
class FeatureExtraction : public Application
//  Software Guide : EndCodeSnippet
{
public:
  //  Software Guide : BeginLatex
  // The \code{ITK} public types for the class, the superclass and smart pointers.
  // Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  typedef FeatureExtraction Self;
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

  itkTypeMacro(FeatureExtraction, otb::Application)
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
      SetName("FeatureExtraction");
      SetDescription("The feature extraction step produces the relevant features for the classication.");

      SetDocName("FeatureExtraction");
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
    AddParameter(ParameterType_InputImage, "rtocr", "Resampled S2 L2A surface reflectances");

    AddParameter(ParameterType_OutputImage, "fts", "Feature time series");
    MandatoryOff("fts");
    AddParameter(ParameterType_OutputImage, "ndvi", "NDVI time series");
    MandatoryOff("ndvi");
    AddParameter(ParameterType_OutputImage, "ndwi", "NDWI time series");
    MandatoryOff("ndwi");
    AddParameter(ParameterType_OutputImage, "brightness", "Brightness time series");
    MandatoryOff("brightness");

     //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
    // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("rtocr", "reflectances.tif");
    SetDocExampleParameterValue("fts", "feature-time-series.tif");
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
      m_bandsPerImage = 4;
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
      // check that at least one of fts, ndvi, ndwi or brightness is set
      if (!HasValue("fts") && !HasValue("ndvi") && !HasValue("ndwi") && !HasValue("brightness")) {
          itkExceptionMacro("You must specify a value for at least one of 'fts', 'ndvi', 'ndwi' and 'brightness'!");
      }

      //Read the input time series
      m_imgReader->SetFileName(GetParameterString("rtocr"));
      m_imgReader->UpdateOutputInformation();

      int numImages = m_imgReader->GetOutput()->GetNumberOfComponentsPerPixel() / m_bandsPerImage;

      // verify what parameters are set
      if(HasValue("fts")) {
          m_ftsFilter = FeatureTimeSeriesFilterType::New();
          m_ftsFilter->SetNumberOfOutputBands(numImages * (m_bandsPerImage + 3));
          m_ftsFilter->SetFunctor(FeatureTimeSeriesFunctorType(m_bandsPerImage));
          m_ftsFilter->SetInput(m_imgReader->GetOutput());
          SetParameterOutputImage("fts", m_ftsFilter->GetOutput());
      }
      if(HasValue("ndvi")) {
          m_ndviFilter = NDVIFilterType::New();
          m_ndviFilter->SetNumberOfOutputBands(numImages);
          m_ndviFilter->SetFunctor(NDVIFunctorType(m_bandsPerImage));
          m_ndviFilter->SetInput(m_imgReader->GetOutput());
          SetParameterOutputImage("ndvi", m_ndviFilter->GetOutput());
      }
      if(HasValue("ndwi")) {
          m_ndwiFilter = NDWIFilterType::New();
          m_ndwiFilter->SetNumberOfOutputBands(numImages);
          m_ndwiFilter->SetFunctor(NDWIFunctorType(m_bandsPerImage));
          m_ndwiFilter->SetInput(m_imgReader->GetOutput());
          SetParameterOutputImage("ndwi", m_ndwiFilter->GetOutput());
      }
      if(HasValue("brightness")) {
          m_brightnessFilter = BrightnessFilterType::New();
          m_brightnessFilter->SetNumberOfOutputBands(numImages);
          m_brightnessFilter->SetFunctor(BrightnessFunctorType(m_bandsPerImage));
          m_brightnessFilter->SetInput(m_imgReader->GetOutput());
          SetParameterOutputImage("brightness", m_brightnessFilter->GetOutput());
      }
  }
  //  Software Guide :EndCodeSnippet

  // The number of bands per image
  int m_bandsPerImage;

  ReaderType::Pointer                       m_imgReader;
  FeatureTimeSeriesFilterType::Pointer      m_ftsFilter;
  NDVIFilterType::Pointer                   m_ndviFilter;
  NDWIFilterType::Pointer                   m_ndwiFilter;
  BrightnessFilterType::Pointer             m_brightnessFilter;
};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::FeatureExtraction)
//  Software Guide :EndCodeSnippet


