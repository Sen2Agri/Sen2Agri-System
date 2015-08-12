#if(1)
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

    FloatVectorImageType::Pointer       m_L2AIn;
    FloatVectorImageType::Pointer       m_CSM, m_WM, m_SM, m_WeightsL2A;
    FloatVectorImageType::Pointer       m_PrevL3A;
    FloatVectorImageType::Pointer       m_PrevWeightPixel, m_WeightAvgDate, m_WeightAvgRef, m_PixelStat;
    ImageListType::Pointer              m_ImageList;
    ListConcatenerFilterType::Pointer   m_Concat;
    ExtractROIFilterListType::Pointer   m_ExtractorList;
    FunctorFilterType::Pointer          m_UpdateSynthesisFunctor;
    UpdateSynthesisFunctorType          m_Functor;



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
        AddParameter(ParameterType_InputFilename, "xmlcur", "Input current L2A XML");
        AddParameter(ParameterType_InputFilename, "xmlgen", "Input general L2A XML");
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

        //TODO: get sensor type from local XML
        SensorType sensorType = SENSOR_S2;
        //TODO: get resolution from local XML
        ResolutionType resType = RES_10M;

        m_Concat->SetInput(m_ImageList);
        m_Functor.Initialize(sensorType, resType, l3aExist);
        m_UpdateSynthesisFunctor = FunctorFilterType::New();
        m_UpdateSynthesisFunctor->SetFunctor(m_Functor);
        m_UpdateSynthesisFunctor->SetInput(m_Concat->GetOutput());
        //m_UpdateSynthesisFunctor->UpdateOutputInformation();
        if(sensorType == SENSOR_S2)
            m_UpdateSynthesisFunctor->GetOutput()->SetNumberOfComponentsPerPixel(10);
        else
            m_UpdateSynthesisFunctor->GetOutput()->SetNumberOfComponentsPerPixel(14);


        SetParameterOutputImage("out", m_UpdateSynthesisFunctor->GetOutput());

        return;
    }

//BinaryFilterType::Pointer binaryFilter;
};

}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::UpdateSynthesis)
//  Software Guide :EndCodeSnippet
#endif

#if(0)
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
#include "otbImageList.h"
#include "itkResampleImageFilter.h"
#include "itkIndent.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkBinaryFunctorImageFilter.h"
#include <vector>

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
  bool operator!=(const CustomFunctor &) const
  {
    return false;
  }
  bool operator==(const CustomFunctor & other) const
  {
    return !( *this != other );
  }
  inline TPixel operator()(const TPixel & A,
                            const TPixel & B) const
  {
    return A + B;
  }
};
//  Software Guide : BeginCodeSnippet
class UpdateSynthesis : public Application
//  Software Guide : EndCodeSnippet
{
public:
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

    typedef float                                     PixelType;

    typedef otb::Image<PixelType, 2>                    ImageType;

    typedef otb::Image<PixelType, 2>                   OutputImageType;
    //typedef otb::ImageList<OutputImageType>            ImageListType;
    typedef otb::ImageList<FloatImageType>  ImageListType;
    typedef otb::ImageFileWriter<OutputImageType>      WriterType;
   // typedef otb::VectorImageToImageListFilter<FloatVectorImageType, ImageListType>
    //VectorImageToImageListType;
    /*
    typedef otb::MultiToMonoChannelExtractROI<FloatVectorImageType::InternalPixelType,
                                              FloatImageType::PixelType>    ExtractROIFilterType;*/
     typedef otb::MultiToMonoChannelExtractROI<FloatVectorImageType::InternalPixelType,
                                               FloatImageType::PixelType>           ExtractROIFilterType;
    typedef otb::ObjectList<ExtractROIFilterType>                           ExtractROIFilterListType;

    typedef otb::BandMathImageFilter<FloatImageType>   BMFilterType;

    typedef itk::ImageRegionIterator<FloatImageType> IteratorType;
    typedef itk::ImageRegionConstIterator<FloatImageType> ConstIteratorType;

    typedef itk::VectorIndexSelectionCastImageFilter<FloatVectorImageType, ImageType> IndexSelectionType;

    typedef itk::BinaryFunctorImageFilter< FloatVectorImageType, FloatVectorImageType, FloatVectorImageType,
                              CustomFunctor<FloatVectorImageType::PixelType> > FilterType;

private:

    //  Software Guide : BeginLatex
    //  \code{DoInit()} method contains class information and description, parameter set up, and example values.
    //  Software Guide : EndLatex
    ExtractROIFilterType::Pointer     m_ExtractROIFilter;
    ExtractROIFilterListType::Pointer m_ChannelExtractorList1;
    ExtractROIFilterListType::Pointer m_ChannelExtractorList2;
    ExtractROIFilterListType::Pointer m_ChannelExtractorListOut;
    std::vector<ExtractROIFilterType::Pointer> chanExtractorVector1;
    std::vector<ExtractROIFilterType::Pointer> chanExtractorVector2;
    FloatVectorImageType::Pointer m_inputImage1;
    FloatVectorImageType::Pointer m_inputImage2;
    ImageType::Pointer m_outputImage;
    //VectorImageToImageListType::Pointer imageList1;
    //VectorImageToImageListType::Pointer imageList2;
    ExtractROIFilterType::Pointer m_extractROI;

    BMFilterType::Pointer  m_Filter;

    ImageListType::Pointer m_ImageList1;
    ImageListType::Pointer m_ImageList2;

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
        m_ImageList1 = ImageListType::New();
        m_ImageList2 = ImageListType::New();
        m_ChannelExtractorList1 = ExtractROIFilterListType::New();
        m_ChannelExtractorList2 = ExtractROIFilterListType::New();

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
        AddParameter(ParameterType_InputImage, "in1", "Input Image 1");
        AddParameter(ParameterType_InputImage, "in2", "Input Image 2");

        AddParameter(ParameterType_OutputImage, "out", "Out Image");


        // Set default value for parameters
        //SetDefaultParameterFloat("ratio", 0.75);
         //  Software Guide : EndCodeSnippet

        // Software Guide : BeginLatex
        // An example commandline is automatically generated. Method \code{SetDocExampleParameterValue()} is
        // used to set parameters. Dataset should be located in  \code{OTB-Data/Examples} directory.
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet
        SetDocExampleParameterValue("in1", "/path/to/input_image_1.tif");
        SetDocExampleParameterValue("in2", "/path/to/input_image_2.tif");
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

        unsigned int i = 0, j = 0;
        //Int16ImageType::Pointer inImage1 = GetParameterInt16Image("in1");
        FloatVectorImageType::Pointer inImage1 = GetParameterImage("in1");
        FloatVectorImageType::Pointer inImage2 = GetParameterImage("in2");
        inImage1->UpdateOutputInformation();
        inImage2->UpdateOutputInformation();
        //ConstIteratorType inputIt1(inImage1, inImage1->GetLargestPossibleRegion());
        unsigned int nbChannels1 = inImage1->GetNumberOfComponentsPerPixel();

        for (j = 0; j < inImage1->GetNumberOfComponentsPerPixel(); j++)
        {
            ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
            extractor->SetInput(inImage1);
            extractor->SetChannel(j + 1);
            extractor->GetOutput()->UpdateOutputInformation();
            m_ChannelExtractorList1->PushBack(extractor);
            m_ImageList1->PushBack( extractor->GetOutput() );
        }
        for (j = 0; j < inImage2->GetNumberOfComponentsPerPixel(); j++)
        {
            ExtractROIFilterType::Pointer extractor = ExtractROIFilterType::New();
            extractor->SetInput(inImage2);
            extractor->SetChannel(j + 1);
            extractor->GetOutput()->UpdateOutputInformation();
            m_ChannelExtractorList2->PushBack(extractor);
            m_ImageList2->PushBack( extractor->GetOutput() );
        }
        if(m_ChannelExtractorList1->Size() != m_ChannelExtractorList2->Size()) {
            std::cerr << "Unmatched sizes" << std::endl;
            return;
        }
        IteratorType inputIt1(m_ImageList1->GetNthElement(0), m_ImageList1->GetNthElement(0)->GetRequestedRegion());
        for (inputIt1.GoToBegin()/*, inputIt2.GoToBegin(), outputIt.GoToBegin()*/;
             !inputIt1.IsAtEnd() /*||  !inputIt2.IsAtEnd() || !outputIt.IsAtEnd()*/;
             ++inputIt1/*, ++inputIt2, ++outputIt*/)
          {
          //ImageType::IndexType idx = outputIt.GetIndex();
          //idx[0] =  requestedIndex[0] + requestedSize[0] - 1 - idx[0];
//                  outputIt.Set(inputImage->GetPixel(idx));

            std::cout << "add" << std::endl;

            const double dA = static_cast< double >( inputIt1.Get() );
            //const double dB = static_cast< double >( inputIt2.Get() );
            double diff = dA - 3;
            PixelType pixel = static_cast<PixelType>(diff);
            //outputIt.Set(pixel);
          }


/*
        FilterType::Pointer filter = FilterType::New();
        filter->SetInput1(inImage1);
        filter->SetInput2(inImage2);
        //filter->UpdateOutputInformation();
        SetParameterOutputImage("out" , filter->GetOutput());
        return;
        */
        /*
        itk::Index<2> pixelIndex;
        pixelIndex.Fill(0);

        ImageType::PixelType input1PixelValue = image1->GetPixel(pixelIndex);
        ImageType::PixelType input2PixelValue = image2->GetPixel(pixelIndex);
        ImageType::PixelType outputPixelValue = filter->GetOutput()->GetPixel(pixelIndex);
*/

#if(0)
        m_extractROI = ExtractROIFilterType::New();

        m_extractROI->SetExtractionRegion(inImage1->GetLargestPossibleRegion());
        m_extractROI->SetInput(inImage1);

        m_ChannelExtractorList1 = ExtractROIFilterListType::New();
        m_ChannelExtractorList2 = ExtractROIFilterListType::New();
        m_ChannelExtractorListOut = ExtractROIFilterListType::New();

        m_Filter               = BMFilterType::New();




        VectorImageToImageListType::Pointer imageList1 = VectorImageToImageListType::New();
        imageList1->SetInput(inImage1);

        imageList1->UpdateOutputInformation();
        VectorImageToImageListType::Pointer imageList2 = VectorImageToImageListType::New();
        imageList2->SetInput(inImage2);

        imageList2->UpdateOutputInformation();

        unsigned int nbChannels1 = inImage1->GetNumberOfComponentsPerPixel();
        unsigned int nbChannels2 = inImage2->GetNumberOfComponentsPerPixel();

        if(nbChannels1 != nbChannels2) {
            std::cerr << "Unmatched sizes" << std::endl;
            return;
        }
        for(i = 0; i < nbChannels1 ; ++i) {
            m_extractROI->SetChannel(i);
            ConstIteratorType inputIt1(m_extractROI->GetOutput(), m_extractROI->GetOutput()->GetLargestPossibleRegion());
        }

        for(i = 0; i < nbChannels1 ; ++i) {
            m_inputImage1 = imageList1->GetOutput()->GetNthElement(i);
            std::cout << "First Update" << std::endl;
            m_inputImage1->UpdateOutputInformation();
            ConstIteratorType inputIt1(m_inputImage1, m_inputImage1->GetLargestPossibleRegion());
            ConstIteratorType inputIt2(m_inputImage2, m_inputImage2->GetLargestPossibleRegion());
            m_outputImage = ImageType::New();
            std::cout << "1" << std::endl;
            m_outputImage->SetRegions(imageList1->GetOutput()->GetNthElement(i)->GetLargestPossibleRegion());
            std::cout << "1" << std::endl;
            m_outputImage->CopyInformation(imageList1->GetOutput()->GetNthElement(i));
            std::cout << "2" << std::endl;
            m_outputImage->Allocate();
            std::cout << "3" << std::endl;

//            itk::ImageRegionIterator<FloatVectorImageType> outputIt(outputImage, outputImage->GetRequestedRegion());
            IteratorType outputIt(m_outputImage, m_outputImage->GetLargestPossibleRegion());
            for (inputIt1.GoToBegin()/*, inputIt2.GoToBegin(), outputIt.GoToBegin()*/;
                 !inputIt1.IsAtEnd() /*||  !inputIt2.IsAtEnd() || !outputIt.IsAtEnd()*/;
                 ++inputIt1/*, ++inputIt2, ++outputIt*/)
              {
              //ImageType::IndexType idx = outputIt.GetIndex();
              //idx[0] =  requestedIndex[0] + requestedSize[0] - 1 - idx[0];
//                  outputIt.Set(inputImage->GetPixel(idx));

                std::cout << "add" << std::endl;

                const double dA = static_cast< double >( inputIt1.Get() );
                const double dB = static_cast< double >( inputIt2.Get() );
                double diff = dA - dB;
                PixelType pixel = static_cast<PixelType>(diff);
                outputIt.Set(pixel);
              }
            std::cout << "4" << std::endl;

        }

        for(i = 0; i < nbChannels1 ; ++i)
          {
            chanExtractorVector1.push_back(ExtractROIFilterType::New());
            chanExtractorVector1.back()->SetInput(inImage1);
            chanExtractorVector1.back()->SetChannel(i+1);
          }
        for(i = 0; i < nbChannels2 ; ++i)
          {
            chanExtractorVector2.push_back(ExtractROIFilterType::New());
            chanExtractorVector2.back()->SetInput(inImage2);
            chanExtractorVector2.back()->SetChannel(i+1);
          }

        for (j = 0; j < inImage1->GetNumberOfComponentsPerPixel(); j++)
        {
            m_ExtractROIFilter = ExtractROIFilterType::New();
            m_ExtractROIFilter->SetInput(inImage1);
            m_ExtractROIFilter->SetChannel(j + 1);
            m_ExtractROIFilter->GetOutput()->UpdateOutputInformation();
            m_ChannelExtractorList1->PushBack(m_ExtractROIFilter);
        }
        for (j = 0; j < inImage2->GetNumberOfComponentsPerPixel(); j++)
        {
            m_ExtractROIFilter = ExtractROIFilterType::New();
            m_ExtractROIFilter->SetInput(inImage2);
            m_ExtractROIFilter->SetChannel(j + 1);
            m_ExtractROIFilter->GetOutput()->UpdateOutputInformation();
            m_ChannelExtractorList2->PushBack(m_ExtractROIFilter);
        }
        if(m_ChannelExtractorList1->Size() != m_ChannelExtractorList2->Size()) {
            std::cerr << "Unmatched sizes" << std::endl;
            return;
        }

        //ImageType::SizeType outputSize;

        //outputSize[0] = static_cast<double>(inputSize[0]) * 2;
        //outputSize[1] = static_cast<double>(inputSize[1]) * 2;
        ImageType::SpacingType outputSpacing;
        //ImageType::SizeType inputSize = reader->GetOutput()->GetLargestPossibleRegion().GetSize();
        //outputSpacing[0] = reader->GetOutput()->GetSpacing()[0] * (static_cast<double>(inputSize[0]) / static_cast<double>(outputSize[0]));
        //outputSpacing[1] = reader->GetOutput()->GetSpacing()[1] * (static_cast<double>(inputSize[1]) / static_cast<double>(outputSize[1]));
        //outputSpacing[0] = reader->GetOutput()->GetSpacing()[0] * (static_cast<double>(inputSize[0]) / (static_cast<double>(inputSize[0]) * 2) );
        //outputSpacing[1] = reader->GetOutput()->GetSpacing()[1] * (static_cast<double>(inputSize[1]) / (static_cast<double>(inputSize[1]) * 2));
        for(i = 0, j = 0; i < 1/*m_ChannelExtractorList1.size()*/; i++, j++)
        {
            //ImageType::Pointer inputImage1 = m_ChannelExtractorList1->GetNthElement(i)->GetOutput();
            m_inputImage1 = m_ChannelExtractorList1->GetNthElement(i)->GetOutput();
            //ImageType::Pointer inputImage1 = chanExtractorVector1[j]->GetOutput();//imageList1->GetOutput()->GetNthElement(j);
            std::cout << "First Update" << std::endl;
            m_inputImage1->Update();
            ConstIteratorType inputIt1(m_inputImage1, m_inputImage1->GetLargestPossibleRegion());
            //ImageType::Pointer inputImage2 = m_ChannelExtractorList2->GetNthElement(i)->GetOutput();
            //ImageType::Pointer inputImage2 = chanExtractorVector2[j]->GetOutput();//imageList1->GetOutput()->GetNthElement(j);
            std::cout << "Second Update" << std::endl;
            //inputImage2->Update();
            std::cout << "Second Update finished" << std::endl;
            ConstIteratorType inputIt2( m_ChannelExtractorList2->GetNthElement(i)->GetOutput(),  m_ChannelExtractorList2->GetNthElement(i)->GetOutput()->GetLargestPossibleRegion());
            m_outputImage = ImageType::New();
            std::cout << "1" << std::endl;
            m_outputImage->SetRegions(m_ChannelExtractorList2->GetNthElement(i)->GetOutput()->GetLargestPossibleRegion());
            std::cout << "1" << std::endl;
            m_outputImage->CopyInformation(m_ChannelExtractorList2->GetNthElement(i)->GetOutput());
            std::cout << "2" << std::endl;
            m_outputImage->Allocate();
            std::cout << "3" << std::endl;

//            itk::ImageRegionIterator<FloatVectorImageType> outputIt(outputImage, outputImage->GetRequestedRegion());
            IteratorType outputIt(m_outputImage, m_outputImage->GetLargestPossibleRegion());
            for (inputIt1.GoToBegin()/*, inputIt2.GoToBegin(), outputIt.GoToBegin()*/;
                 !inputIt1.IsAtEnd() /*||  !inputIt2.IsAtEnd() || !outputIt.IsAtEnd()*/;
                 ++inputIt1/*, ++inputIt2, ++outputIt*/)
              {
              //ImageType::IndexType idx = outputIt.GetIndex();
              //idx[0] =  requestedIndex[0] + requestedSize[0] - 1 - idx[0];
//                  outputIt.Set(inputImage->GetPixel(idx));

                std::cout << "add" << std::endl;
                //FloatVectorImageType::PixelType pixel;
                //std::cout << "3" << std::endl;
                //pixel.SetSize(1);
                //pixel.SetElement(0, inputIt1.Get() + inputIt2.Get());
                //pixel.SetElement(1, inputIt1.Get() - inputIt2.Get());
                const double dA = static_cast< double >( inputIt1.Get() );
                const double dB = static_cast< double >( inputIt2.Get() );
                double diff = dA - dB;
                PixelType pixel = static_cast<PixelType>(diff);
                outputIt.Set(pixel);
              }
            std::cout << "4" << std::endl;
            //SetParameterOutputImagePixelType("out", ImagePixelType_float);
            //FloatVectorImageType tmp

//            ExtractROIFilterType::Pointer tmpExtractROIFilter = ExtractROIFilterType::New();
//            tmpExtractROIFilter->SetInput(outputImage);
//            tmpExtractROIFilter->SetChannel(j + 1);
//            tmpExtractROIFilter->GetOutput()->UpdateOutputInformation();

            SetParameterOutputImage("out" , m_outputImage.GetPointer());//tmpExtractROIFilter->GetOutput());

         }

#endif
    }

};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::UpdateSynthesis)
//  Software Guide :EndCodeSnippet


#endif
