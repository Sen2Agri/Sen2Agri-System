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
 
#ifndef METADATAHELPER_H
#define METADATAHELPER_H

#include <string>
#include <vector>
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageList.h"
#include "otbImageFileReader.h"
#include "otbStreamingResampleImageFilter.h"
#include "otbImageListToVectorImageFilter.h"
#include "ResamplingBandExtractor.h"

#define UNUSED(expr) do { (void)(expr); } while (0)

#define LANDSAT_MISSION_STR    "LANDSAT"
#define SENTINEL_MISSION_STR   "SENTINEL"
#define SPOT_MISSION_STR       "SPOT"
#define SPOT4_MISSION_STR      "SPOT4"
#define SPOT5_MISSION_STR      "SPOT5"

typedef struct {
    double zenith;
    double azimuth;
} MeanAngles_Type;

struct MetadataHelperAngleList
{
    std::string ColumnUnit;
    std::string ColumnStep;
    std::string RowUnit;
    std::string RowStep;
    std::vector<std::vector<double> > Values;
};

struct MetadataHelperAngles
{
    MetadataHelperAngleList Zenith;
    MetadataHelperAngleList Azimuth;
};

struct MetadataHelperViewingAnglesGrid
{
    std::string BandId;
    std::string DetectorId;
    MetadataHelperAngles Angles;
};

typedef enum {MSK_CLOUD=1, MSK_SNOW=2, MSK_WATER=4, MSK_SAT=8, MSK_VALID=16, ALL=0x1F} MasksFlagType;

template <typename PixelType, typename MasksPixelType=short>
class MetadataHelper
{
public:

    typedef short                                           ShortPixelType;
    typedef otb::Image<ShortPixelType, 2>                   SingleBandShortImageType;
    typedef otb::VectorImage<ShortPixelType, 2>             ShortVectorImageType;
    typedef otb::ImageFileReader<ShortVectorImageType>      ShortImageReaderType;
    typedef otb::ImageList<SingleBandShortImageType>        ShortImageListType;

    typedef float                                           FloatPixelType;
    typedef otb::Image<FloatPixelType, 2>                   SingleBandFloatImageType;
    typedef otb::VectorImage<FloatPixelType, 2>             FloatVectorImageType;
    typedef otb::ImageFileReader<FloatVectorImageType>      FloatImageReaderType;
    typedef otb::ImageList<SingleBandFloatImageType>        FloatImageListType;

    //typedef ShortPixelType                                  PixelType;
    typedef otb::Image<MasksPixelType, 2>                   SingleBandMasksImageType;
    typedef otb::Image<PixelType, 2>                        SingleBandImageType;
    typedef otb::VectorImage<PixelType, 2>                  VectorImageType;
    typedef otb::ImageFileReader<VectorImageType>           ImageReaderType;
    typedef otb::ImageList<SingleBandImageType>             ImageListType;

    typedef otb::StreamingResampleImageFilter<VectorImageType, VectorImageType, double>              ResampleVectorImageFilterType;
    typedef otb::ImageListToVectorImageFilter<otb::ImageList<SingleBandImageType>, VectorImageType>  ListConcatenerFilterType;


public:
    MetadataHelper();
    virtual ~MetadataHelper();

    bool LoadMetadataFile(const std::string& file);

    // GENETAL FIELDS API
    virtual std::string GetMissionName() { return m_Mission; }
    virtual std::string GetMissionShortName() { return m_MissionShortName; }
    virtual std::string GetInstrumentName() { return m_Instrument; }

    virtual typename VectorImageType::Pointer GetImage(const std::vector<std::string> &bandNames, int outRes = -1) = 0;
    virtual typename VectorImageType::Pointer GetImage(const std::vector<std::string> &bandNames,
                                                   std::vector<int> *pRetRelBandIdxs, int outRes = -1) = 0;
    virtual typename ImageListType::Pointer GetImageList(const std::vector<std::string> &bandNames,
                                                     typename ImageListType::Pointer outImgList,
                                                     int outRes = -1) = 0;

    // returns the bands for the given resolution. If no such resolution supported, all bands are returned
    virtual std::vector<std::string> GetBandNamesForResolution(int res) = 0;

    /**
     * @brief GetBandNamesForResolution - Return all bands that are in the definition of the product, including the ones that are not
     * physically there (for example, for S2 are returned also the ones of 60m. This is used for
     * situations when exist external files containing all the bands (ex. LAI RSR bands that describe responses
     * for all S2 bands, including the 60m ones)
     * For Bands that exist physically in the product see GetPhysicallyBandNames, see
     * @return the list of all bands
     */
    virtual std::vector<std::string> GetAllBandNames() = 0;
    virtual std::vector<std::string> GetPhysicalBandNames() = 0;
    virtual int GetResolutionForBand(const std::string &bandName) = 0;

    virtual std::string GetRedBandName() { return m_nRedBandName; }
    virtual std::string GetBlueBandName() { return m_nBlueBandName; }
    virtual std::string GetGreenBandName() { return m_nGreenBandName; }
    virtual std::string GetNirBandName() { return m_nNirBandName; }
    virtual std::string GetNarrowNirBandName() { return m_nNarrowNirBandName; }
    virtual std::string GetSwirBandName() { return m_nSwirBandName; }
    virtual std::vector<std::string> GetRedEdgeBandNames() { return m_redEdgeBandNames; }

    virtual std::string GetNoDataValue() { return m_strNoDataValue; }

    // MASKS API
    // The following 4 functions are not very useful here as are very specific to each sensor
    // They should be kept only in the derived classes
    virtual typename SingleBandMasksImageType::Pointer GetMasksImage(MasksFlagType nMaskFlags, bool binarizeResult, int resolution = -1) = 0;

    // DATE API
    // returns the acquisition date in the format YYYYMMDD
    virtual std::string GetAcquisitionDate() { return m_AcquisitionDate; }
    // Returns the acquisition datetime formatted as yyyyMMddThhmmss
    virtual std::string GetAcquisitionDateTime() { return m_AcquisitionDateTime; }
    virtual int GetAcquisitionDateAsDoy();

    // AOT API
    // TODO: This should be replaced with an GetAotImage
    virtual std::string GetAotImageFileName(int res)  = 0;
    virtual double GetReflectanceQuantificationValue() {return m_ReflQuantifVal; }
    virtual float GetAotQuantificationValue(int res) = 0;
    virtual float GetAotNoDataValue(int res) = 0;
    virtual int GetAotBandIndex(int res) = 0;

    // ANGLES API
    virtual bool HasGlobalMeanAngles() { return m_bHasGlobalMeanAngles; }
    virtual bool HasBandMeanAngles() { return m_bHasBandMeanAngles; }
    virtual MeanAngles_Type GetSolarMeanAngles() { return m_solarMeanAngles;}
    virtual MeanAngles_Type GetSensorMeanAngles();
    virtual double GetRelativeAzimuthAngle();
    virtual MeanAngles_Type GetSensorMeanAngles(int nBand);
    virtual bool HasDetailedAngles() { return m_bHasDetailedAngles; }
    virtual int GetDetailedAnglesGridSize() { return m_detailedAnglesGridSize; }
    virtual MetadataHelperAngles GetDetailedSolarAngles() { return m_detailedSolarAngles; }
    virtual std::vector<MetadataHelperViewingAnglesGrid> GetDetailedViewingAngles(int /*res*/) { return m_detailedViewingAngles; }
    virtual std::vector<MetadataHelperViewingAnglesGrid> GetAllDetectorsDetailedViewingAngles() { return m_allDetectorsDetailedViewingAngles; }


    // PRODUCT API
    virtual std::vector<int> GetProductResolutions() { return m_vectResolutions; }

    // BANDS HANDLING API
    // the total number of bands that are physically present in the product
    // (for ex. for S2 the 60m resolution bands are not present in the product but
    //  are present in the metadata)
    virtual int GetBandsPresentInPrdTotalNo() { return m_nTotalBandsNo; }

    // Extract the RGB band names
    virtual bool GetTrueColourBandNames(std::string &redIdx, std::string &greenIdx, std::string &blueIdx);

protected:
    virtual bool DoLoadMetadata(const std::string &file) = 0;
    void Reset();
    std::string buildFullPath(const std::string& fileName);

    typename MetadataHelper<PixelType, MasksPixelType>::ImageReaderType::Pointer CreateReader(const std::string &imgPath) {
       typename MetadataHelper<PixelType, MasksPixelType>::ImageReaderType::Pointer reader =
               MetadataHelper<PixelType, MasksPixelType>::ImageReaderType::New();
       reader->SetFileName(imgPath);
       reader->UpdateOutputInformation();
       this->m_readers.push_back(reader);
       return reader;
   }

   typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer CreateImageList() {
       typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer imgList = MetadataHelper<PixelType, MasksPixelType>::ImageListType::New();
       this->m_ImageListVect.push_back(imgList);
       return imgList;
   }

   typename MetadataHelper<PixelType, MasksPixelType>::ListConcatenerFilterType::Pointer CreateConcatenner() {
       typename MetadataHelper<PixelType, MasksPixelType>::ListConcatenerFilterType::Pointer concat =
                MetadataHelper<PixelType, MasksPixelType>::ListConcatenerFilterType::New();
       this->m_Concateners.push_back(concat);
       return concat;
   }



protected:
    std::string m_Mission;
    // this is for ex. SPOT, LANDSAT, SENTINEL etc.
    std::string m_MissionShortName;
    std::string m_Instrument;

    std::string m_AcquisitionDate;
    // The datetime formatted as yyyyMMddThhmmss
    std::string m_AcquisitionDateTime;

    std::string m_strNoDataValue;
    double m_ReflQuantifVal;

    std::string m_nRedBandName;
    std::string m_nBlueBandName;
    std::string m_nGreenBandName;
    std::string m_nNirBandName;
    std::string m_nNarrowNirBandName;
    std::string m_nSwirBandName;

    std::vector<std::string> m_redEdgeBandNames;

    int m_nTotalBandsNo;

    MeanAngles_Type m_solarMeanAngles;
    std::vector<MeanAngles_Type> m_sensorBandsMeanAngles;
    bool m_bHasGlobalMeanAngles;
    bool m_bHasBandMeanAngles;

    bool m_bHasDetailedAngles;
    int m_detailedAnglesGridSize;
    MetadataHelperAngles m_detailedSolarAngles;
    std::vector<MetadataHelperViewingAnglesGrid> m_detailedViewingAngles;
    std::vector<MetadataHelperViewingAnglesGrid> m_allDetectorsDetailedViewingAngles;

protected:

    typedef struct AotInfos
    {
        AotInfos() {
            m_fAotQuantificationValue = 0.0;
            m_fAotNoDataVal = 0;
            m_nAotBandIndex = -1;
            isInitialized = false;
        }
        float m_fAotQuantificationValue;
        float m_fAotNoDataVal;
        int m_nAotBandIndex;
        bool isInitialized;
    } AotInfos;

    std::string m_inputMetadataFileName;
    std::string m_DirName;
    std::vector<int> m_vectResolutions;

    ResamplingBandExtractor<MasksPixelType> m_maskFlagsBandsExtractor;
    ImageResampler<typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType,
                    typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType>  m_ImageResampler;
    ResamplingBandExtractor<PixelType>  m_bandsExtractor;

private :
    std::vector<typename MetadataHelper<PixelType, MasksPixelType>::ImageReaderType::Pointer> m_readers;
    std::vector<typename ImageListType::Pointer> m_ImageListVect;
    std::vector<typename ListConcatenerFilterType::Pointer>   m_Concateners;

};

#include "../src/MetadataHelper.cpp"

#endif // METADATAHELPER_H
