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
#include "UpdateSynthesisFunctor.h"
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
//  UpdateSynthesis class is derived from Application class.
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
class UpdateSynthesis : public Application
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
    typedef UpdateSynthesis Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    // Software Guide : EndCodeSnippet

    //  Software Guide : BeginLatex
    //  Invoke the macros necessary to respect ITK object factory mechanisms.
    //  Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    itkNewMacro(Self)

    itkTypeMacro(UpdateSynthesis, otb::Application)
    //  Software Guide : EndCodeSnippet

    typedef float                                   PixelType;
    typedef short                                   PixelShortType;

    typedef otb::ImageList<FloatImageType>  ImageListType;
    typedef ImageListToVectorImageFilter<ImageListType,
                                         FloatVectorImageType >                   ListConcatenerFilterType;
    typedef MultiToMonoChannelExtractROI<FloatVectorImageType::InternalPixelType,
                                         FloatImageType::PixelType>               ExtractROIFilterType;
    typedef ObjectList<ExtractROIFilterType>                                      ExtractROIFilterListType;

    typedef UpdateSynthesisFunctor <FloatVectorImageType::PixelType, FloatVectorImageType::PixelType> UpdateSynthesisFunctorType;
    typedef itk::UnaryFunctorImageFilter< FloatVectorImageType, FloatVectorImageType,
                              UpdateSynthesisFunctorType > FunctorFilterType;

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
        SetName("UpdateSynthesis");
        SetDescription("Computes NDVI from RED and NIR bands");

        SetDocName("UpdateSynthesis");
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
        AddParameter(ParameterType_InputImage, "in", "L2A MACSS product");
        AddParameter(ParameterType_Int, "res", "Input current L2A XML");
        AddParameter(ParameterType_InputFilename, "xml", "Input general L2A XML");
        // AddParameter(ParameterType_InputImage, "ref", "Reflectance raster"); -> TODO: compute it
        AddParameter(ParameterType_InputImage, "csm", "Cloud-Shadow Mask");
        AddParameter(ParameterType_InputImage, "wm", "Water Mask");
        AddParameter(ParameterType_InputImage, "sm", "Snow Mask");
        AddParameter(ParameterType_InputImage, "wl2a", "Weights of L2A product for date N");

        //not mandatory

        AddParameter(ParameterType_InputImage, "prevl3a", "Previous l3a product");
        MandatoryOff("prevl3a");
        /*
        AddParameter(ParameterType_InputImage, "prevw", "Weight for each pixel obtained so far");
        MandatoryOff("prevw");
        AddParameter(ParameterType_InputImage, "wavgdate", "Weighted average date for L3A product so far");
        MandatoryOff("wavgdate");
        AddParameter(ParameterType_InputImage, "wavgref", "Weighted average reflectance value so far, for each pixel and each spectral band");
        MandatoryOff("wavgref");
        AddParameter(ParameterType_InputImage, "pixstat", "Status of each L3A pixel: cloud, water, snow");
        MandatoryOff("pixstat");
        */
        // out rasters for L3A product
        /*
        AddParameter(ParameterType_OutputImage, "outw", "Out weight counter for each pixel and for each band");
        AddParameter(ParameterType_OutputImage, "outdate", "Out weighted average date for L3A product so far");
        AddParameter(ParameterType_OutputImage, "outro", "Out weighted average reflectance value so far for each pixel and each spectral band");
        AddParameter(ParameterType_OutputImage, "outstat", "Out status of each L3A pixel: cloud, water, snow");
        */

        //test only
        AddParameter(ParameterType_OutputImage, "out", "Out weight counter for each pixel and for each band");

        m_ImageList = ImageListType::New();
        m_Concat = ListConcatenerFilterType::New();
        m_ExtractorList = ExtractROIFilterListType::New();

        // Set default value for parameters
        //SetDefaultParameterFloat("ratio", 0.75);
         //  Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
        // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet
        //SetDocExampleParameterValue("in1", "/path/to/input_image_1.tif");
        //SetDocExampleParameterValue("in2", "/path/to/input_image_2.tif");
        //SetDocExampleParameterValue("out", "/path/to/output_image.tif");
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
        int resType = GetParameterInt("res");

        m_L2AIn = GetParameterFloatVectorImage("in");
        //m_L2AIn->UpdateOutputInformation();
        m_CSM = GetParameterFloatVectorImage("csm");
        //m_CSM->UpdateOutputInformation();
        m_WM = GetParameterFloatVectorImage("wm");
        //m_WM->UpdateOutputInformation();
        m_SM = GetParameterFloatVectorImage("sm");
        //m_SM->UpdateOutputInformation();
        m_WeightsL2A = GetParameterFloatVectorImage("wl2a");
        //Int16VectorImageType::Pointer inImage2 = GetParameterInt16VectorImage("in2");

        unsigned int j=0;
        for(j=0; j < m_L2AIn->GetNumberOfComponentsPerPixel(); j++)
        {
            ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
            extractor->SetInput( m_L2AIn );
            extractor->SetChannel( j+1 );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );
            m_ImageList->PushBack( extractor->GetOutput() );
        }
        // will the following have only one band, for sure? if yes, simply push_back each one of them
        for(j=0; j < m_CSM->GetNumberOfComponentsPerPixel(); j++)
        {
            ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
            extractor->SetInput( m_CSM );
            extractor->SetChannel( j+1 );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );
            m_ImageList->PushBack( extractor->GetOutput() );
        }

        for(j=0; j < m_WM->GetNumberOfComponentsPerPixel(); j++)
        {
            ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
            extractor->SetInput( m_WM );
            extractor->SetChannel( j+1 );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );
            m_ImageList->PushBack( extractor->GetOutput() );
        }

        for(j=0; j < m_SM->GetNumberOfComponentsPerPixel(); j++)
        {
            ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
            extractor->SetInput( m_SM );
            extractor->SetChannel( j+1 );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );
            m_ImageList->PushBack( extractor->GetOutput() );
        }

        for(j=0; j < m_WeightsL2A->GetNumberOfComponentsPerPixel(); j++)
        {
            ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
            extractor->SetInput( m_WeightsL2A );
            extractor->SetChannel( j+1 );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );
            m_ImageList->PushBack( extractor->GetOutput() );
        }

        bool l3aExist = false;
        if(HasValue("prevl3a")) {
            m_PrevL3A = GetParameterFloatVectorImage("prevl3a");
            l3aExist = true;
            for(j=0; j < m_PrevL3A->GetNumberOfComponentsPerPixel(); j++)
            {
                ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
                extractor->SetInput( m_PrevL3A );
                extractor->SetChannel( j+1 );
                extractor->UpdateOutputInformation();
                m_ExtractorList->PushBack( extractor );
                m_ImageList->PushBack( extractor->GetOutput() );
            }
        }

        SensorType sensorType = SENSOR_S2;
        auto maccsReader = itk::MACCSMetadataReader::New();
        if (auto m = maccsReader->ReadMetadata(GetParameterAsString("xml")))
            sensorType = SENSOR_S2;//m->ProductInformation
        else
        {
            auto spot4Reader = itk::SPOT4MetadataReader::New();
            if (auto m = spot4Reader->ReadMetadata(GetParameterAsString("xml")))
                sensorType = SENSOR_L8;
        }

        m_Concat->SetInput(m_ImageList);
        m_Functor.Initialize(sensorType, (resType == 10 ? RES_10M : RES_20M), l3aExist);
        m_UpdateSynthesisFunctor = FunctorFilterType::New();
        m_UpdateSynthesisFunctor->SetFunctor(m_Functor);
        m_UpdateSynthesisFunctor->SetInput(m_Concat->GetOutput());
        m_UpdateSynthesisFunctor->UpdateOutputInformation();
        if(sensorType == SENSOR_S2)
            m_UpdateSynthesisFunctor->GetOutput()->SetNumberOfComponentsPerPixel(10);
        else
            m_UpdateSynthesisFunctor->GetOutput()->SetNumberOfComponentsPerPixel(14);


        SetParameterOutputImage("out", m_UpdateSynthesisFunctor->GetOutput());

        return;
    }
    FloatVectorImageType::Pointer       m_L2AIn;
    FloatVectorImageType::Pointer       m_CSM, m_WM, m_SM, m_WeightsL2A;
    FloatVectorImageType::Pointer       m_PrevL3A;
    FloatVectorImageType::Pointer       m_PrevWeightPixel, m_WeightAvgDate, m_WeightAvgRef, m_PixelStat;
    ImageListType::Pointer              m_ImageList;
    ListConcatenerFilterType::Pointer   m_Concat;
    ExtractROIFilterListType::Pointer   m_ExtractorList;
    FunctorFilterType::Pointer          m_UpdateSynthesisFunctor;
    UpdateSynthesisFunctorType          m_Functor;
};

}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::UpdateSynthesis)
//  Software Guide :EndCodeSnippet


