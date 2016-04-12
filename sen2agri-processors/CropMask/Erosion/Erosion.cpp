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

#include "itkBinaryBallStructuringElement.h"

#include "itkBinaryErodeImageFilter.h"
#include "otbBandMathImageFilter.h"

#include <unordered_set>

typedef short                                PixelValueType;
typedef otb::Image<PixelValueType, 2>        InternalImageType;

typedef otb::ImageFileReader<InternalImageType>                                     ReaderType;
typedef itk::BinaryBallStructuringElement<PixelValueType, 2>                        BallStructuringType;
typedef BallStructuringType::RadiusType                                             RadiusType;

typedef InternalImageType::SizeType          ReferenceSizeType;
typedef InternalImageType::SizeValueType     ReferenceSizeValueType;

typedef itk::BinaryErodeImageFilter<InternalImageType, InternalImageType, BallStructuringType>   BinaryErodeImageFilterType;
typedef otb::ObjectList<BinaryErodeImageFilterType>                    BinaryErodeImageFilterListType;


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
class Erosion : public Application
//  Software Guide : EndCodeSnippet
{
public:
  //  Software Guide : BeginLatex
  // The \code{ITK} public types for the class, the superclass and smart pointers.
  // Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  typedef Erosion Self;
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

  itkTypeMacro(Erosion, otb::Application)
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
      SetName("Erosion");
      SetDescription("The feature extraction step produces the relevant features for the classication.");

      SetDocName("Erosion");
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
    AddParameter(ParameterType_InputImage, "in", "The input raster");
    AddParameter(ParameterType_Int, "radius", "The erosion radius");
    MandatoryOff("radius");

    AddParameter(ParameterType_OutputImage, "out", "The eroded raster");

    SetDefaultParameterInt("radius", 5);

     //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
    // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("in", "reference.tif");
    SetDocExampleParameterValue("radius", "1");
    SetDocExampleParameterValue("out", "eroded_reference.tif");
    //  Software Guide : EndCodeSnippet
  }

  // Software Guide : BeginLatex
  // \code{DoUpdateParameters()} is called as soon as a parameter value change. Section \ref{sec:appDoUpdateParameters}
  // gives a complete description of this method.
  // Software Guide : EndLatex
  //  Software Guide :BeginCodeSnippet
  void DoUpdateParameters()
  {

      m_inReader = ReaderType::New();
      m_erodeFilterList = BinaryErodeImageFilterListType::New();

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
      // Get the input parameters
      int radius = GetParameterInt("radius");

      //Read the input file
      m_inReader->SetFileName(GetParameterString("in"));
      m_inReader->Update();

      InternalImageType::Pointer refImg = m_inReader->GetOutput();
      ReferenceSizeType imgSize = refImg->GetLargestPossibleRegion().GetSize();

      // build ball structure
      RadiusType rad;
      rad[0] = radius;
      rad[1] = radius;
      BallStructuringType se;
      se.SetRadius(rad);
      se.CreateStructuringElement();

      // define the class set
      std::unordered_set<PixelValueType> classes;

      for (ReferenceSizeValueType i = 0; i < imgSize[0]; i++) {
          for (ReferenceSizeValueType j = 0; j < imgSize[1]; j++) {
              InternalImageType::IndexType index;
              index[0] = i;
              index[1] = j;

              InternalImageType::PixelType pix = refImg->GetPixel(index);
              classes.insert(pix);
          }
      }

      BinaryErodeImageFilterType::Pointer prevErodeFilter;

      // erode each class
      for (PixelValueType pix : classes) {
          BinaryErodeImageFilterType::Pointer currentErodeFilter = BinaryErodeImageFilterType::New();

          if (prevErodeFilter.IsNull()) {
              currentErodeFilter->SetInput(m_inReader->GetOutput());
          } else {
              currentErodeFilter->SetInput(prevErodeFilter->GetOutput());
          }

          // set the kernel and foreground and background values
          currentErodeFilter->SetKernel(se);
          currentErodeFilter->SetErodeValue(pix);
          currentErodeFilter->SetBackgroundValue(0);

          // add to list
          m_erodeFilterList->PushBack(currentErodeFilter);

          // update the previous erode filter
          prevErodeFilter = currentErodeFilter;
      }
      // set the output
      if (prevErodeFilter.IsNotNull()) {
          SetParameterOutputImage("out", prevErodeFilter->GetOutput());
      } else {
          SetParameterOutputImage("out", m_inReader->GetOutput());
      }

  }
  //  Software Guide :EndCodeSnippet

  ReaderType::Pointer                               m_inReader;
  BinaryErodeImageFilterListType::Pointer           m_erodeFilterList;
};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::Erosion)
//  Software Guide :EndCodeSnippet


