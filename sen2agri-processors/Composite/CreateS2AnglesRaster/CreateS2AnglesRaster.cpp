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
//    INPUTS: {input image}, {xml file desc}
//    OUTPUTS: {output image NDVI}
//  Software Guide : EndCommandLineArgs


//  Software Guide : BeginLatex
//
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
#include "otbWrapperTypes.h"
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbBandMathImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"

#include "MACCSMetadataReader.hpp"
#include "ViewingAngles.hpp"

#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkCastImageFilter.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbStreamingResampleImageFilter.h"

//Transform
#include "itkScalableAffineTransform.h"
#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

#include "itkResampleImageFilter.h"
#include "itkIndent.h"
#include <vector>
#include "libgen.h"

#define ANGLES_GRID_SIZE    23


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
//  CreateS2AnglesRaster class is derived from Application class.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
class CreateS2AnglesRaster : public Application
//  Software Guide : EndCodeSnippet
{
public:
    //  Software Guide : BeginLatex
    // The \code{ITK} public types for the class, the superclass and smart pointers.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    typedef CreateS2AnglesRaster Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    // Software Guide : EndCodeSnippet

    //  Software Guide : BeginLatex
    //  Invoke the macros necessary to respect ITK object factory mechanisms.
    //  Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    itkNewMacro(Self)

    itkTypeMacro(CreateS2AnglesRaster, otb::Application)
    //  Software Guide : EndCodeSnippet

    typedef FloatVectorImageType                            OutputImageType;
    typedef otb::ImageFileReader<Int16VectorImageType>      ImageReaderType;

    typedef otb::ImageList<OutputImageType>            ImageListType;
    typedef otb::ImageFileWriter<OutputImageType>      WriterType;

    typedef otb::MultiToMonoChannelExtractROI<FloatVectorImageType::InternalPixelType,
                                              FloatImageType::PixelType>    ExtractROIFilterType;
    typedef otb::ObjectList<ExtractROIFilterType>                           ExtractROIFilterListType;

    typedef otb::BandMathImageFilter<Int16ImageType>   BMFilterType;



private:

    //  Software Guide : BeginLatex
    //  \code{DoInit()} method contains class information and description, parameter set up, and example values.
    //  Software Guide : EndLatex

    typedef itk::MACCSMetadataReader                                   MACCSMetadataReaderType;
    typedef otb::StreamingResampleImageFilter<OutputImageType, OutputImageType, double>    ResampleFilterType;
    typedef itk::LinearInterpolateImageFunction<OutputImageType,  double>          LinearInterpolationType;
    typedef itk::IdentityTransform<double, OutputImageType::ImageDimension>      IdentityTransformType;
    typedef itk::ScalableAffineTransform<double, OutputImageType::ImageDimension> ScalableTransformType;
    typedef ScalableTransformType::OutputVectorType                         OutputVectorType;

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
        SetName("CreateS2AnglesRaster");
        SetDescription("Computes NDVI from RED and NIR bands");

        SetDocName("CreateS2AnglesRaster");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("AG");
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
        // - in: Input image filename with bands for RED and NIR
        // - xml: Input xml filename with description for input image
        // The output parameters:
        // - out: Vector file containing reference data for training
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet        
        AddParameter(ParameterType_InputFilename, "xml", "Xml Desc");

        AddParameter(ParameterType_Int, "outres", "Output image resolution");
        AddParameter(ParameterType_OutputImage, "out", "Out Image");


        // Set default value for parameters
        //SetDefaultParameterFloat("ratio", 0.75);
         //  Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
        // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet        
        SetDocExampleParameterValue("xml", "xml_description.xml");
        SetDocExampleParameterValue("outres", "resolution in meters for output (10, 20 or 30)");
        SetDocExampleParameterValue("out", "/path/to/output_image.tif");
        //SetDocExampleParameterValue("vp", "validation_polygons.shp");
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

    // The algorithm consists in a applying a formula for computing the NDVI for each pixel,
    // using BandMathFilter
    void DoExecute()
    {
       MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
       m_Resampler = 0;
        std::string xmlDesc = GetParameterAsString("xml");
        std::vector<char> buf(xmlDesc.begin(), xmlDesc.end());
        m_DirName = std::string(dirname(buf.data()));
        m_DirName += '/';
        auto meta = maccsMetadataReader->ReadMetadata(xmlDesc);
        // check if it is a sentinel 2 product, otherwise -> exception
        if (meta == nullptr)
            itkExceptionMacro("The metadata file could not be read !");

        if (meta->Header.FixedHeader.Mission.find("SENTINEL") == std::string::npos)
            itkExceptionMacro("Mission is not a SENTINEL !");

        int resolution = GetParameterInt("outres");
        if(resolution != 10 && resolution != 20)
            itkExceptionMacro("Accepted resolutions for Sentinel mission are 10 or 20 only!");

        m_AnglesRaster = OutputImageType::New();
        OutputImageType::IndexType start;

        start[0] =   0;  // first index on X
        start[1] =   0;  // first index on Y

        OutputImageType::SizeType size;

          size[0]  = 23;  // size along X
          size[1]  = 23;  // size along Y

        OutputImageType::RegionType region;

        region.SetSize(size);
        region.SetIndex(start);

        m_AnglesRaster->SetRegions(region);
        m_AnglesRaster->SetNumberOfComponentsPerPixel(10);
        m_AnglesRaster->Allocate();

        const auto &viewingAngles = ComputeViewingAngles(meta->ProductInformation.ViewingAngles);


        std::vector<size_t> bandsToExtract = {
            1, 2, 3, 7
        };
        for (int band = 0; band < 4; band++) {
            if(viewingAngles[bandsToExtract[band]].Angles.Zenith.Values.size() != ANGLES_GRID_SIZE ||
                viewingAngles[bandsToExtract[band]].Angles.Azimuth.Values.size() != ANGLES_GRID_SIZE )
                itkExceptionMacro("The width and/or height of computed angles from the xml file is/are not 23");
        }

        for(unsigned int i = 0; i < ANGLES_GRID_SIZE; i++) {
            for(unsigned int j = 0; j < ANGLES_GRID_SIZE; j++) {
                itk::VariableLengthVector<float> vct(10);
                vct[0] = meta->ProductInformation.SolarAngles.Zenith.Values[i][j];
                vct[1] = meta->ProductInformation.SolarAngles.Azimuth.Values[i][j];
                for (int band = 0; band < 4; band++) {
                    vct[band * 2 + 2] = viewingAngles[bandsToExtract[band]].Angles.Zenith.Values[i][j];
                    vct[band * 2 + 3] = viewingAngles[bandsToExtract[band]].Angles.Azimuth.Values[i][j];
                }

                OutputImageType::IndexType idx;
                idx[0] = j;
                idx[1] = i;
                m_AnglesRaster->SetPixel(idx, vct);
            }
        }
        m_AnglesRaster->UpdateOutputInformation();
        std::string resSuffix("_FRE_R1");
        if(resolution == 20)
            resSuffix = "_FRE_R2";
        std::string fileName = getMACCSRasterFileName(m_DirName, meta->ProductOrganization.ImageFiles, resSuffix, false);

        ImageReaderType::Pointer image = ImageReaderType::New();
        image->SetFileName(fileName);
        image->UpdateOutputInformation();;
        auto sz = image->GetOutput()->GetLargestPossibleRegion().GetSize();

        int width = sz[0];
        int height = sz[1];

        if(width == 0 || height == 0)
            itkExceptionMacro("The read width/height from the resolution metadata file is/are 0");
        createResampler(m_AnglesRaster, width, height);

        if(m_Resampler.GetPointer() == 0)
            itkExceptionMacro("Could not resample !");
        SetParameterOutputImage("out" , m_Resampler->GetOutput() );
    }

    void createResampler(const OutputImageType::Pointer& image, const int wantedWidth, const int wantedHeight) {

         m_Resampler = ResampleFilterType::New();
         m_Resampler->SetInput(image);

         // Set the interpolator
         LinearInterpolationType::Pointer interpolator = LinearInterpolationType::New();
         m_Resampler->SetInterpolator(interpolator);

         IdentityTransformType::Pointer transform = IdentityTransformType::New();

         m_Resampler->SetOutputParametersFromImage( image );
         // Scale Transform
         auto sz = image->GetLargestPossibleRegion().GetSize();
         OutputVectorType scale;
         scale[0] = (float)sz[0] / wantedWidth;
         scale[1] = (float)sz[1] / wantedHeight;

         // Evaluate spacing
         OutputImageType::SpacingType spacing = image->GetSpacing();
         OutputImageType::SpacingType OutputSpacing;
         OutputSpacing[0] = spacing[0] * scale[0];
         OutputSpacing[1] = spacing[1] * scale[1];

         m_Resampler->SetOutputSpacing(OutputSpacing);

         FloatVectorImageType::PointType origin = image->GetOrigin();
         FloatVectorImageType::PointType outputOrigin;
         outputOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale[0] - 1.0);
         outputOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale[1] - 1.0);

         m_Resampler->SetOutputOrigin(outputOrigin);

         m_Resampler->SetTransform(transform);

         ResampleFilterType::SizeType recomputedSize;
         recomputedSize[0] = wantedWidth;
         recomputedSize[1] = wantedHeight;

         m_Resampler->SetOutputSize(recomputedSize);
    }

    // Return the path to a file for which the name end in the ending
    std::string getMACCSRasterFileName(const std::string& rootFolder,
                                       const std::vector<MACCSFileInformation>& imageFiles,
                                       const std::string& ending,
                                       const bool fileTypeMeta) {

        for (const MACCSFileInformation& fileInfo : imageFiles) {
            if (fileInfo.LogicalName.length() >= ending.length() &&
                    0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
                return rootFolder + fileInfo.FileLocation.substr(0, fileInfo.FileLocation.find_last_of('.')) + (fileTypeMeta ?  ".HDR" : ".DBL.TIF");
            }

        }
        return "";
    }

    OutputImageType::Pointer            m_AnglesRaster;
    ResampleFilterType::Pointer         m_Resampler;

    std::string                         m_DirName;

};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::CreateS2AnglesRaster)
//  Software Guide :EndCodeSnippet


