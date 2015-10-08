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

#include "MaskExtractorFilter.hxx"

#include <vector>
#include "libgen.h"
#include "MaskHandlerFunctor.h"
#include "MACCSMetadataReader.hpp"
#include "SPOT4MetadataReader.hpp"

#define LANDSAT    "LANDSAT"
#define SENTINEL   "SENTINEL"

namespace otb
{

namespace Wrapper
{

class MaskHandler : public Application
{
public:    

    //  Software Guide : BeginLatex
    // The \code{ITK} public types for the class, the superclass and smart pointers.
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    typedef MaskHandler Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    // Software Guide : EndCodeSnippet

    //  Software Guide : BeginLatex
    //  Invoke the macros necessary to respect ITK object factory mechanisms.
    //  Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
    itkNewMacro(Self)

    itkTypeMacro(MaskHandler, otb::Application)
    //  Software Guide : EndCodeSnippet

    typedef otb::ImageFileReader<Int16VectorImageType>                          ReaderType;
    typedef otb::ImageList<Int16ImageType>                                      ImageListType;
    typedef ImageListToVectorImageFilter<ImageListType, Int16VectorImageType >  ListConcatenerFilterType;
    typedef MaskHandlerFunctor <Int16VectorImageType::PixelType,
                                    Int16VectorImageType::PixelType, Int16VectorImageType::PixelType> MaskHandlerFunctorType;

    typedef itk::BinaryFunctorImageFilter< Int16VectorImageType, Int16VectorImageType,
                                            Int16VectorImageType, MaskHandlerFunctorType > FunctorFilterType;

    typedef otb::MaskExtractorFilter<Int16VectorImageType, Int16VectorImageType> MaskExtractorFilterType;

private:

    //  Software Guide : BeginLatex
    //  \code{DoInit()} method contains class information and description, parameter set up, and example values.
    //  Software Guide : EndLatex




    void DoInit()
    {
        SetName("MaskHandler");
        SetDescription("Extracts Cloud, Water and Snow masks from _div.tif and _sat.tif SPOT files ");

        SetDocName("MaskHandler");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("AG");
        SetDocSeeAlso(" ");        
        AddDocTag(Tags::Vector);        
        AddParameter(ParameterType_String, "xml", "General xml input file for L2A");
        AddParameter(ParameterType_Int, "sentinelres", "Resolution for which the masks sould be handled, SENTINEL-S2 only");
        MandatoryOff("sentinelres");

        AddParameter(ParameterType_OutputImage, "out", "Out file for cloud, water and snow mask");

    }

    void DoUpdateParameters()
    {
      // Nothing to do.
    }
#if(0)
    void DoExecute()
    {
        const std::string &tmp = GetParameterAsString("xml");
        std::vector<char> buf(tmp.begin(), tmp.end());
        m_DirName = std::string(dirname(buf.data()));
        m_DirName += '/';

        m_SpotMaskHandlerFunctor = FunctorFilterType::New();
        m_ReaderCloud = ReaderType::New();
        m_ReaderWaterSnow = ReaderType::New();

        auto spot4Reader = itk::SPOT4MetadataReader::New();
        auto maccsReader = itk::MACCSMetadataReader::New();
        if ( std::unique_ptr<SPOT4Metadata> metaSPOT = spot4Reader->ReadMetadata(GetParameterAsString("xml") ))
        {
            m_ReaderCloud->SetFileName(m_DirName + metaSPOT->Files.MaskNua);
            m_ReaderWaterSnow->SetFileName(m_DirName + metaSPOT->Files.MaskDiv);
            m_SpotMaskHandlerFunctor->GetFunctor().SetBitsMask(0, 1, 2);
        }
        else
        if (std::unique_ptr<MACCSFileMetadata> metaMACCS = maccsReader->ReadMetadata(GetParameterAsString("xml"))) {
            std::string maskFileCloud("");
            std::string maskFileWaterSnow("");
            std::string suffix("");
            std::string cld("_CLD");
            std::string msk("_MSK");

            if (metaMACCS->Header.FixedHeader.Mission.find(LANDSAT) != std::string::npos) {
                // Interpret landsat product
                suffix.empty();
            } else if (metaMACCS->Header.FixedHeader.Mission.find(SENTINEL) != std::string::npos) {
                // Interpret sentinel product
                if(!HasValue("sentinelres"))
                    itkExceptionMacro("In case of SENTINEL-S2, 'sentinelres' parameter with resolution as 10 or 20 meters should be provided");
                int resolution = GetParameterInt("sentinelres");
                switch(resolution)
                {
                case 10:
                    suffix = "_R1";
                    break;
                case 20:
                    suffix = "_R2";
                    break;
                default:
                    itkExceptionMacro("In case of SENTINEL-S2, 'sentinelres' parameter should be 10 or 20");
                }

            } else {
                itkExceptionMacro("Unknown mission: " + metaMACCS->Header.FixedHeader.Mission);
            }
            maskFileCloud = getMACCSMaskFileName(m_DirName, metaMACCS->ProductOrganization.AnnexFiles, cld + suffix);
            if(maskFileCloud.length() == 0)
                itkExceptionMacro("Could not read the filename for cloud mask for " + metaMACCS->Header.FixedHeader.Mission);
            maskFileWaterSnow = getMACCSMaskFileName(m_DirName, metaMACCS->ProductOrganization.AnnexFiles, msk + suffix);
            if(maskFileCloud.length() == 0)
                itkExceptionMacro("Could not read the filename for water and snow mask for " + metaMACCS->Header.FixedHeader.Mission);
            m_ReaderCloud->SetFileName(maskFileCloud);
            m_ReaderWaterSnow->SetFileName(maskFileWaterSnow);
            m_SpotMaskHandlerFunctor->GetFunctor().SetBitsMask(0, 0, 5);
        }
        else
            itkExceptionMacro("No SPOT or MACCS xml");

        m_SpotMaskHandlerFunctor->SetInput1(m_ReaderCloud->GetOutput());
        m_SpotMaskHandlerFunctor->SetInput2(m_ReaderWaterSnow->GetOutput());

        m_SpotMaskHandlerFunctor->UpdateOutputInformation();
        m_SpotMaskHandlerFunctor->GetOutput()->SetNumberOfComponentsPerPixel(3);

        SetParameterOutputImage("out", m_SpotMaskHandlerFunctor->GetOutput());

        return;
    }
#endif

    void DoExecute()
    {
        const std::string &tmp = GetParameterAsString("xml");
        std::vector<char> buf(tmp.begin(), tmp.end());
        m_DirName = std::string(dirname(buf.data()));
        m_DirName += '/';


        m_ReaderCloud = ReaderType::New();
        m_ReaderWaterSnow = ReaderType::New();

        auto spot4Reader = itk::SPOT4MetadataReader::New();
        auto maccsReader = itk::MACCSMetadataReader::New();
        m_MaskExtractor = MaskExtractorFilterType::New();

        if ( std::unique_ptr<SPOT4Metadata> metaSPOT = spot4Reader->ReadMetadata(GetParameterAsString("xml") ))
        {
            m_ReaderCloud->SetFileName(m_DirName + metaSPOT->Files.MaskNua);
            m_ReaderWaterSnow->SetFileName(m_DirName + metaSPOT->Files.MaskDiv);
            m_MaskExtractor->SetBitsMask(0, 1, 2);
        }
        else
        if (std::unique_ptr<MACCSFileMetadata> metaMACCS = maccsReader->ReadMetadata(GetParameterAsString("xml"))) {
            std::string maskFileCloud("");
            std::string maskFileWaterSnow("");
            std::string suffix("");
            std::string cld("_CLD");
            std::string msk("_MSK");

            if (metaMACCS->Header.FixedHeader.Mission.find(LANDSAT) != std::string::npos) {
                // Interpret landsat product
                suffix.empty();
            } else if (metaMACCS->Header.FixedHeader.Mission.find(SENTINEL) != std::string::npos) {
                // Interpret sentinel product
                if(!HasValue("sentinelres"))
                    itkExceptionMacro("In case of SENTINEL-S2, 'sentinelres' parameter with resolution as 10 or 20 meters should be provided");
                int resolution = GetParameterInt("sentinelres");
                switch(resolution)
                {
                case 10:
                    suffix = "_R1";
                    break;
                case 20:
                    suffix = "_R2";
                    break;
                default:
                    itkExceptionMacro("In case of SENTINEL-S2, 'sentinelres' parameter should be 10 or 20");
                }

            } else {
                itkExceptionMacro("Unknown mission: " + metaMACCS->Header.FixedHeader.Mission);
            }
            maskFileCloud = getMACCSMaskFileName(m_DirName, metaMACCS->ProductOrganization.AnnexFiles, cld + suffix);
            if(maskFileCloud.length() == 0)
                itkExceptionMacro("Could not read the filename for cloud mask for " + metaMACCS->Header.FixedHeader.Mission);
            maskFileWaterSnow = getMACCSMaskFileName(m_DirName, metaMACCS->ProductOrganization.AnnexFiles, msk + suffix);
            if(maskFileCloud.length() == 0)
                itkExceptionMacro("Could not read the filename for water and snow mask for " + metaMACCS->Header.FixedHeader.Mission);
            m_ReaderCloud->SetFileName(maskFileCloud);
            m_ReaderWaterSnow->SetFileName(maskFileWaterSnow);
            m_MaskExtractor->SetBitsMask(0, 0, 5);
        }
        else
            itkExceptionMacro("No SPOT or MACCS xml");

        m_MaskExtractor->SetInput(0, m_ReaderCloud->GetOutput());
        m_MaskExtractor->SetInput(1, m_ReaderWaterSnow->GetOutput());

        SetParameterOutputImagePixelType("out", ImagePixelType_int16);
        SetParameterOutputImage("out", m_MaskExtractor->GetOutput());

        return;
    }

    // Return the path to a file for which the name end in the ending
    std::string getMACCSMaskFileName(const std::string& rootFolder, const std::vector<MACCSAnnexInformation>& maskFiles, const std::string& ending) {

        for (const MACCSAnnexInformation& fileInfo : maskFiles) {
            if (fileInfo.File.LogicalName.length() >= ending.length() &&
                    0 == fileInfo.File.LogicalName.compare (fileInfo.File.LogicalName.length() - ending.length(), ending.length(), ending)) {
                return rootFolder + fileInfo.File.FileLocation.substr(0, fileInfo.File.FileLocation.find_last_of('.')) + ".DBL.TIF";
            }
        }
        return "";
    }


    ReaderType::Pointer                 m_ReaderCloud;
    ReaderType::Pointer                 m_ReaderWaterSnow ;
    FunctorFilterType::Pointer      m_SpotMaskHandlerFunctor;
    MaskHandlerFunctorType          m_Functor;
    std::string                         m_DirName;

    MaskExtractorFilterType::Pointer             m_MaskExtractor;




    std::unique_ptr<SPOT4Metadata> m_MetaMACCS;
};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::MaskHandler)



