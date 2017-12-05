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
#include "otbConcatenateVectorImageFilter.h"
#include "otbMultiChannelExtractROI.h"
#include "otbBandMathImageFilter.h"
#include "otbLabelImageToVectorDataFilter.h"

#include "MahalanobisPixelExtractor.h"
#include "MahalanobisTrimmingFilter.h"


#include "otbVectorImage.h"
#include "otbImage.h"

typedef float                                PixelValueType;
typedef otb::VectorImage<PixelValueType, 2>  ImageType;
typedef otb::Image<PixelValueType, 2>        InternalImageType;

typedef otb::ImageFileReader<ImageType>         VectorImageReaderType;
typedef otb::ImageFileReader<InternalImageType> ImageReaderType;

typedef InternalImageType::SizeType          ReferenceSizeType;
typedef InternalImageType::SizeValueType     ReferenceSizeValueType;

typedef MahalanobisPixelExtractorFilter<ImageType> MahalanobisPixelExtractorFilterType;
typedef MahalanobisTrimmingFilter<InternalImageType, InternalImageType> MahalanobisTrimmingFilterType;

//typedef std::map<InternalImageType::PixelType, MahalanobisTrimmingFilterType::Pointer> MahalanobisTrimmingFilterMap;

/** Filters typedef */
typedef otb::MultiChannelExtractROI<ImageType::InternalPixelType,
                                    ImageType::InternalPixelType> ExtractROIFilterType;

typedef otb::BandMathImageFilter<InternalImageType>                       BandMathImageFilterType;
typedef otb::LabelImageToVectorDataFilter<InternalImageType>   LabelImageToVectorDataFilterType;

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
class Trimming : public Application
//  Software Guide : EndCodeSnippet
{
public:
  //  Software Guide : BeginLatex
  // The \code{ITK} public types for the class, the superclass and smart pointers.
  // Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  typedef Trimming Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
  // Software Guide : EndCodeSnippet

  //  Software Guide : BeginLatex
  //  Invoke the macros necessary to respect ITK object factory mechanisms.
  //  Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  itkNewMacro(Self)

  itkTypeMacro(Trimming, otb::Application)
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
      SetName("Trimming");
      SetDescription("The feature extraction step produces the relevant features for the classication.");

      SetDocName("Trimming");
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
    AddParameter(ParameterType_InputImage, "feat", "The features raster");
    AddParameter(ParameterType_InputImage, "ref", "The reference raster");

    AddParameter(ParameterType_Float, "alpha", "The alpha parameter");
    MandatoryOff("alpha");
    AddParameter(ParameterType_Int, "nbsamples", "The number of crop/nocrop samples required or 0 if all samples selected");
    MandatoryOff("nbsamples");
    AddParameter(ParameterType_Int, "seed", "The seed for the random number generation");
    MandatoryOff("seed");

    // This application is called in the context of the unsupervised crop mask processor, where the input has more features
    // than should be used for trimming. It would be better to make the channel list configurable, but for now allow the
    // user to disable that hard-coded selection for inputs which don't have 5 bands.
    AddParameter(ParameterType_Empty, "allbands", "Use all bands from the input file");
    MandatoryOff("allbands");

    AddParameter(ParameterType_OutputImage, "out", "The training samples raster file");


    SetDefaultParameterFloat("alpha", 0.01);
    SetDefaultParameterInt("nbsamples", 0);
    SetDefaultParameterInt("seed", 0);
     //  Software Guide : EndCodeSnippet

    // Software Guide : BeginLatex
    // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
    // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("feat", "features.tif");
    SetDocExampleParameterValue("ref", "reference.tif");
    SetDocExampleParameterValue("alpha", "0.01");
    SetDocExampleParameterValue("nbsamples", "0");
    SetDocExampleParameterValue("seed", "0");
    SetDocExampleParameterValue("out", "noinsitu_training_data.tif");
    //  Software Guide : EndCodeSnippet
  }

  // Software Guide : BeginLatex
  // \code{DoUpdateParameters()} is called as soon as a parameter value change. Section \ref{sec:appDoUpdateParameters}
  // gives a complete description of this method.
  // Software Guide : EndLatex
  //  Software Guide :BeginCodeSnippet
  void DoUpdateParameters()
  {
      // define all needed types

      m_FeaturesReader = VectorImageReaderType::New();
      m_ReferenceReader = ImageReaderType::New();
      m_BandsExtractor = ExtractROIFilterType::New();

      m_PixelExtractor = MahalanobisPixelExtractorFilterType::New();
      m_Trimming = MahalanobisTrimmingFilterType::New();
//      m_ShapeBuilder = LabelImageToVectorDataFilterType::New();
//      m_ShapeMask = BandMathImageFilterType::New();


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
      GetLogger()->Debug("Starting trimming\n");
      // get the input parameters
      const double alpha = GetParameterFloat("alpha");
      const int nbSamples = GetParameterInt("nbsamples");
      const int seed = GetParameterInt("seed");
      auto allbands = GetParameterEmpty("allbands");

      //Read the Features input file
      m_FeaturesReader->SetFileName(GetParameterString("feat"));
      m_FeaturesReader->UpdateOutputInformation();

      bool needExtract = false;

      int comp = m_FeaturesReader->GetOutput()->GetNumberOfComponentsPerPixel();
      if (comp != 5 && !allbands) {
          // Extract only the Green, Red and NIR bands at max NDVI and the Red and NIR bands at min NDVI
          m_BandsExtractor->SetInput(m_FeaturesReader->GetOutput());

          int inBands = comp == 20 ? 4 : 8;
          m_BandsExtractor->SetChannel(2 * inBands + 1);
          m_BandsExtractor->SetChannel(2 * inBands + 2);
          m_BandsExtractor->SetChannel(2 * inBands + 3);
          m_BandsExtractor->SetChannel(3 * inBands + 2);
          m_BandsExtractor->SetChannel(3 * inBands + 3);

          needExtract = true;
      }

      //Read the Reference input file
      m_ReferenceReader->SetFileName(GetParameterString("ref"));
      m_ReferenceReader->Update();

      // Get the output
      InternalImageType::Pointer refImg = m_ReferenceReader->GetOutput();

      ReferenceSizeType imgSize = refImg->GetLargestPossibleRegion().GetSize();
      //int bandCount = 0;

      // loop through the image
      GetLogger()->Debug("Splitting pixels into classes!\n");
      // signal that class 11 is found.
      //bool have11 = false;
      m_PixelExtractor->SetAlpha(alpha);
      m_PixelExtractor->SetNbSamples(nbSamples);
      m_PixelExtractor->SetSeed(seed);
      m_PixelExtractor->SetInput(needExtract ? m_BandsExtractor->GetOutput() : m_FeaturesReader->GetOutput());
      for (ReferenceSizeValueType i = 0; i < imgSize[0]; i++) {
          for (ReferenceSizeValueType j = 0; j < imgSize[1]; j++) {
              InternalImageType::IndexType index;
              index[0] = i;
              index[1] = j;

              InternalImageType::PixelType pix = refImg->GetPixel(index);
              if (pix > 0) {
                  m_PixelExtractor->AddPoint(index, pix);
              }
          }
      }

      // Update the pixels extractor
      m_PixelExtractor->Update();
      GetLogger()->Debug("Splitting done!\n");
      GetLogger()->Debug("Performing Trimming!\n");

      m_Trimming->SetInput(m_ReferenceReader->GetOutput());
      m_Trimming->SetPoints(m_PixelExtractor->GetIndeces());

      SetParameterOutputImage("out", m_Trimming->GetOutput());
      GetLogger()->Debug("Trimming done!\n");

//      m_ShapeMask->SetNthInput(0, m_Trimming->GetOutput());
//      m_ShapeMask->SetExpression("(b1>=0) ? 1 : 0");

//      m_ShapeBuilder->SetInput(m_Trimming->GetOutput());
//      m_ShapeBuilder->SetInputMask(m_ShapeMask->GetOutput());
//      m_ShapeBuilder->SetFieldName("CROP");
//      SetParameterOutputVectorData("out", m_ShapeBuilder->GetOutput());

  }
  //  Software Guide :EndCodeSnippet
  VectorImageReaderType::Pointer                m_FeaturesReader;
  ImageReaderType::Pointer                      m_ReferenceReader;
  MahalanobisPixelExtractorFilterType::Pointer  m_PixelExtractor;
  MahalanobisTrimmingFilterType::Pointer        m_Trimming;
  ExtractROIFilterType::Pointer                 m_BandsExtractor;
//  LabelImageToVectorDataFilterType::Pointer     m_ShapeBuilder;
//  BandMathImageFilterType::Pointer              m_ShapeMask;
};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::Trimming)
//  Software Guide :EndCodeSnippet


