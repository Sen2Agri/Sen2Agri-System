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
#include "BandsCfgMappingParser.h"

namespace otb
{

namespace Wrapper
{

class CompositeSplitter2 : public Application
{
public:    
    typedef CompositeSplitter2 Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)

    itkTypeMacro(CompositeSplitter2, otb::Application)


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

    void DoInit()
    {
        SetName("CompositeSplitter2");
        SetDescription("Extracts Cloud, Water and Snow masks from _div.tif and _sat.tif SPOT files ");

        SetDocName("CompositeSplitter2");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");        
        AddDocTag(Tags::Vector);
        AddParameter(ParameterType_InputImage, "in", "L3A product");
        AddParameter(ParameterType_String, "xml", "General xml input file for L2A");
        AddParameter(ParameterType_InputFilename, "bmap", "Master to secondary bands mapping");
        AddParameter(ParameterType_OutputImage, "outweights", "Out file weights");
        AddParameter(ParameterType_OutputImage, "outdates", "Out file weights");
        AddParameter(ParameterType_OutputImage, "outrefls", "Out file weights");
        AddParameter(ParameterType_OutputImage, "outflags", "Out file weights");

        AddParameter(ParameterType_Int, "res", "Input current L3A XML");
        SetDefaultParameterInt("res", -1);
        MandatoryOff("res");

        AddParameter(ParameterType_OutputImage, "outrgb", "Output rgb filename");
        MandatoryOff("outrgb");
    }

    void DoUpdateParameters()
    {
      // Nothing to do.
    }

    void DoExecute()
    {
        int resolution = GetParameterInt("res");
        const std::string inXml = GetParameterAsString("xml");
        std::string strBandsMappingFileName = GetParameterAsString("bmap");
        m_L3AIn = GetParameterInt16VectorImage("in");
        m_L3AIn->UpdateOutputInformation();
        if(resolution <= 0) {
            resolution = m_L3AIn->GetSpacing()[0];
        }

        m_bandsCfgMappingParser.ParseFile(strBandsMappingFileName);
        BandsMappingConfig bandsMappingCfg = m_bandsCfgMappingParser.GetBandsMappingCfg();
        int nExtractedBandsNo = 0;
        // create an array of bands presences with the same size as the master band size
        // and having the presences of Master band
        std::string masterMissionName = bandsMappingCfg.GetMasterMissionName();
        std::vector<int> bandsPresenceVect = bandsMappingCfg.GetBandsPresence(resolution, masterMissionName, nExtractedBandsNo);

        otbAppLogINFO( << "InXML: " << inXml << std::endl );
        otbAppLogINFO( << "Resolution: " << resolution << std::endl );

        m_WeightsConcat = ImageListToVectorImageFilterType::New();
        m_ReflsConcat = ImageListToVectorImageFilterType::New();
        m_DatesConcat = ImageListToVectorImageFilterType::New();
        m_FlagsConcat = ImageListToVectorImageFilterType::New();
        m_RGBConcat   = ImageListToVectorImageFilterType::New();

        m_ReflectancesList = ImgListType::New();
        m_WeightList = ImgListType::New();
        m_DatesList = ImgListType::New();
        m_FlagsList = ImgListType::New();
        m_RGBOutList = ImgListType::New();

        int nReflsBandsNo = nExtractedBandsNo;
        unsigned int nTotalBandsNo = (2*nReflsBandsNo+2);
        if(m_L3AIn->GetNumberOfComponentsPerPixel() != nTotalBandsNo)
        {
            itkExceptionMacro("Wrong number of bands ! " + m_L3AIn->GetNumberOfComponentsPerPixel());
        }

        m_ImgSplit = VectorImageToImageListType::New();
        m_ImgSplit->SetInput(m_L3AIn);
        m_ImgSplit->UpdateOutputInformation();

        int cnt = 0;
        for(unsigned int i = 0; i < bandsPresenceVect.size(); i++) {
            if(bandsPresenceVect[cnt] != -1) {
                m_WeightList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(cnt));
                cnt++;
            }
        }
        m_DatesList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(cnt++));
        int redIdx, greenIdx, blueIdx;
        bool bHasTrueColorBandIndexes = GetTrueColorBandIndexes(inXml, bandsMappingCfg, resolution, redIdx, greenIdx, blueIdx);
        int redBandNo = -1;
        int greenBandNo = -1;
        int blueBandNo = -1;
        for(unsigned int i = 0; i < bandsPresenceVect.size(); i++) {
            if(bandsPresenceVect[i] != -1) {
                m_ReflectancesList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(cnt));
                if(bHasTrueColorBandIndexes) {
                    if(bandsPresenceVect[i] == redIdx) {
                        redBandNo = cnt;
                    }
                    if(bandsPresenceVect[i] == greenIdx) {
                        greenBandNo = cnt;
                    }
                    if(bandsPresenceVect[i] == blueIdx) {
                        blueBandNo = cnt;
                    }
                }
                cnt++;
            }
        }
        m_FlagsList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(cnt++));

        if((redBandNo != -1) && (greenBandNo != -1) && (blueBandNo != -1)) {
            m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(redBandNo));
            m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(greenBandNo));
            m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(blueBandNo));
        }

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


        if(HasValue("outrgb") && m_RGBOutList->Size() > 0) {
            m_RGBConcat = ImageListToVectorImageFilterType::New();
            m_RGBConcat->SetInput(m_RGBOutList);
            SetParameterOutputImagePixelType("outrgb", ImagePixelType_int16);
            SetParameterOutputImage("outrgb", m_RGBConcat->GetOutput());
        }

        return;
    }

    bool GetTrueColorBandIndexes(const std::string &inXml, BandsMappingConfig &bandsMappingCfg, int resolution,
                                 int &redIdx, int &greenIdx, int &blueIdx) {
        auto factory = MetadataHelperFactory::New();
        auto pHelper = factory->GetMetadataHelper(inXml);
        std::string curMissionName = pHelper->GetMissionName();
        redIdx = greenIdx = blueIdx = -1;

        int nRedIdx, nGreenIdx, nBlueIdx;
        bool bHasTrueColors = pHelper->GetTrueColourBandIndexes(nRedIdx, nGreenIdx, nBlueIdx);
        if(!bHasTrueColors)
            return false;
        int nMasterRedIdx = bandsMappingCfg.GetMasterBandIndex(curMissionName, resolution, nRedIdx);
        int nMasterGreenIdx = bandsMappingCfg.GetMasterBandIndex(curMissionName, resolution, nGreenIdx);
        int nMasterBlueIdx = bandsMappingCfg.GetMasterBandIndex(curMissionName, resolution, nBlueIdx);
        // if one of them is not present, return
        if(nMasterRedIdx <= 0 ||  nMasterGreenIdx <= 0 || nMasterBlueIdx <= 0) {
            return false;
        }
        // now convert the absolute indexes into relative indexes to our raster type
        std::string masterMissionName = bandsMappingCfg.GetMasterMissionName();
        redIdx = bandsMappingCfg.GetIndexInPresenceArray(resolution, masterMissionName, nMasterRedIdx);
        greenIdx = bandsMappingCfg.GetIndexInPresenceArray(resolution, masterMissionName, nMasterGreenIdx);
        blueIdx = bandsMappingCfg.GetIndexInPresenceArray(resolution, masterMissionName, nMasterBlueIdx);

        // these indexes are 0 based
        if(redIdx < 0 ||  greenIdx < 0 || blueIdx < 0) {
            return false;
        }
        return true;
    }

    InputImageType::Pointer             m_L3AIn;

    VectorImageToImageListType::Pointer       m_ImgSplit;
    ImageListToVectorImageFilterType::Pointer m_WeightsConcat;
    ImageListToVectorImageFilterType::Pointer m_ReflsConcat;
    ImageListToVectorImageFilterType::Pointer m_DatesConcat;
    ImageListToVectorImageFilterType::Pointer m_FlagsConcat;
    ImageListToVectorImageFilterType::Pointer m_RGBConcat;

    ImgListType::Pointer m_ReflectancesList;
    ImgListType::Pointer m_WeightList;
    ImgListType::Pointer m_DatesList;
    ImgListType::Pointer m_FlagsList;
    ImgListType::Pointer m_RGBOutList;

    BandsCfgMappingParser m_bandsCfgMappingParser;
};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::CompositeSplitter2)



