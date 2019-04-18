/*=========================================================================
  *
  * Program:      Sen2agri-Processors
  * Language:     C++
  * Copyright:    2015-2016, CS Romania, office@c-s.ro
  * See COPYRIGHT file for details.
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.

 =========================================================================*/
 
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

        AddParameter(ParameterType_Int, "isfinal", "Specifies if it is the final splitter from the composition.");
        SetDefaultParameterInt("isfinal", 0);
        MandatoryOff("isfinal");

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
        bool bIsFinalProduct = (GetParameterInt("isfinal") != 0);
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
        const std::vector<int> &bandsPresenceVect = bandsMappingCfg.GetMasterBandsPresence(resolution, nExtractedBandsNo);

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
        unsigned int nTotalBandsNo = (4*nReflsBandsNo);
        unsigned int nTotalBandsWithAddedBlueBandNo = (4*(nReflsBandsNo + 1));
        bool bHasPrevL3ABlueBand = (m_L3AIn->GetNumberOfComponentsPerPixel() == nTotalBandsWithAddedBlueBandNo);
        // check if we have the expected number of bands
        if((m_L3AIn->GetNumberOfComponentsPerPixel() != nTotalBandsWithAddedBlueBandNo) &&
           (m_L3AIn->GetNumberOfComponentsPerPixel() != nTotalBandsNo))

        {
            itkExceptionMacro("Wrong number of bands ! " + m_L3AIn->GetNumberOfComponentsPerPixel());
        }

        m_ImgSplit = VectorImageToImageListType::New();
        m_ImgSplit->SetInput(m_L3AIn);
        m_ImgSplit->UpdateOutputInformation();

        int redBandNo = -1;
        int greenBandNo = -1;
        int blueBandNo = -1;

        int cnt = 0;
        AddExtractedBandToList(bandsPresenceVect, bHasPrevL3ABlueBand, bIsFinalProduct, m_WeightList, cnt);
        AddExtractedBandToList(bandsPresenceVect, bHasPrevL3ABlueBand, bIsFinalProduct, m_DatesList, cnt);

        // first extract the RGB bands before incrementing cnt during reflectance extraction
        GetTrueColorBandsIndexesInRaster(bandsMappingCfg, bandsPresenceVect, resolution, cnt, redBandNo, greenBandNo, blueBandNo);

        // now extract the reflectance bands
        AddExtractedBandToList(bandsPresenceVect, bHasPrevL3ABlueBand, bIsFinalProduct, m_ReflectancesList, cnt);
        AddExtractedBandToList(bandsPresenceVect, bHasPrevL3ABlueBand, bIsFinalProduct, m_FlagsList, cnt);

        if((redBandNo != -1) && (greenBandNo != -1) && (blueBandNo != -1)) {
            m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(redBandNo));
            m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(greenBandNo));
            m_RGBOutList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(blueBandNo));
        }

        m_WeightsConcat = ImageListToVectorImageFilterType::New();
        m_WeightsConcat->SetInput(m_WeightList);
        m_WeightsConcat->UpdateOutputInformation();
        SetParameterOutputImagePixelType("outweights", ImagePixelType_int16);
        SetParameterOutputImage("outweights", m_WeightsConcat->GetOutput());

        m_DatesConcat = ImageListToVectorImageFilterType::New();
        m_DatesConcat->SetInput(m_DatesList);
        m_DatesConcat->UpdateOutputInformation();
        SetParameterOutputImagePixelType("outdates", ImagePixelType_int16);
        SetParameterOutputImage("outdates", m_DatesConcat->GetOutput());

        m_ReflsConcat = ImageListToVectorImageFilterType::New();
        m_ReflsConcat->SetInput(m_ReflectancesList);
        m_ReflsConcat->UpdateOutputInformation();
        SetParameterOutputImagePixelType("outrefls", ImagePixelType_int16);
        SetParameterOutputImage("outrefls", m_ReflsConcat->GetOutput());

        m_FlagsConcat = ImageListToVectorImageFilterType::New();
        m_FlagsConcat->SetInput(m_FlagsList);
        m_FlagsConcat->UpdateOutputInformation();
        SetParameterOutputImagePixelType("outflags", ImagePixelType_uint8);
        SetParameterOutputImage("outflags", m_FlagsConcat->GetOutput());

        if(HasValue("outrgb")) {
            if(m_RGBOutList->Size() > 0) {
                m_RGBConcat = ImageListToVectorImageFilterType::New();
                m_RGBConcat->SetInput(m_RGBOutList);
                SetParameterOutputImagePixelType("outrgb", ImagePixelType_int16);
                SetParameterOutputImage("outrgb", m_RGBConcat->GetOutput());
            }
            else {
                otbAppLogINFO( << "Could not get the indexes for RGB bands. The 'outrgb' file '" << GetParameterString("outrgb") << "' will not be created ");
                DisableParameter("outrgb");
            }
        }
        return;
    }

    void AddExtractedBandToList(const std::vector<int> &bandsPresenceVect, bool bHasPrevL3ABlueBand, bool bIsFinalProduct,
                                ImgListType::Pointer imgList, int &cnt)
    {
        for(unsigned int i = 0; i < bandsPresenceVect.size(); i++) {
            if(bandsPresenceVect[i] != -1) {
                imgList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(cnt++));
            }
        }
        // if it is not the last product and we have an additional blue band in the reflectance raster
        // then add this additional blue band
        // NOTE: The additional blue band is added only for 20m resolution in case of Sentinel2 when
        if(bHasPrevL3ABlueBand) {
            if(!bIsFinalProduct) {
                imgList->PushBack(m_ImgSplit->GetOutput()->GetNthElement(cnt));
            }
            // move to the next band (either if is final or not)
            cnt++;
        }
    }

    void GetTrueColorBandsIndexesInRaster(BandsMappingConfig &bandsMappingCfg, const std::vector<int> &bandsPresenceVect,
                                          int resolution, int cnt,
                                          int &redBandNo, int &greenBandNo, int &blueBandNo)
    {
        int redIdx, greenIdx, blueIdx;
        const std::string inXml = GetParameterAsString("xml");
        otbAppLogINFO( << "InXML: " << inXml << std::endl );

        auto factory = MetadataHelperFactory::New();
        auto pHelper = factory->GetMetadataHelper<short>(inXml);
        bool bHasTrueColorBandIndexes = GetTrueColorBandIndexes(bandsMappingCfg, pHelper, resolution, redIdx, greenIdx, blueIdx);

        for(unsigned int i = 0; i < bandsPresenceVect.size(); i++) {
            if(bandsPresenceVect[i] != -1) {
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
    }

    bool GetTrueColorBandIndexes(BandsMappingConfig &bandsMappingCfg, const std::unique_ptr<MetadataHelper<short>> &pHelper, int resolution,
                                 int &redIdx, int &greenIdx, int &blueIdx) {
        std::string curMissionName = pHelper->GetMissionName();
        redIdx = greenIdx = blueIdx = -1;

        std::string redBandName , greenBandName, blueBandName;
        bool bHasTrueColors = pHelper->GetTrueColourBandNames(redBandName, greenBandName, blueBandName);
        if(!bHasTrueColors)
            return false;
        const std::string &masterRedBandName = bandsMappingCfg.GetMasterBandName(curMissionName, resolution, redBandName);
        const std::string &masterGreenBandName = bandsMappingCfg.GetMasterBandName(curMissionName, resolution, greenBandName);
        const std::string &masterBlueBandName = bandsMappingCfg.GetMasterBandName(curMissionName, resolution, blueBandName);
        // if one of them is not present, return
        if(masterRedBandName.size() == 0 ||  masterGreenBandName.size() == 0 || masterBlueBandName.size() == 0) {
            return false;
        }
        // now convert the absolute indexes into relative indexes to our raster type
        redIdx = bandsMappingCfg.GetIndexInMasterPresenceArray(resolution, masterRedBandName);
        greenIdx = bandsMappingCfg.GetIndexInMasterPresenceArray(resolution, masterGreenBandName);
        blueIdx = bandsMappingCfg.GetIndexInMasterPresenceArray(resolution, masterBlueBandName);

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



