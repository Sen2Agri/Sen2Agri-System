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
#include "MACCSMetadataReader.hpp"
#include "SPOT4MetadataReader.hpp"
#define LANDSAT    "LANDSAT"
#define SENTINEL   "SENTINEL"
#define L3A_SIZE            22
#define L3A_WEIGHT_LIMIT    10
#define L3A_REFL_LIMIT      21
namespace otb
{

namespace Wrapper
{

class CompositeSplitter : public Application
{
public:    

    //  Software Guide : BeginLatex
    // The \code{ITK} public types for the class, the superclass and smart pointers.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    typedef CompositeSplitter Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    // Software Guide : EndCodeSnippet

    //  Software Guide : BeginLatex
    //  Invoke the macros necessary to respect ITK object factory mechanisms.
    //  Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    itkNewMacro(Self)

    itkTypeMacro(CompositeSplitter, otb::Application)
    //  Software Guide : EndCodeSnippet


    typedef Int16VectorImageType                    InputImageType;
    typedef Int16VectorImageType                    OutImageType;

    typedef otb::ImageList<otb::ImageList<otb::VectorImage<short, 2> > >          InternalImageListType;
    typedef otb::Image<short, 2>                                                  OutImageType1;
    typedef otb::ImageList<OutImageType1>                                         OutputImageListType;
    typedef otb::VectorImageToImageListFilter<OutImageType, OutputImageListType>  VectorImageToImageListType;


    typedef otb::ImageList<OutImageType1>                                       ImgListType;
    typedef otb::VectorImage<short, 2>                                          ImageType;
    typedef otb::ImageListToVectorImageFilter<ImgListType, ImageType>           ImageListToVectorImageFilterType;
    typedef itk::MACCSMetadataReader                                            MACCSMetadataReaderType;
    typedef itk::SPOT4MetadataReader                                            SPOT4MetadataReaderType;


private:

    //  Software Guide : BeginLatex
    //  \code{DoInit()} method contains class information and description, parameter set up, and example values.
    //  Software Guide : EndLatex




    void DoInit()
    {
        SetName("CompositeSplitter");
        SetDescription("Extracts Cloud, Water and Snow masks from _div.tif and _sat.tif SPOT files ");

        SetDocName("CompositeSplitter");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("AG");
        SetDocSeeAlso(" ");        
        AddDocTag(Tags::Vector);
        AddParameter(ParameterType_InputImage, "in", "L3A product");
        AddParameter(ParameterType_String, "xml", "General xml input file for L2A");
        AddParameter(ParameterType_OutputImage, "outweights", "Out file weights");
        AddParameter(ParameterType_OutputImage, "outdates", "Out file weights");
        AddParameter(ParameterType_OutputImage, "outrefls", "Out file weights");
        AddParameter(ParameterType_OutputImage, "outflags", "Out file weights");

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

        m_WeightsConcat = ImageListToVectorImageFilterType::New();
        m_ReflsConcat = ImageListToVectorImageFilterType::New();
        m_DatesConcat = ImageListToVectorImageFilterType::New();
        m_FlagsConcat = ImageListToVectorImageFilterType::New();

        m_ReflectancesList = ImgListType::New();
        m_WeightList = ImgListType::New();
        m_DatesList = ImgListType::New();
        m_FlagsList = ImgListType::New();

        MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
        SPOT4MetadataReaderType::Pointer spot4MetadataReader = SPOT4MetadataReaderType::New();

        m_L3AIn = GetParameterInt16VectorImage("in");
        m_L3AIn->UpdateOutputInformation();
        if(m_L3AIn->GetNumberOfComponentsPerPixel() != L3A_SIZE)
        {
            itkExceptionMacro("Wrong number of bands ! " + m_L3AIn->GetNumberOfComponentsPerPixel());
        }
        //m_L3AIn->UpdateOutputInformation();
        m_ImgSplit = VectorImageToImageListType::New();
        m_ImgSplit->SetInput(m_L3AIn);
        m_ImgSplit->UpdateOutputInformation();
        int arrL3ABandPresence[L3A_SIZE] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

        if (auto meta = maccsMetadataReader->ReadMetadata(tmp)) {
            // add the information to the list
            if (meta->Header.FixedHeader.Mission.find(LANDSAT) != std::string::npos) {
                // Interpret landsat product
                int tmp[L3A_SIZE] = {0, 1, 2, -1, -1, -1, -1, 7, 8, 9, 10, 11, 12, 13, -1, -1, -1, -1, 18, 19, 20, 21};
                memcpy(arrL3ABandPresence, tmp, sizeof(tmp));

            } else if (meta->Header.FixedHeader.Mission.find(SENTINEL) != std::string::npos) {
                // Interpret sentinel product
                int tmp[L3A_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};
                memcpy(arrL3ABandPresence, tmp, sizeof(tmp));
            } else {
                itkExceptionMacro("Unknown mission: " + meta->Header.FixedHeader.Mission);
            }
        }else if (auto meta = spot4MetadataReader->ReadMetadata(tmp)) {
            // add the information to the list
            int tmp[L3A_SIZE] = {-1, 1, 2, -1, -1, -1, 6, -1, 8, -1, 10, -1, 12, 13, -1, -1, -1, 17, -1, 19, -1, 21};
            memcpy(arrL3ABandPresence, tmp, sizeof(tmp));

        } else {
            itkExceptionMacro("Unable to read metadata from " << tmp);
        }

        int i = 0;
        for(i = 0; i < L3A_WEIGHT_LIMIT; i++) {
            if(arrL3ABandPresence[i] != -1)
                m_WeightList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(i));
        }
        m_DatesList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(i++));
        for(; i < L3A_REFL_LIMIT; i++) {
            if(arrL3ABandPresence[i] != -1)
                m_ReflectancesList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(i));
        }
        m_FlagsList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(i++));

        m_WeightsConcat = ImageListToVectorImageFilterType::New();
        m_WeightsConcat->SetInput(m_WeightList);
        SetParameterOutputImagePixelType("outweights", ImagePixelType_int16);
        SetParameterOutputImage("outweights", m_WeightsConcat->GetOutput());

        m_DatesConcat = ImageListToVectorImageFilterType::New();
        m_DatesConcat->SetInput(m_DatesList);
        SetParameterOutputImagePixelType("outdates", ImagePixelType_int16);
        SetParameterOutputImage("outdates", m_DatesConcat->GetOutput());

        m_ReflsConcat = ImageListToVectorImageFilterType::New();
        m_ReflsConcat->SetInput(m_ReflectancesList);
        SetParameterOutputImagePixelType("outrefls", ImagePixelType_int16);
        SetParameterOutputImage("outrefls", m_ReflsConcat->GetOutput());

        m_FlagsConcat = ImageListToVectorImageFilterType::New();
        m_FlagsConcat->SetInput(m_FlagsList);
        SetParameterOutputImagePixelType("outflags", ImagePixelType_int16);
        SetParameterOutputImage("outflags", m_FlagsConcat->GetOutput());
        return;
    }

    InputImageType::Pointer             m_L3AIn;

    VectorImageToImageListType::Pointer       m_ImgSplit;
    ImageListToVectorImageFilterType::Pointer m_WeightsConcat;
    ImageListToVectorImageFilterType::Pointer m_ReflsConcat;
    ImageListToVectorImageFilterType::Pointer m_DatesConcat;
    ImageListToVectorImageFilterType::Pointer m_FlagsConcat;

    ImgListType::Pointer m_ReflectancesList;
    ImgListType::Pointer m_WeightList;
    ImgListType::Pointer m_DatesList;
    ImgListType::Pointer m_FlagsList;

    std::string         m_DirName;

};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::CompositeSplitter)



