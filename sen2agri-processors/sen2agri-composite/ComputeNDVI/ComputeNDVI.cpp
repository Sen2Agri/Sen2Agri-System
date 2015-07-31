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

#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkCastImageFilter.h"
#include "otbVectorImageToImageListFilter.h"
#include "itkResampleImageFilter.h"
#include "itkIndent.h"


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

    typedef float                                     PixelType;

    typedef otb::Image<PixelType, 2>                   OutputImageType;
    typedef otb::ImageList<OutputImageType>            ImageListType;
    typedef otb::ImageFileWriter<OutputImageType>      WriterType;
    typedef otb::VectorImageToImageListFilter<FloatVectorImageType, ImageListType>
    VectorImageToImageListType;
    typedef otb::MultiToMonoChannelExtractROI<FloatVectorImageType::InternalPixelType,
                                              FloatImageType::PixelType>    ExtractROIFilterType;
    typedef otb::ObjectList<ExtractROIFilterType>                           ExtractROIFilterListType;

    typedef otb::BandMathImageFilter<FloatImageType>   BMFilterType;



private:

    //  Software Guide : BeginLatex
    //  \code{DoInit()} method contains class information and description, parameter set up, and example values.
    //  Software Guide : EndLatex
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
        AddParameter(ParameterType_InputImage, "in", "Input Image");
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
        SetDocExampleParameterValue("in", "/path/to/input_image.tif");
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
        FloatVectorImageType::Pointer inImage = GetParameterImage("in");
        //inImage->UpdateOutputInformation();
        m_ChannelExtractorList = ExtractROIFilterListType::New();
        m_Filter               = BMFilterType::New();
        m_ImageList = VectorImageToImageListType::New();

        m_ImageList->SetInput(inImage);
        m_ImageList->UpdateOutputInformation();

        otbAppLogINFO( << "Input image has "
                     << inImage->GetNumberOfComponentsPerPixel()
                     << " components" << std::endl );

        for (unsigned int j = 0; j < inImage->GetNumberOfComponentsPerPixel(); j++)
        {
            std::ostringstream tmpParserVarName;
            tmpParserVarName << "b" << j + 1;
            m_Filter->SetNthInput(j, m_ImageList->GetOutput()->GetNthElement(j));
         }

        //  Now we can define the mathematical expression to perform on the layers (b3, b4).
        //  The filter takes advantage of the parsing capabilities of the muParser library and
        //  allows to set the expression as on a digital calculator.
        //
        //  The expression below returns 255 if the ratio $(NIR-RED)/(NIR+RED)$ is greater than 0.4 and 0 if not.
        //
        // TODO: use information from the xml regarding the bands NIR and RED indexes.
        std::string idxNIR("b"); idxNIR.append("4");
        std::string idxRED("b"); idxRED.append("3");
        std::string ndviExpr;
#ifdef OTB_MUPARSER_HAS_CXX_LOGICAL_OPERATORS
        ndviExpr = " (( " + idxNIR + " - " + idxRED + " )/( " + idxNIR + " + " + idxRED + " ) > 0.4) ? 255 : 0";
#else        
        ndviExpr = "if((" + idxNIR + "-" + idxRED + ") / (" + idxNIR + "+" + idxRED +") > 0.4, 255, 0)";
#endif

        m_Filter->SetExpression(ndviExpr);

        SetParameterOutputImage("out" , m_Filter->GetOutput() );
    }

};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::ComputeNDVI)
//  Software Guide :EndCodeSnippet


