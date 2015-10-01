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
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbBandMathImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"

#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkCastImageFilter.h"
#include "otbVectorImageToImageListFilter.h"
#include "itkResampleImageFilter.h"
#include "itkIndent.h"
#include <vector>
#include "libgen.h"


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
//  ComputeNDVI class is derived from Application class.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
class ComputeNDVI : public Application
//  Software Guide : EndCodeSnippet
{
public:
    //  Software Guide : BeginLatex
    // The \code{ITK} public types for the class, the superclass and smart pointers.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    typedef ComputeNDVI Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    // Software Guide : EndCodeSnippet

    //  Software Guide : BeginLatex
    //  Invoke the macros necessary to respect ITK object factory mechanisms.
    //  Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    itkNewMacro(Self)

    itkTypeMacro(ComputeNDVI, otb::Application)
    //  Software Guide : EndCodeSnippet

    typedef short                                     PixelType;

    typedef otb::Image<PixelType, 2>                   OutputImageType;
    typedef Int16VectorImageType                                 ImageType;
    typedef otb::ImageFileReader<ImageType>                            ImageReaderType;

    typedef otb::ImageList<OutputImageType>            ImageListType;
    typedef otb::ImageFileWriter<OutputImageType>      WriterType;
    typedef otb::VectorImageToImageListFilter<ImageType, ImageListType>    VectorImageToImageListType;

    typedef otb::MultiToMonoChannelExtractROI<FloatVectorImageType::InternalPixelType,
                                              FloatImageType::PixelType>    ExtractROIFilterType;
    typedef otb::ObjectList<ExtractROIFilterType>                           ExtractROIFilterListType;

    typedef otb::BandMathImageFilter<Int16ImageType>   BMFilterType;



private:

    //  Software Guide : BeginLatex
    //  \code{DoInit()} method contains class information and description, parameter set up, and example values.
    //  Software Guide : EndLatex

    typedef itk::MACCSMetadataReader                                   MACCSMetadataReaderType;


    ExtractROIFilterType::Pointer     m_ExtractROIFilter;
    ExtractROIFilterListType::Pointer m_ChannelExtractorList;

    BMFilterType::Pointer  m_Filter;
    VectorImageToImageListType::Pointer m_ImageList;

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
        SetName("ComputeNDVI");
        SetDescription("Computes NDVI from RED and NIR bands");

        SetDocName("ComputeNDVI");
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
        //AddParameter(ParameterType_InputImage, "in", "Input Image");
        AddParameter(ParameterType_InputFilename, "xml", "Xml Desc");

        AddParameter(ParameterType_OutputImage, "out", "Out Image");


        // Set default value for parameters
        //SetDefaultParameterFloat("ratio", 0.75);
         //  Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
        // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet
        //SetDocExampleParameterValue("in", "/path/to/input_image.tif");
        SetDocExampleParameterValue("xml", "xml_description.xml");
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
        std::string xmlDesc = GetParameterAsString("xml");
        std::vector<char> buf(xmlDesc.begin(), xmlDesc.end());
        m_DirName = std::string(dirname(buf.data()));
        m_DirName += '/';
        auto meta = maccsMetadataReader->ReadMetadata(xmlDesc);
        // check if it is a sentinel 2 product, otherwise -> exception
        if (meta != nullptr) {
            if (meta->Header.FixedHeader.Mission.find("SENTINEL") == std::string::npos) {
                itkExceptionMacro("Mission is not a SENTINEL !");
            }
        }
        else
            itkExceptionMacro("Mission is not a SENTINEL !");

        std::string imageFile1 = getMACCSRasterFileName(m_DirName, (*meta).ProductOrganization.ImageFiles, "_FRE_R1");
        if(imageFile1.length() <= 0)
            itkExceptionMacro("Couldn't get the FRE_R1 file name !");
        m_InImage = ImageReaderType::New();
        m_InImage->SetFileName(imageFile1);
        m_InImage->UpdateOutputInformation();

        //inImage->UpdateOutputInformation();
        m_ChannelExtractorList = ExtractROIFilterListType::New();
        m_Filter               = BMFilterType::New();

        m_ImageList = VectorImageToImageListType::New();

        m_ImageList->SetInput(m_InImage->GetOutput());
        m_ImageList->UpdateOutputInformation();
        if(m_InImage->GetOutput()->GetNumberOfComponentsPerPixel() < 4)
            itkExceptionMacro("The image has less than 4 bands, which is not acceptable for a SENTINEL-S2 product with resolution 10 meters !");

        unsigned int j = 0;
        for (j = 0; j < m_InImage->GetOutput()->GetNumberOfComponentsPerPixel(); j++)
            m_Filter->SetNthInput(j, m_ImageList->GetOutput()->GetNthElement(j));

        // The significance of the bands is:
        // b1 - G
        // b2 - R
        // b3 - NIR
        // b4 - SWIR
        std::string ndviExpr;
#ifdef OTB_MUPARSER_HAS_CXX_LOGICAL_OPERATORS
        ndviExpr = "(b3==-10000 || b2==-10000) ? -10000 : (abs(b3+b2)<0.000001) ? 0 : 10000 * (b3-b2)/(b3+b2)";
#else        
        ndviExpr = "if(b3==-10000 or b2==-10000,-10000,if(abs(b3+b2)<0.000001,0,(b3-b2)/(b3+b2)";
#endif

        m_Filter->SetExpression(ndviExpr);
        SetParameterOutputImagePixelType("out", ImagePixelType_int16);

        SetParameterOutputImage("out" , m_Filter->GetOutput() );
    }

    std::string getMACCSRasterFileName(const std::string& rootFolder, const std::vector<MACCSFileInformation>& imageFiles, const std::string& ending) {

        for (const MACCSFileInformation& fileInfo : imageFiles) {
            if (fileInfo.LogicalName.length() >= ending.length() &&
                    0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
                return rootFolder + fileInfo.FileLocation.substr(0, fileInfo.FileLocation.find_last_of('.')) + ".DBL.TIF";
            }

        }
        return "";
    }

    ImageReaderType::Pointer            m_InImage;

    std::string                         m_DirName;

};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::ComputeNDVI)
//  Software Guide :EndCodeSnippet


