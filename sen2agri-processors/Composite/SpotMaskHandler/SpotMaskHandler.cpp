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
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"

#include "itkBinaryFunctorImageFilter.h"

#include <vector>
#include "libgen.h"
#include "SpotMaskHandlerFunctor.h"
#include "MACCSMetadataReader.hpp"
#include "SPOT4MetadataReader.hpp"

namespace otb
{

namespace Wrapper
{

class SpotMaskHandler : public Application
{
public:    

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

    typedef otb::ImageFileReader<Int16VectorImageType>                          ReaderType;
    typedef otb::ImageList<Int16ImageType>                                      ImageListType;
    typedef ImageListToVectorImageFilter<ImageListType, Int16VectorImageType >  ListConcatenerFilterType;
    typedef SpotMaskHandlerFunctor <Int16VectorImageType::PixelType,
                                    Int16VectorImageType::PixelType, Int16VectorImageType::PixelType> SpotMaskHandlerFunctorType;
    typedef itk::BinaryFunctorImageFilter< Int16VectorImageType, Int16VectorImageType,
                                            Int16VectorImageType, SpotMaskHandlerFunctorType > FunctorFilterType;

private:

    //  Software Guide : BeginLatex
    //  \code{DoInit()} method contains class information and description, parameter set up, and example values.
    //  Software Guide : EndLatex




    void DoInit()
    {
        SetName("SpotMaskHandler");
        SetDescription("Extracts Cloud, Water and Snow masks from _div.tif and _sat.tif SPOT files ");

        SetDocName("SpotMaskHandler");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("AG");
        SetDocSeeAlso(" ");        
        AddDocTag(Tags::Vector);        
        AddParameter(ParameterType_String, "xml", "General xml input file for L2A");
        AddParameter(ParameterType_OutputImage, "out", "Out file for cloud mask");

    }

    void DoUpdateParameters()
    {
      // Nothing to do.
    }

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

    ReaderType::Pointer                 m_ReaderCloud;
    ReaderType::Pointer                 m_ReaderWaterSnow ;
    FunctorFilterType::Pointer          m_SpotMaskHandlerFunctor;
    SpotMaskHandlerFunctorType          m_Functor;
    std::string                         m_DirName;
};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::SpotMaskHandler)



