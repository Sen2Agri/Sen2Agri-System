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
#include "otbStreamingResampleImageFilter.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbImageFileWriter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkCastImageFilter.h"
#include "otbVectorImageToImageListFilter.h"
#include "itkResampleImageFilter.h"
#include "itkIndent.h"
#include "MACCSMetadataReader.hpp"
#include "SPOT4MetadataReader.hpp"
#include "itkInterpolateImageFunction.h"
#include "libgen.h"

//Transform
#include "itkScalableAffineTransform.h"
#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"


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
//  ResampleAtS2Res class is derived from Application class.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
class ResampleAtS2Res : public Application
//  Software Guide : EndCodeSnippet
{
public:
    //  Software Guide : BeginLatex
    // The \code{ITK} public types for the class, the superclass and smart pointers.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    typedef ResampleAtS2Res Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    // Software Guide : EndCodeSnippet

    //  Software Guide : BeginLatex
    //  Invoke the macros necessary to respect ITK object factory mechanisms.
    //  Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    itkNewMacro(Self)

    itkTypeMacro(ResampleAtS2Res, otb::Application)
    //  Software Guide : EndCodeSnippet

    typedef float                                      PixelType;
    typedef otb::VectorImage<PixelType, 2>             ImageType;
    typedef otb::Image<PixelType, 2>                   InternalImageType;
    typedef otb::ImageList<ImageType>                  ImageListType;
    typedef otb::ImageList<InternalImageType>          InternalImageListType;
    typedef otb::ImageFileWriter<ImageType>            WriterType;

    typedef otb::MultiToMonoChannelExtractROI<ImageType::InternalPixelType,
                                              InternalImageType::PixelType>     ExtractROIFilterType;
    typedef otb::ObjectList<ExtractROIFilterType>                               ExtractROIFilterListType;

    typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
    typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;
    typedef otb::ImageListToVectorImageFilter<InternalImageListType,
                                         ImageType >                   ListConcatenerFilterType;

    typedef otb::StreamingResampleImageFilter<InternalImageType, InternalImageType, double>     ResampleFilterType;
    typedef otb::ObjectList<ResampleFilterType>                                                 ResampleFilterListType;

    typedef itk::NearestNeighborInterpolateImageFunction<InternalImageType, double>             NearestNeighborInterpolationType;
    typedef itk::LinearInterpolateImageFunction<InternalImageType, double>                      LinearInterpolationType;
    typedef itk::IdentityTransform<double, InternalImageType::ImageDimension>                   IdentityTransformType;

    typedef itk::ScalableAffineTransform<double, InternalImageType::ImageDimension>             ScalableTransformType;

    typedef ScalableTransformType::OutputVectorType     OutputVectorType;

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
        SetName("ResampleAtS2Res");
        SetDescription("Computes NDVI from RED and NIR bands");

        SetDocName("ResampleAtS2Res");
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
        m_ConcatenerRes10 = ListConcatenerFilterType::New();
        m_ConcatenerRes20 = ListConcatenerFilterType::New();
        m_ImageListRes10 = InternalImageListType::New();
        m_ImageListRes20 = InternalImageListType::New();
        m_ResamplersList = ResampleFilterListType::New();
        m_ExtractorList = ExtractROIFilterListType::New();
        m_ImageReaderList = ImageReaderListType::New();

        // Software Guide : BeginLatex
        // The input parameters:
        // - in: Input image filename with bands for RED and NIR
        // - xml: Input xml filename with description for input image
        // The output parameters:
        // - out: Vector file containing reference data for training
        // Software Guide : EndLatex

        //  Software Guide : BeginCodeSnippet

        AddParameter(ParameterType_String, "xml", "Xml description");
        AddParameter(ParameterType_String, "spotmask", "Image with 3 bands as masks, cloud, water snow for SPOT only");
        MandatoryOff("spotmask");

        AddParameter(ParameterType_OutputImage, "outres10", "Out Image");
        AddParameter(ParameterType_OutputImage, "outres20", "Out Image");
        AddParameter(ParameterType_OutputImage, "outcmres10", "Out Image");
        AddParameter(ParameterType_OutputImage, "outwmres10", "Out Image");
        AddParameter(ParameterType_OutputImage, "outcmres20", "Out Image");
        AddParameter(ParameterType_OutputImage, "outwmres20", "Out Image");


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
        const std::string &tmp = GetParameterAsString("xml");
        std::vector<char> buf(tmp.begin(), tmp.end());
        m_DirName = std::string(dirname(buf.data()));

        auto maccsReader = itk::MACCSMetadataReader::New();
        if (auto m = maccsReader->ReadMetadata(GetParameterAsString("xml")))
        {
            ProcessLANDSAT8(m);
        }
        else
        {
            auto spot4Reader = itk::SPOT4MetadataReader::New();
            if (auto m = spot4Reader->ReadMetadata(GetParameterAsString("xml")))
            {
                ProcessSPOT4(m);
            }
        }
        m_ConcatenerRes10->SetInput( m_ImageListRes10 );

        SetParameterOutputImage("outres10", m_ConcatenerRes10->GetOutput());

        m_ConcatenerRes20->SetInput( m_ImageListRes20 );

        SetParameterOutputImage("outres20", m_ConcatenerRes20->GetOutput());

        SetParameterOutputImage("outcmres10", m_ImageCloudRes10.GetPointer());

        SetParameterOutputImage("outwmres10", m_ImageWaterRes10.GetPointer());

        SetParameterOutputImage("outcmres20", m_ImageCloudRes20.GetPointer());

        SetParameterOutputImage("outwmres20", m_ImageWaterRes20.GetPointer());
    }


    bool ProcessSPOT4(const std::unique_ptr<SPOT4Metadata>& meta)
    {
        if(meta->Radiometry.Bands.size() != 4) {
            itkExceptionMacro("Wrong number of bands for SPOT4: " + meta->Radiometry.Bands.size() );
            return false;
        }

        std::string imageFile = m_DirName + "/" + meta->Files.OrthoSurfCorrPente;

        ImageReaderType::Pointer reader = getReader(imageFile);

        std::vector<std::string>::iterator it;
        int i = 0;
        ResampleFilterType::Pointer resampler;
        ExtractROIFilterType::Pointer extractor;
        for (it = meta->Radiometry.Bands.begin(), i = 1; it != meta->Radiometry.Bands.end(); it++, i++)
        {
            extractor = ExtractROIFilterType::New();
            extractor->SetInput( reader->GetOutput() );
            extractor->SetChannel( i );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );
            if((*it).compare("XS1") == 0 || (*it).compare("XS2") == 0 || (*it).compare("XS3") == 0) {
                // resample from 20m to 10m
                resampler = getResampler(extractor->GetOutput(), 2.0, false);

                m_ImageListRes10->PushBack(resampler->GetOutput());
            }
            else
                if((*it).compare("SWIR") == 0)
                    m_ImageListRes20->PushBack(extractor->GetOutput());
                else {
                    itkExceptionMacro("Wrong band name for SPOT4: " + (*it));
                    return false;
                }
        }
        imageFile = m_DirName + "/" +meta->Files.MaskNua;

        if(!HasValue("spotmask")) {
            itkExceptionMacro("The mask file for SPOT was not provided, do set 'spotmask' flag ");
            return false;
        }
        ImageReaderType::Pointer spotMasks = getReader(GetParameterAsString("spotmask"));

        extractor = ExtractROIFilterType::New();
        extractor->SetInput( spotMasks->GetOutput() );
        extractor->SetChannel( 1 );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );
        m_ImageCloudRes20 = extractor->GetOutput();
        resampler = getResampler(extractor->GetOutput(), 2.0, true);
        m_ImageCloudRes10 = resampler->GetOutput();

        extractor = ExtractROIFilterType::New();
        extractor->SetInput( spotMasks->GetOutput() );
        extractor->SetChannel( 2 );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );
        m_ImageWaterRes20 = extractor->GetOutput();
        resampler = getResampler(extractor->GetOutput(), 2.0, true);
        m_ImageWaterRes10 = resampler->GetOutput();



/*
        ImageReaderType::Pointer readerCloud = getReader(m_DirName + "/" + meta->Files.MaskNua);

        extractor = ExtractROIFilterType::New();
        extractor->SetInput( readerCloud->GetOutput() );
        extractor->SetChannel( 1 );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );
        m_ImageCloudRes20 = extractor->GetOutput();
        resampler = getResampler(extractor->GetOutput(), 2.0, true);
        m_ImageCloudRes10 = resampler->GetOutput();


        ImageReaderType::Pointer readerWater = getReader(m_DirName + "/" + meta->Files.MaskDiv);
        extractor = ExtractROIFilterType::New();
        extractor->SetInput( readerWater->GetOutput() );
        extractor->SetChannel( 1 );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );
        m_ImageWaterRes20 = extractor->GetOutput();
        resampler = getResampler(extractor->GetOutput(), 2.0, true);
        m_ImageWaterRes10 = resampler->GetOutput();
*/
        return true;

    }

    bool ProcessLANDSAT8(const std::unique_ptr<MACCSFileMetadata>& meta)
    {
        if(meta->ImageInformation.Bands.size() != 8) {
            itkExceptionMacro("Wrong number of bands for LANDSAT: " + meta->ImageInformation.Bands.size() );
            return false;
        }
        std::string imageXMLFile("");
        std::string cloudXMLFile("");
        std::string waterXMLFile("");
        for(auto fileInf = meta->ProductOrganization.ImageFiles.begin(); fileInf != meta->ProductOrganization.ImageFiles.end(); fileInf++)
        {
            if(fileInf->LogicalName.substr(fileInf->LogicalName.size() - 4, 4).compare("_FRE") == 0 && fileInf->FileLocation.size() > 0)
                imageXMLFile = m_DirName + "/" + fileInf->FileLocation;
            else
            if(fileInf->LogicalName.substr(fileInf->LogicalName.size() - 4, 4).compare("_CLD") == 0 && fileInf->FileLocation.size() > 0)
                cloudXMLFile = m_DirName + "/" + fileInf->FileLocation;
            else
            if(fileInf->LogicalName.substr(fileInf->LogicalName.size() - 4, 4).compare("_WAT") == 0 && fileInf->FileLocation.size() > 0)
                waterXMLFile = m_DirName + "/" + fileInf->FileLocation;


        }
        if(imageXMLFile.empty()) //TODO add error msg
            return false;

        auto maccsImageReader = itk::MACCSMetadataReader::New();
        std::unique_ptr<MACCSFileMetadata> imageMeta = nullptr;

        if ((imageMeta = maccsImageReader->ReadMetadata(imageXMLFile)) == nullptr) //TODO add error msg
            return false;

        if(imageMeta->ImageInformation.Bands.size() != 8) //TODO add error msg
            return false;

        //creates image filename
        std::string imageFile = imageXMLFile;
        imageFile.replace(imageFile.size() - 4, 4, ".DBL.TIF");

        ImageReaderType::Pointer reader = getReader(imageFile);

        std::vector<MACCSBand>::iterator it;
        int i = 0;
        ResampleFilterType::Pointer resampler;
        ExtractROIFilterType::Pointer extractor;
        for (it = imageMeta->ImageInformation.Bands.begin(), i = 1; it != imageMeta->ImageInformation.Bands.end(); it++, i++)
        {
            extractor = ExtractROIFilterType::New();
            extractor->SetInput( reader->GetOutput() );
            extractor->SetChannel( i );
            extractor->UpdateOutputInformation();
            m_ExtractorList->PushBack( extractor );
            if((*it).Name.compare("B2") == 0 || (*it).Name.compare("B3") == 0 || (*it).Name.compare("B4") == 0) {
                // resample from 30m to 10m
                resampler = getResampler(extractor->GetOutput(), 3.0, false);

                m_ImageListRes10->PushBack(resampler->GetOutput());
            }
            else
                if((*it).Name.compare("B5") == 0 || (*it).Name.compare("B7") == 0 || (*it).Name.compare("B8") == 0) {
                    // resample from 30m to 20m
                    resampler = getResampler(extractor->GetOutput(), 1.5, false);
                    m_ImageListRes20->PushBack(extractor->GetOutput());
                }
        }

        imageFile = cloudXMLFile;
        imageFile.replace(imageFile.size() - 4, 4, ".DBL.TIF");

        ImageReaderType::Pointer readerCloud = getReader(imageFile);

        extractor = ExtractROIFilterType::New();
        extractor->SetInput( readerCloud->GetOutput() );
        extractor->SetChannel( 1 );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );

        resampler = getResampler(extractor->GetOutput(), 3.0, true);
        m_ImageCloudRes10 = resampler->GetOutput();
        resampler = getResampler(extractor->GetOutput(), 2.0, true);
        m_ImageCloudRes20 = extractor->GetOutput();

        imageFile = waterXMLFile;
        imageFile.replace(imageFile.size() - 4, 4, ".DBL.TIF");

        ImageReaderType::Pointer readerWater = getReader(imageFile);

        extractor = ExtractROIFilterType::New();
        extractor->SetInput( readerWater->GetOutput() );
        extractor->SetChannel( 1 );
        extractor->UpdateOutputInformation();
        m_ExtractorList->PushBack( extractor );

        resampler = getResampler(extractor->GetOutput(), 3.0, true);
        m_ImageWaterRes10 = resampler->GetOutput();
        resampler = getResampler(extractor->GetOutput(), 2.0, true);
        m_ImageWaterRes20 = extractor->GetOutput();

        return true;

    }

    ResampleFilterType::Pointer getResampler(const InternalImageType::Pointer& image, const float& ratio, bool isMask) {
         ResampleFilterType::Pointer resampler = ResampleFilterType::New();
         resampler->SetInput(image);

         // Set the interpolator
         if(isMask) {
             NearestNeighborInterpolationType::Pointer interpolator = NearestNeighborInterpolationType::New();
             resampler->SetInterpolator(interpolator);
         }
         else {
            LinearInterpolationType::Pointer interpolator = LinearInterpolationType::New();
            resampler->SetInterpolator(interpolator);
         }

         IdentityTransformType::Pointer transform = IdentityTransformType::New();

         resampler->SetOutputParametersFromImage( image );
         // Scale Transform
         OutputVectorType scale;
         scale[0] = 1.0 / ratio;
         scale[1] = 1.0 / ratio;

         // Evaluate spacing
         InternalImageType::SpacingType spacing = image->GetSpacing();
         InternalImageType::SpacingType OutputSpacing;
         OutputSpacing[0] = spacing[0] * scale[0];
         OutputSpacing[1] = spacing[1] * scale[1];

         resampler->SetOutputSpacing(OutputSpacing);

         FloatVectorImageType::PointType origin = image->GetOrigin();
         FloatVectorImageType::PointType outputOrigin;
         outputOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale[0] - 1.0);
         outputOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale[1] - 1.0);

         resampler->SetOutputOrigin(outputOrigin);

         resampler->SetTransform(transform);

         // Evaluate size
         ResampleFilterType::SizeType recomputedSize;
         recomputedSize[0] = 4;//image->GetLargestPossibleRegion().GetSize()[0] / scale[0];
         recomputedSize[1] = image->GetLargestPossibleRegion().GetSize()[1] / scale[1];

         resampler->SetOutputSize(recomputedSize);

         m_ResamplersList->PushBack(resampler);
         return resampler;
    }

    // get a reader from the file path
    ImageReaderType::Pointer getReader(const std::string& filePath) {
        ImageReaderType::Pointer reader = ImageReaderType::New();

        // set the file name
        reader->SetFileName(filePath);

        // add it to the list and return
        m_ImageReaderList->PushBack(reader);
        return reader;
    }

    ListConcatenerFilterType::Pointer     m_ConcatenerRes10;
    ListConcatenerFilterType::Pointer     m_ConcatenerRes20;
    ExtractROIFilterListType::Pointer     m_ExtractorList;
    ImageReaderListType::Pointer          m_ImageReaderList;
    InternalImageListType::Pointer        m_ImageListRes10;
    InternalImageListType::Pointer        m_ImageListRes20;
    InternalImageType::Pointer            m_ImageCloudRes10;
    InternalImageType::Pointer            m_ImageWaterRes10;
    InternalImageType::Pointer            m_ImageCloudRes20;
    InternalImageType::Pointer            m_ImageWaterRes20;
    ResampleFilterType::Pointer           m_ResamplerCloudRes10;
    ResampleFilterType::Pointer           m_ResamplerWaterRes10;
    ExtractROIFilterType::Pointer         m_ExtractROIFilter;
    ExtractROIFilterListType::Pointer     m_ChannelExtractorList;
    ResampleFilterListType::Pointer       m_ResamplersList;
    std::string                           m_DirName;
};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::ResampleAtS2Res)
//  Software Guide :EndCodeSnippet


