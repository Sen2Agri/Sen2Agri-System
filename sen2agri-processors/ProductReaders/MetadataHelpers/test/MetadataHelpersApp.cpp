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
#include "otbVectorImage.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbStreamingResampleImageFilter.h"
#include "MetadataHelperFactory.h"
#include "ImageResampler.h"

//Transform
#include "itkScalableAffineTransform.h"
#include "GlobalDefs.h"
#include "itkBinaryFunctorImageFilter.h"

#include <fstream>

namespace otb
{
namespace Wrapper
{
class MetadataHelpersApp : public Application
{
public:
    /** Standard class typedefs. */
    typedef MetadataHelpersApp        Self;
    typedef Application                   Superclass;
    typedef itk::SmartPointer<Self>       Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Standard macro */
    itkNewMacro(Self);

    itkTypeMacro(MetadataHelpersApp, otb::Application);


private:
    MetadataHelpersApp()
    {
    }

    void DoInit() override
    {
        SetName("MetadataHelpersApp");
        SetDescription("Test application for the product readers.");

        // Documentation
        SetDocName("Test application for the product readers");
        SetDocLongDescription("");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Learning);

        AddParameter(ParameterType_String, "xml", "The product metadata file");
        SetParameterDescription("xml","The product metadata file");

        AddParameter(ParameterType_OutputImage, "out", "Out image");
        AddParameter(ParameterType_OutputImage, "outmsk", "Out Masks image");

        AddRAMParameter();

        // Doc example parameter settings
        SetDocExampleParameterValue("xml", "support_image.hdr");
    }

    void DoUpdateParameters() override
    {
    }

    void DoExecute() override
    {
        const std::string &xmlFile = GetParameterAsString("xml");
        std::cout << xmlFile << std::endl;

        auto factory = MetadataHelperFactory::New();
        m_pHelper = factory->GetMetadataHelper<short>(xmlFile);

        PrintProductInformations();

        //std::vector<int> bands = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
//        std::vector<std::string> bands = {"B1", "B2", "B4", "B5"};

        const std::vector<std::string> &bandNames = {m_pHelper->GetGreenBandName(),
                                                m_pHelper->GetRedBandName(),
                                                m_pHelper->GetNirBandName(),
                                                m_pHelper->GetSwirBandName()};

        SetParameterOutputImagePixelType("out", ImagePixelType_int16);
        MetadataHelper<short>::VectorImageType::Pointer img = m_pHelper->GetImage(bandNames);
        SetParameterOutputImage("out", (MetadataHelper<short>::VectorImageType*) img);

        SetParameterOutputImagePixelType("outmsk", ImagePixelType_uint8);
        //MasksFlagType flgs = ALL;
        MasksFlagType flgs = MSK_CLOUD;
        MetadataHelper<short>::SingleBandImageType::Pointer mskImg = m_pHelper->GetMasksImage(flgs, false);
        SetParameterOutputImage("outmsk", (MetadataHelper<short>::SingleBandImageType*) mskImg);

    }

    void PrintProductInformations() {
        const std::string &missionName = m_pHelper->GetMissionName();
        const std::string &missionShortName = m_pHelper->GetMissionShortName();
        const std::string &instrName = m_pHelper->GetInstrumentName();

//        GetImage(const std::vector<std::string> &bandNames, int outRes = -1)
//        GetImage(const std::vector<std::string> &bandNames, std::vector<int> *pRetRelBandIdxs, int outRes = -1)
//        GetImageList(const std::vector<std::string> &bandNames, typename ImageListType::Pointer outImgList, int outRes = -1)

        const std::vector<std::string> &resBandNames10M = m_pHelper->GetBandNamesForResolution(10);
        const std::vector<std::string> &resBandNames20M = m_pHelper->GetBandNamesForResolution(20);
        const std::vector<std::string> &resBandNames30M = m_pHelper->GetBandNamesForResolution(30);
        const std::vector<std::string> &resBandNames60M = m_pHelper->GetBandNamesForResolution(60);

        const std::vector<std::string> &allBandNames = m_pHelper->GetAllBandNames();
        const std::vector<std::string> &physicalBandNames = m_pHelper->GetPhysicalBandNames();
        int bandResB1 = m_pHelper->GetResolutionForBand("B1");
        int bandResB2 = m_pHelper->GetResolutionForBand("B2");
        int bandResB4 = m_pHelper->GetResolutionForBand("B4");
        int bandResB8 = m_pHelper->GetResolutionForBand("B8");
        int bandResB8A = m_pHelper->GetResolutionForBand("B8A");
        int bandResB5 = m_pHelper->GetResolutionForBand("B5");
        int bandResB10 = m_pHelper->GetResolutionForBand("B10");
        int bandResB11 = m_pHelper->GetResolutionForBand("B11");
        int bandResB12 = m_pHelper->GetResolutionForBand("B12");

        const std::string &redBandName = m_pHelper->GetRedBandName();
        const std::string &blueBandName = m_pHelper->GetBlueBandName();
        const std::string &greenBandName = m_pHelper->GetGreenBandName();
        const std::string &nirBandName = m_pHelper->GetNirBandName();
        const std::string &narrowNirBandName = m_pHelper->GetNarrowNirBandName();
        const std::string &swirBandName = m_pHelper->GetSwirBandName();
        const std::vector<std::string> &rededgeBandNames = m_pHelper->GetRedEdgeBandNames();

        const std::string &noDataVal = m_pHelper->GetNoDataValue();

        //GetMasksImage(MasksFlagType nMaskFlags, bool binarizeResult, int resolution = -1);

        const std::string &acqDate = m_pHelper->GetAcquisitionDate();
        int acqDoy = m_pHelper->GetAcquisitionDateAsDoy();

        const std::string &aotFile10M = m_pHelper->GetAotImageFileName(10);
        const std::string &aotFile20M = m_pHelper->GetAotImageFileName(20);
        const std::string &aotFile30M = m_pHelper->GetAotImageFileName(30);
        double reflQuantifVal = m_pHelper->GetReflectanceQuantificationValue();
        float aotQuantifVal10M = m_pHelper->GetAotQuantificationValue(10);
        float aotQuantifVal20M = m_pHelper->GetAotQuantificationValue(20);
        float aotQuantifVal30M = m_pHelper->GetAotQuantificationValue(30);
        float aotNoDataVal = m_pHelper->GetAotNoDataValue(10);
        int aotBandIdx10M = m_pHelper->GetAotBandIndex(10);
        int aotBandIdx20M = m_pHelper->GetAotBandIndex(20);
        int aotBandIdx30M = m_pHelper->GetAotBandIndex(30);

        bool bHasGlobalMeanAngles = m_pHelper->HasGlobalMeanAngles();
        bool bHasBandMeanAngles = m_pHelper->HasBandMeanAngles();
        MeanAngles_Type solarMeanAngles = m_pHelper->GetSolarMeanAngles();
        MeanAngles_Type sensorMeanAngles = m_pHelper->GetSensorMeanAngles();
        double relAzimuthAngle = m_pHelper->GetRelativeAzimuthAngle();
        MeanAngles_Type bandSensorMeanAngles = m_pHelper->GetSensorMeanAngles(2);
        bool bHasDetailedAngles = m_pHelper->HasDetailedAngles();
        int detAnglGridSize = m_pHelper->GetDetailedAnglesGridSize();
        MetadataHelperAngles detailedSolarAngles = m_pHelper->GetDetailedSolarAngles();
        std::vector<MetadataHelperViewingAnglesGrid> detailedViewAngles10M = m_pHelper->GetDetailedViewingAngles(10);
//        std::vector<MetadataHelperViewingAnglesGrid> detailedViewAngles20M = m_pHelper->GetDetailedViewingAngles(20);
//        std::vector<MetadataHelperViewingAnglesGrid> detailedViewAngles30M = m_pHelper->GetDetailedViewingAngles(30);
//        std::vector<MetadataHelperViewingAnglesGrid> detailedViewAnglesAllDet = m_pHelper->GetAllDetectorsDetailedViewingAngles();
        std::vector<int> prdResolutions = m_pHelper->GetProductResolutions();
        int nTotalBandsNo = m_pHelper->GetBandsPresentInPrdTotalNo();

        std::string redColorBandName, greenColorBandName, blueColorBandName;
        m_pHelper->GetTrueColourBandNames(redColorBandName, greenColorBandName, blueColorBandName);

        std::cout << "missionName: " <<  missionName << std::endl;
        std::cout << "missionShortName: " <<  missionShortName << std::endl;
        std::cout << "instrName: " <<  instrName << std::endl;

        PrintStringVector("resBandNames10M", resBandNames10M);
        PrintStringVector("resBandNames20M", resBandNames20M);
        PrintStringVector("resBandNames30M", resBandNames30M);
        PrintStringVector("resBandNames60M", resBandNames60M);

        PrintStringVector("allBandNames", allBandNames);

        PrintStringVector("physicalBandNames", allBandNames);

        std::cout << "bandResB1: " << bandResB1  << std::endl;
        std::cout << "bandResB2: " << bandResB2  << std::endl;
        std::cout << "bandResB4: " << bandResB4  << std::endl;
        std::cout << "bandResB8: " << bandResB8  << std::endl;
        std::cout << "bandResB8A: " << bandResB8A  << std::endl;
        std::cout << "bandResB5: " <<  bandResB5 << std::endl;
        std::cout << "bandResB10: " << bandResB10  << std::endl;
        std::cout << "bandResB11: " <<  bandResB11 << std::endl;
        std::cout << "bandResB12: " << bandResB12  << std::endl;

        std::cout << "redBandName: " <<  redBandName << std::endl;
        std::cout << "blueBandName: " <<  blueBandName << std::endl;
        std::cout << "greenBandName: " << greenBandName  << std::endl;
        std::cout << "nirBandName: " << nirBandName  << std::endl;
        std::cout << "narrowNirBandName: " <<  narrowNirBandName << std::endl;
        std::cout << "swirBandName: " << swirBandName  << std::endl;

        PrintStringVector("rededgeBandNames", rededgeBandNames);

        std::cout << "noDataVal: " << noDataVal  << std::endl;

        std::cout << "acqDate: " << acqDate  << std::endl;
        std::cout << "acqDoy: " <<  acqDoy << std::endl;
        std::cout << "aotFile10M: " << aotFile10M  << std::endl;
        std::cout << "aotFile20M: " << aotFile20M  << std::endl;
        std::cout << "aotFile30M: " <<  aotFile30M << std::endl;
        std::cout << "reflQuantifVal: " << reflQuantifVal  << std::endl;
        std::cout << "aotQuantifVal10M: " << aotQuantifVal10M  << std::endl;
        std::cout << "aotQuantifVal20M: " <<  aotQuantifVal20M << std::endl;
        std::cout << "aotQuantifVal30M: " << aotQuantifVal30M  << std::endl;
        std::cout << "aotNoDataVal: " << aotNoDataVal  << std::endl;
        std::cout << "aotBandIdx10M: " <<  aotBandIdx10M << std::endl;
        std::cout << "aotBandIdx20M: " << aotBandIdx20M  << std::endl;
        std::cout << "aotBandIdx30M: " << aotBandIdx30M  << std::endl;

        std::cout << "bHasGlobalMeanAngles: " << bHasGlobalMeanAngles  << std::endl;
        std::cout << "bHasBandMeanAngles: " <<  bHasBandMeanAngles << std::endl;

        PrintMeanAngles("solarMeanAngles", solarMeanAngles);
        PrintMeanAngles("sensorMeanAngles", sensorMeanAngles);

        std::cout << "relAzimuthAngle: " <<  relAzimuthAngle << std::endl;

        PrintMeanAngles("bandSensorMeanAngles", bandSensorMeanAngles);

        std::cout << "bHasDetailedAngles: " <<  bHasDetailedAngles << std::endl;
        std::cout << "detAnglGridSize: " <<  detAnglGridSize << std::endl;

        PrintDetailedAngles("detailedSolarAngles", detailedSolarAngles);

        PrintViewDetectorsAngles("detailedViewAnglesAllDet", detailedViewAngles10M);
//        PrintViewDetectorsAngles("detailedViewAnglesAllDet", detailedViewAngles20M);
//        PrintViewDetectorsAngles("detailedViewAnglesAllDet", detailedViewAngles30M);
//        PrintViewDetectorsAngles("detailedViewAnglesAllDet", detailedViewAnglesAllDet);

        PrintPrimitiveVector<int>("prdResolutions", prdResolutions);

        std::cout << "nTotalBandsNo: " << nTotalBandsNo  << std::endl;

        std::cout << "redColorBandName, greenColorBandName, blueColorBandName: " <<  redColorBandName << ", " << greenColorBandName << ", " << blueColorBandName << std::endl;

         std::cout << "################################################"<< std::endl;
    }

    void PrintMeanAngles(const std::string &name, const MeanAngles_Type &angles) {
        std::cout << "Mean Angles for : " << name.c_str() << ": " << std::endl;
        std::cout << "\tZenith: " << angles.zenith << std::endl;
        std::cout << "\tAzimuth: " << angles.azimuth << std::endl;
    }

    void PrintDetailedAngles(const std::string &name, const MetadataHelperAngles &angles, int indent = 0) {
        std::string indentStr;
        for (int i = 0; i< indent; i++) {
            indentStr.append("\t");
        }
        std::cout << indentStr.c_str()  << "Detailed Angles for : " << name.c_str() << ": " << std::endl;
        PrintAnglesList("Zenith", angles.Zenith, indent+1);
        PrintAnglesList("Azimuth", angles.Azimuth, indent+1);
    }

    void PrintAnglesList(const std::string &name, const MetadataHelperAngleList &angles, int indent = 0) {
        std::string indentStr;
        for (int i = 0; i< indent; i++) {
            indentStr.append("\t");
        }
        std::cout << indentStr.c_str() << "Angles list for " << name << ": " << std::endl;
        indentStr.append("\t");
        std::cout << indentStr.c_str() <<"ColumnStep: " << angles.ColumnStep << std::endl;
        std::cout << indentStr.c_str() <<"ColumnUnit: " << angles.ColumnUnit << std::endl;
        std::cout << indentStr.c_str() <<"RowStep: " << angles.RowStep << std::endl;
        std::cout << indentStr.c_str() <<"RowUnit: " << angles.RowUnit << std::endl;
        std::cout << indentStr.c_str() <<"Values : ";
        indentStr.append("\t");
        std::cout << indentStr.c_str() ;
        for (const std::vector<double> &line : angles.Values) {
            std::cout << indentStr.c_str();
            for(double val: line) {
                std::cout << val << ", ";
            }
            std::cout << std::endl;
        }
    }

    void PrintViewDetectorsAngles(const std::string &name, const std::vector<MetadataHelperViewingAnglesGrid>  &angles) {
        std::cout << "ViewDetectorsAngles Angles for : " << name.c_str() << ": " << std::endl;
        for(const MetadataHelperViewingAnglesGrid& angle: angles) {
            std::cout << "\tBand ID: " << angle.BandId << ", Detector ID: " << angle.DetectorId << std::endl;
            PrintDetailedAngles("Detailed angles", angle.Angles, 1);
        }
    }

    void PrintStringVector(const std::string &name, const std::vector<std::string> &vect) {
        int i = 0;
        std::cout << "Printing vector: " << name.c_str() << ": " << std::endl;
        for(const std::string &item: vect) {
            std::cout << "\t" << i << ": " << item.c_str() << std::endl;
            i++;
        }
    }

    template <typename T>
    void PrintPrimitiveVector(const std::string &name, const std::vector<T> &vect) {
        int i = 0;
        std::cout << "Printing vector: " << name.c_str();
        std::cout << ": " << std::endl;
        for(const T &item: vect) {
            std::cout << "\t" << i << ": " << item << std::endl;
            i++;
        }
    }

    //MetadataHelper::VectorImageType::Pointer m_img;
    std::unique_ptr<MetadataHelper<short>> m_pHelper;


};

} // end of namespace Wrapper
} // end of namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::MetadataHelpersApp)
