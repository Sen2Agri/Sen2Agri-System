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

        /*auto factory = MetadataHelperFactory::New();
        auto pHelper = factory->GetMetadataHelper(inXml);
        std::string missionName = pHelper->GetMissionName();
        */
        m_bandsCfgMappingParser.ParseFile(strBandsMappingFileName);
        BandsMappingConfig bandsMappingCfg = m_bandsCfgMappingParser.GetBandsMappingCfg();
        int nExtractedBandsNo = 0;
        // create an array of bands presences with the same size as the master band size
        // and having the presences of Master band
        std::string missionName = bandsMappingCfg.GetMasterMissionName();
        std::vector<int> bandsPresenceVect = bandsMappingCfg.GetBandsPresence(resolution, missionName, nExtractedBandsNo);

        otbAppLogINFO( << "InXML: " << inXml << std::endl );
        otbAppLogINFO( << "Resolution: " << resolution << std::endl );

        m_WeightsConcat = ImageListToVectorImageFilterType::New();
        m_ReflsConcat = ImageListToVectorImageFilterType::New();
        m_DatesConcat = ImageListToVectorImageFilterType::New();
        m_FlagsConcat = ImageListToVectorImageFilterType::New();

        m_ReflectancesList = ImgListType::New();
        m_WeightList = ImgListType::New();
        m_DatesList = ImgListType::New();
        m_FlagsList = ImgListType::New();

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
        for(int i = 0; i < nReflsBandsNo; i++) {
            if(bandsPresenceVect[cnt] != -1) {
                m_WeightList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(cnt));
            }
            cnt++;
        }
        m_DatesList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(cnt++));
        for(int i = 0; i < nReflsBandsNo; i++) {
            if(bandsPresenceVect[cnt] != -1) {
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

    ImgListType::Pointer m_ReflectancesList;
    ImgListType::Pointer m_WeightList;
    ImgListType::Pointer m_DatesList;
    ImgListType::Pointer m_FlagsList;

    BandsCfgMappingParser m_bandsCfgMappingParser;
};

}
}
OTB_APPLICATION_EXPORT(otb::Wrapper::CompositeSplitter2)



