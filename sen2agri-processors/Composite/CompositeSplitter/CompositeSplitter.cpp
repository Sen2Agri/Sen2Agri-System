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
#include "MetadataHelperFactory.h"

#define L3A_SIZE            22
#define L3A_ALL_WEIGHT_CNT    10
#define L3A_10M_WEIGHT_CNT    4
#define L3A_20M_WEIGHT_CNT    6
#define L3A_RGB_CNT    3

//                                            {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21};
int S2_ARR_BANDS_PRESENCE_ALL[L3A_SIZE]     = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int L8_ARR_BANDS_PRESENCE_ALL[L3A_SIZE]     = {1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1};
int SPOT_ARR_BANDS_PRESENCE_ALL[L3A_SIZE]   = {0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1};

int S2_ARR_BANDS_PRESENCE_10M[L3A_SIZE]     = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int L8_ARR_BANDS_PRESENCE_10M[L3A_SIZE]     = {1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int SPOT_ARR_BANDS_PRESENCE_10M[L3A_SIZE]   = {0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int S2_ARR_BANDS_PRESENCE_20M[L3A_SIZE]     = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
int L8_ARR_BANDS_PRESENCE_20M[L3A_SIZE]     = {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
int SPOT_ARR_BANDS_PRESENCE_20M[L3A_SIZE]   = {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0};

int S2_ARR_BANDS_RGB_ALL[L3A_RGB_CNT]       = {13, 12, 11};
int L8_ARR_BANDS_RGB_ALL[L3A_RGB_CNT]       = {13, 12, 11};
int SPOT_ARR_BANDS_RGB_ALL[L3A_RGB_CNT]     = {17, 13, 12};

int S2_ARR_BANDS_RGB_10M[L3A_RGB_CNT]       = {7, 6, 5};
int L8_ARR_BANDS_RGB_10M[L3A_RGB_CNT]       = {7, 6, 5};
int SPOT_ARR_BANDS_RGB_10M[L3A_RGB_CNT]     = {8, 7, 8};

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

        AddParameter(ParameterType_Int, "res", "Input current L3A XML");
        SetDefaultParameterInt("res", -1);
        MandatoryOff("res");

        AddParameter(ParameterType_Int, "allinone", "Specifies if all bands of 10 and 20M are present in the L3A product");
        SetDefaultParameterInt("allinone", 0);
        MandatoryOff("allinone");

        AddParameter(ParameterType_Int, "stypemode", "Single product type mode");
        SetDefaultParameterInt("stypemode", 0);
        MandatoryOff("stypemode");

        AddParameter(ParameterType_OutputImage, "outrgb", "Output rgb filename");
        MandatoryOff("outrgb");
    }

    void DoUpdateParameters()
    {
      // Nothing to do.
    }

    void DoExecute()
    {
        int arrL3ABandPresence[L3A_SIZE];
        memset(arrL3ABandPresence, 0, sizeof(arrL3ABandPresence));

        bool isCombinedProductsTypeMode = (GetParameterInt("stypemode") == 0);
        int resolution = GetParameterInt("res");
        bool allInOne = (GetParameterInt("allinone") != 0);
        const std::string inXml = GetParameterAsString("xml");

        otbAppLogINFO( << "InXML: " << inXml << std::endl );
        otbAppLogINFO( << "Combined products mode: " << isCombinedProductsTypeMode << std::endl );
        otbAppLogINFO( << "Resolution: " << resolution << std::endl );
        otbAppLogINFO( << "All in one: " << allInOne << std::endl );
        if(resolution == -1 && !allInOne) {
            allInOne = true;
            otbAppLogINFO( << "As resolution and allinone were not set, allinone will be automatically considered!" << std::endl );
        }

        m_WeightsConcat = ImageListToVectorImageFilterType::New();
        m_RGBConcat   = ImageListToVectorImageFilterType::New();
        m_ReflsConcat = ImageListToVectorImageFilterType::New();
        m_DatesConcat = ImageListToVectorImageFilterType::New();
        m_FlagsConcat = ImageListToVectorImageFilterType::New();

        m_ReflectancesList = ImgListType::New();
        m_RGBOutList = ImgListType::New();
        m_WeightList = ImgListType::New();
        m_DatesList = ImgListType::New();
        m_FlagsList = ImgListType::New();

        m_L3AIn = GetParameterInt16VectorImage("in");
        m_L3AIn->UpdateOutputInformation();

        int nReflsBandsNo = 0;
        if(allInOne) {
            nReflsBandsNo = L3A_ALL_WEIGHT_CNT;
        } else {
            if(resolution == 10) {
                nReflsBandsNo = L3A_10M_WEIGHT_CNT;
            } else if(resolution == 20) {
                nReflsBandsNo = L3A_20M_WEIGHT_CNT;
            } else {
                itkExceptionMacro("This resolution " << resolution << " can be used only with allinone option");
            }
        }
        unsigned int nTotalBandsNo = (2*nReflsBandsNo+2);
        if(m_L3AIn->GetNumberOfComponentsPerPixel() != nTotalBandsNo)
        {
            itkExceptionMacro("Wrong number of bands ! " + m_L3AIn->GetNumberOfComponentsPerPixel());
        }

        m_ImgSplit = VectorImageToImageListType::New();
        m_ImgSplit->SetInput(m_L3AIn);
        m_ImgSplit->UpdateOutputInformation();

        auto factory = MetadataHelperFactory::New();
        auto pHelper = factory->GetMetadataHelper(inXml);
        std::string missionName = pHelper->GetMissionName();

        // add the information to the list
        if (missionName.find(LANDSAT_MISSION_STR) != std::string::npos) {
            if(allInOne) {
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(L8_ARR_BANDS_RGB_ALL[0]));
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(L8_ARR_BANDS_RGB_ALL[1]));
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(L8_ARR_BANDS_RGB_ALL[2]));
                memcpy(arrL3ABandPresence, L8_ARR_BANDS_PRESENCE_ALL, sizeof(arrL3ABandPresence));
            } else if (resolution == 10) {
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(L8_ARR_BANDS_RGB_10M[0]));
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(L8_ARR_BANDS_RGB_10M[1]));
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(L8_ARR_BANDS_RGB_10M[2]));
                memcpy(arrL3ABandPresence, L8_ARR_BANDS_PRESENCE_10M, sizeof(arrL3ABandPresence));
            } else if (resolution == 20) {
                memcpy(arrL3ABandPresence, L8_ARR_BANDS_PRESENCE_20M, sizeof(arrL3ABandPresence));
            }
        } else if (missionName.find(SENTINEL_MISSION_STR) != std::string::npos) {
            if(allInOne) {
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(S2_ARR_BANDS_RGB_ALL[0]));
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(S2_ARR_BANDS_RGB_ALL[1]));
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(S2_ARR_BANDS_RGB_ALL[2]));
                memcpy(arrL3ABandPresence, S2_ARR_BANDS_PRESENCE_ALL, sizeof(arrL3ABandPresence));
            } else if (resolution == 10) {
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(S2_ARR_BANDS_RGB_10M[0]));
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(S2_ARR_BANDS_RGB_10M[1]));
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(S2_ARR_BANDS_RGB_10M[2]));
                memcpy(arrL3ABandPresence, S2_ARR_BANDS_PRESENCE_10M, sizeof(arrL3ABandPresence));
            } else if (resolution == 20) {
                memcpy(arrL3ABandPresence, S2_ARR_BANDS_PRESENCE_20M, sizeof(arrL3ABandPresence));
            }
        } else if (missionName.find(SPOT4_MISSION_STR) != std::string::npos) {
            if(allInOne) {
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(SPOT_ARR_BANDS_RGB_ALL[0]));
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(SPOT_ARR_BANDS_RGB_ALL[1]));
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(SPOT_ARR_BANDS_RGB_ALL[2]));
                memcpy(arrL3ABandPresence, SPOT_ARR_BANDS_PRESENCE_ALL, sizeof(arrL3ABandPresence));
            } else if (resolution == 10) {
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(SPOT_ARR_BANDS_RGB_10M[0]));
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(SPOT_ARR_BANDS_RGB_10M[1]));
                m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(SPOT_ARR_BANDS_RGB_10M[2]));
                memcpy(arrL3ABandPresence, SPOT_ARR_BANDS_PRESENCE_10M, sizeof(arrL3ABandPresence));
            } else if (resolution == 20) {
                memcpy(arrL3ABandPresence, SPOT_ARR_BANDS_PRESENCE_20M, sizeof(arrL3ABandPresence));
            }
        } else {
            itkExceptionMacro("Unknown mission name " << missionName);
        }

        if(m_RGBOutList->Size() > 0) {
            if(!HasValue("outrgb"))
                itkExceptionMacro("The parameter outrgb has to be present. This parameter indicates the filename for rgb");
            m_RGBConcat = ImageListToVectorImageFilterType::New();
            m_RGBConcat->SetInput(m_RGBOutList);
            SetParameterOutputImagePixelType("outrgb", ImagePixelType_int16);
            SetParameterOutputImage("outrgb", m_RGBConcat->GetOutput());
        }

        int cnt = 0;
        for(int i = 0; i < nReflsBandsNo; i++) {
            if(arrL3ABandPresence[cnt] || isCombinedProductsTypeMode) {
                m_WeightList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(cnt));
            }
            cnt++;
        }
        m_DatesList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(cnt++));
        for(int i = 0; i < nReflsBandsNo; i++) {
            if(arrL3ABandPresence[cnt] || isCombinedProductsTypeMode) {
                m_ReflectancesList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(cnt));
            }
            cnt++;
        }
        m_FlagsList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(cnt++));

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
        SetParameterOutputImagePixelType("outflags", ImagePixelType_uint8);
        SetParameterOutputImage("outflags", m_FlagsConcat->GetOutput());

        return;
    }

    InputImageType::Pointer             m_L3AIn;

    VectorImageToImageListType::Pointer       m_ImgSplit;
    ImageListToVectorImageFilterType::Pointer m_WeightsConcat;
    ImageListToVectorImageFilterType::Pointer m_ReflsConcat;
    ImageListToVectorImageFilterType::Pointer m_DatesConcat;
    ImageListToVectorImageFilterType::Pointer m_FlagsConcat;
    ImageListToVectorImageFilterType::Pointer m_RGBConcat;

    ImgListType::Pointer m_ReflectancesList;
    ImgListType::Pointer m_RGBOutList;
    ImgListType::Pointer m_WeightList;
    ImgListType::Pointer m_DatesList;
    ImgListType::Pointer m_FlagsList;
};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::CompositeSplitter)



