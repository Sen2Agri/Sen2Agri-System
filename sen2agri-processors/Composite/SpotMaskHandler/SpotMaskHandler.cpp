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
#include "otbImageListToVectorImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkIndent.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkBinaryFunctorImageFilter.h"
#include "itkVariableLengthVector.h"
#include <vector>
#include "SpotMaskHandlerFunctor.h"
#include "libgen.h"
#include "MACCSMetadataReader.hpp"
#include "SPOT4MetadataReader.hpp"


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
//  SpotMaskHandler class is derived from Application class.
//
//  Software Guide : EndLatex
template< class TPixel>
class CustomFunctor
{
public:
  CustomFunctor() {}
  ~CustomFunctor() {}
  inline void SetTest(int x) {}
  /*
  bool operator!=(const CustomFunctor &) const
  {
    return false;
  }
  bool operator==(const CustomFunctor & other) const
  {
    return !( *this != other );
  }
  */
  inline TPixel operator()(const TPixel & A) const
  {
     Int16VectorImageType::PixelType var(2);
     var[0] = A[1];
     var[1] = A[2];
    return var;
  }
};

//  Software Guide : BeginCodeSnippet
class SpotMaskHandler : public Application
//  Software Guide : EndCodeSnippet
{
public:
    typedef enum flagVal {
        land,
        cloud,
        shadow,
        snow,
        water
    } FLAG_VALUE;

    //  Software Guide : BeginLatex
    // The \code{ITK} public types for the class, the superclass and smart pointers.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    typedef SpotMaskHandler Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    // Software Guide : EndCodeSnippet

    //  Software Guide : BeginLatex
    //  Invoke the macros necessary to respect ITK object factory mechanisms.
    //  Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    itkNewMacro(Self)

    itkTypeMacro(SpotMaskHandler, otb::Application)
    //  Software Guide : EndCodeSnippet

    typedef short                                       PixelShortType;

    typedef otb::ImageFileReader<Int16VectorImageType> ReaderType;

    typedef otb::ImageList<Int16ImageType>  ImageListType;
    typedef ImageListToVectorImageFilter<ImageListType,
                                         Int16VectorImageType >                   ListConcatenerFilterType;
    typedef MultiToMonoChannelExtractROI<Int16VectorImageType::InternalPixelType,
                                         Int16ImageType::PixelType>               ExtractROIFilterType;
    typedef ObjectList<ExtractROIFilterType>                                      ExtractROIFilterListType;

    typedef SpotMaskHandlerFunctor <Int16VectorImageType::PixelType, Int16VectorImageType::PixelType, Int16VectorImageType::PixelType> SpotMaskHandlerFunctorType;
    typedef itk::BinaryFunctorImageFilter< Int16VectorImageType, Int16VectorImageType, Int16VectorImageType,
                              SpotMaskHandlerFunctorType > FunctorFilterType;

    /*
    typedef itk::BinaryFunctorImageFilter< Int16VectorImageType, Int16VectorImageType, Int16VectorImageType,
                              CustomFunctor<Int16VectorImageType::PixelType> > BinaryFilterType;

    typedef itk::UnaryFunctorImageFilter< Int16VectorImageType, Int16VectorImageType,
                              CustomFunctor<Int16VectorImageType::PixelType> > FilterType__;
    */

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
        SetName("SpotMaskHandler");
        SetDescription("Extracts Cloud, Water and Snow masks from _div.tif and _sat.tif SPOT files ");

        SetDocName("SpotMaskHandler");
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
        // - input_img: Input image filename with bands for RED and NIR
        // - xml: Input xml filename with description for input image
        // The output parameters:
        // - output_img: Vector file containing reference data for training
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet
        AddParameter(ParameterType_String, "xml", "Input general L2A XML");

        //not mandatory

        //AddParameter(ParameterType_InputImage, "prevl3a", "Previous l3a product");
        //MandatoryOff("prevl3a");

        AddParameter(ParameterType_OutputImage, "out", "Out file for cloud mask");
        //AddParameter(ParameterType_OutputImage, "outw", "Out file for water mask");
        //AddParameter(ParameterType_OutputImage, "outs", "Out file for snow mask");

        m_ImageList = ImageListType::New();
        m_Concat = ListConcatenerFilterType::New();
        m_ExtractorList = ExtractROIFilterListType::New();
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
        const std::string &tmp = GetParameterAsString("xml");
        std::vector<char> buf(tmp.begin(), tmp.end());
        m_DirName = std::string(dirname(buf.data()));


        auto spot4Reader = itk::SPOT4MetadataReader::New();
        std::unique_ptr<SPOT4Metadata> metaSPOT;
        if ( (metaSPOT = spot4Reader->ReadMetadata(GetParameterAsString("xml"))) == nullptr)
        {
            itkExceptionMacro("The receieved xml file is not for  the SPOT mission");
            return;
        }

        m_ReaderCloud = ReaderType::New();
        m_ReaderCloud->SetFileName(m_DirName + "/" + metaSPOT->Files.MaskNua);

        m_ReaderWaterSnow = ReaderType::New();
        m_ReaderWaterSnow->SetFileName(m_DirName + "/" + metaSPOT->Files.MaskDiv);

        m_SpotMaskHandlerFunctor = FunctorFilterType::New();
        m_SpotMaskHandlerFunctor->SetFunctor(m_Functor);
        m_SpotMaskHandlerFunctor->SetInput1(m_ReaderCloud->GetOutput());
        m_SpotMaskHandlerFunctor->SetInput2(m_ReaderWaterSnow->GetOutput());
        m_SpotMaskHandlerFunctor->UpdateOutputInformation();
        m_SpotMaskHandlerFunctor->GetOutput()->SetNumberOfComponentsPerPixel(3);

        SetParameterOutputImage("out", m_SpotMaskHandlerFunctor->GetOutput());

        return;
    }

    Int16VectorImageType::Pointer       m_CSM, m_WSM;
    ImageListType::Pointer              m_ImageList;
    ListConcatenerFilterType::Pointer   m_Concat;
    ExtractROIFilterListType::Pointer   m_ExtractorList;
    FunctorFilterType::Pointer          m_SpotMaskHandlerFunctor;
    SpotMaskHandlerFunctorType          m_Functor;
    std::string                         m_DirName;

    ReaderType::Pointer                 m_ReaderCloud;
    ReaderType::Pointer                 m_ReaderWaterSnow ;

};

}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::SpotMaskHandler)
//  Software Guide :EndCodeSnippet


