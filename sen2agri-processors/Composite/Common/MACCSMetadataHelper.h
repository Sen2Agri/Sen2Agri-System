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
 
#ifndef S2METADATAHELPER_H
#define S2METADATAHELPER_H

#include "MetadataHelper.h"
#include <vector>

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"
#include "ResamplingBandExtractor2.h"
#include "itkNaryFunctorImageFilter.h"

typedef itk::MACCSMetadataReader                                   MACCSMetadataReaderType;


class MACCSMetadataHelper : public MetadataHelper
{
    template< class TInput, class TOutput>
    class NaryMaskHandlerFunctor
    {
    public:
        NaryMaskHandlerFunctor(){}
        void Initialize(MasksFlagType nMaskFlags, bool binarizeResult) { m_MaskFlags = nMaskFlags; m_bBinarizeResult = binarizeResult;}
        NaryMaskHandlerFunctor& operator =(const NaryMaskHandlerFunctor& copy) {
            m_MaskFlags=copy.m_MaskFlags;
            m_bBinarizeResult = copy.m_bBinarizeResult;
            return *this;
        }
        bool operator!=( const NaryMaskHandlerFunctor & a) const { return (this->m_MaskFlags != a.m_MaskFlags) || (this->m_bBinarizeResult != a.m_bBinarizeResult) ;}
        bool operator==( const NaryMaskHandlerFunctor & a ) const { return !(*this != a); }

        // The expected order in the array would be : Cloud, Saturation, Valid, (Water or Snow)
        TOutput operator()( const std::vector< TInput > & B) {
            // The order is MSK_CLOUD, MSK_SAT, MSK_VALID, (MSK_SNOW/MSK_WATTER)
            switch (B.size())
            {
            case 1:
                if(((m_MaskFlags & MSK_VALID) != 0) && ((B[0] & 0x01) != 0)) return IMG_FLG_NO_DATA;
                if(((m_MaskFlags & MSK_CLOUD) != 0) && (B[0] != 0)) return IMG_FLG_CLOUD;
                if(((m_MaskFlags & MSK_SAT) != 0) && (B[0] != 0)) return IMG_FLG_SATURATION;
                if(((m_MaskFlags & MSK_WATER) != 0) && ((B[0] & 0x01) != 0)) return IMG_FLG_WATER;
                if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[0] & 0x20) != 0)) return IMG_FLG_SNOW;
                break;
            case 2:
                if((m_MaskFlags & MSK_CLOUD) != 0) {
                    if((m_MaskFlags & MSK_SAT) != 0) {
                        if(B[0] != 0) return IMG_FLG_CLOUD;
                        if(B[1] != 0) return IMG_FLG_SATURATION;
                    } else {
                        if((m_MaskFlags & MSK_VALID) != 0) {
                            // Normally, we should start with the check of the validity as if the validity is no data, then
                            // there is no use to check the others. Also, we could get false cloud even if validity is not good
                            if((B[1] & 0x01) != 0) return IMG_FLG_NO_DATA;
                            if(B[0] != 0) return IMG_FLG_CLOUD;
                        } else {
                            if(B[0] != 0) return IMG_FLG_CLOUD;
                            // we have water or snow
                            if(((m_MaskFlags & MSK_WATER) != 0) && ((B[1] & 0x01) != 0)) return IMG_FLG_WATER;
                            if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[1] & 0x20) != 0)) return IMG_FLG_SNOW;
                        }
                    }
                } else {
                    // we have no cloud mask but we might have one of the others
                    if((m_MaskFlags & MSK_SAT) != 0) {
                        if((m_MaskFlags & MSK_VALID) != 0) {
                            // Normally, we should start with the check of the validity as if the validity is no data, then
                            // there is no use to check the others. Also, we could get false saturation even if validity is not good
                            if((B[1] & 0x01) != 0) return IMG_FLG_NO_DATA;
                            if(B[0] != 0) return IMG_FLG_SATURATION;
                        } else {
                            if(B[0] != 0) return IMG_FLG_SATURATION;
                            // we have water or snow
                            if(((m_MaskFlags & MSK_WATER) != 0) && ((B[1] & 0x01) != 0)) return IMG_FLG_WATER;
                            if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[1] & 0x20) != 0)) return IMG_FLG_SNOW;
                        }
                    } else {
                        // In this case we have certainly MSK_VALID on first position
                        if((B[0] & 0x01) != 0) return IMG_FLG_NO_DATA;
                        // we have water or snow
                        if(((m_MaskFlags & MSK_WATER) != 0) && ((B[1] & 0x01) != 0)) return IMG_FLG_WATER;
                        if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[1] & 0x20) != 0)) return IMG_FLG_SNOW;
                    }
                }
                break;
            case 3:
                // TODO: Normally, we should start with the check of the validity as if the validity is no data, then
                // there is no use to check the others. Also, we could get false cloud even if validity is not good
                if((m_MaskFlags & MSK_CLOUD) != 0) {
                    if(B[0] != 0) return IMG_FLG_CLOUD;

                    if((m_MaskFlags & MSK_SAT) != 0) {
                        if(B[1] != 0) return IMG_FLG_SATURATION;

                        // the third one is one of these
                        if(((m_MaskFlags & MSK_VALID) != 0) && ((B[2] & 0x01) != 0)) return IMG_FLG_NO_DATA;
                        if(((m_MaskFlags & MSK_WATER) != 0) && ((B[2] & 0x01) != 0)) return IMG_FLG_WATER;
                        if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[2] & 0x20) != 0)) return IMG_FLG_SNOW;

                    } else {
                        if((m_MaskFlags & MSK_VALID) != 0) {
                            if((B[1] & 0x01) != 0) return IMG_FLG_NO_DATA;
                            if(((m_MaskFlags & MSK_WATER) != 0) && ((B[2] & 0x01) != 0)) return IMG_FLG_WATER;
                            if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[2] & 0x20) != 0)) return IMG_FLG_SNOW;
                        } else {
                            // we have both water AND snow
                            if(((m_MaskFlags & MSK_WATER) != 0) && ((B[1] & 0x01) != 0)) return IMG_FLG_WATER;
                            if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[2] & 0x20) != 0)) return IMG_FLG_SNOW;
                        }
                    }
                } else {
                    if((m_MaskFlags & MSK_SAT) != 0) {
                        if(B[0] != 0) return IMG_FLG_SATURATION;
                        if((m_MaskFlags & MSK_VALID) != 0) {
                            if((B[1] & 0x01) != 0) return IMG_FLG_NO_DATA;
                            if(((m_MaskFlags & MSK_WATER) != 0) && ((B[2] & 0x01) != 0)) return IMG_FLG_WATER;
                            if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[2] & 0x20) != 0)) return IMG_FLG_SNOW;
                        } else {
                            // we have both water AND snow
                            if(((m_MaskFlags & MSK_WATER) != 0) && ((B[1] & 0x01) != 0)) return IMG_FLG_WATER;
                            if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[2] & 0x20) != 0)) return IMG_FLG_SNOW;
                        }
                    } else {
                        // if we do not have cloud and saturation, we have all others
                        if(((m_MaskFlags & MSK_VALID) != 0) && ((B[0] & 0x01) != 0)) return IMG_FLG_NO_DATA;
                        if(((m_MaskFlags & MSK_WATER) != 0) && ((B[1] & 0x01) != 0)) return IMG_FLG_WATER;
                        if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[2] & 0x20) != 0)) return IMG_FLG_SNOW;

                    }
                    // if we do not have cloud, we have all others
                    if(((m_MaskFlags & MSK_VALID) != 0) && ((B[1] & 0x01) != 0)) return IMG_FLG_NO_DATA;
                    if(((m_MaskFlags & MSK_SAT) != 0) && (B[0] != 0)) return IMG_FLG_SATURATION;
                    if(((m_MaskFlags & MSK_WATER) != 0) && ((B[2] & 0x01) != 0)) return IMG_FLG_WATER;
                    if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[2] & 0x20) != 0)) return IMG_FLG_SNOW;
                }
                break;
            case 4:
                // in this case we have certainly all images
                if(((m_MaskFlags & MSK_VALID) != 0) && ((B[2] & 0x01) != 0)) return IMG_FLG_NO_DATA;
                if(((m_MaskFlags & MSK_CLOUD) != 0) && (B[0] != 0)) return IMG_FLG_CLOUD;
                if(((m_MaskFlags & MSK_SAT) != 0) && (B[1] != 0)) return IMG_FLG_SATURATION;
                if(((m_MaskFlags & MSK_WATER) != 0) && ((B[3] & 0x01) != 0)) return IMG_FLG_WATER;
                if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[3] & 0x20) != 0)) return IMG_FLG_SNOW;
                break;
            }
            return m_bBinarizeResult ? 0 : IMG_FLG_LAND;
        }

    private:
        MasksFlagType m_MaskFlags;
        bool m_bBinarizeResult;
    };

public:
    typedef NaryMaskHandlerFunctor<MetadataHelper::SingleBandShortImageType::PixelType,
                                        MetadataHelper::SingleBandShortImageType::PixelType>    NaryMaskHandlerFunctorType;
    typedef itk::NaryFunctorImageFilter< MetadataHelper::SingleBandShortImageType,
                                        MetadataHelper::SingleBandShortImageType,
                                        NaryMaskHandlerFunctorType>                             NaryFunctorImageFilterType;

    MACCSMetadataHelper();

    const char * GetNameOfClass() { return "S2MetadataHelper"; }

    virtual std::string GetBandName(unsigned int nBandIdx, bool bRelativeIdx=true);
    virtual int GetRelativeBandIndex(unsigned int nAbsBandIdx);
    virtual float GetAotQuantificationValue();
    virtual float GetAotNoDataValue();
    virtual int GetAotBandIndex();

protected:
    virtual bool DoLoadMetadata();

    void UpdateValuesForLandsat();
    void UpdateValuesForSentinel();

    std::string getImageFileName();
    std::string getAotFileName();
    std::string getCloudFileName();
    std::string getWaterFileName();
    std::string getSnowFileName();
    std::string getQualityFileName();

    std::string getMACCSImageFileName(const std::vector<MACCSFileInformation>& imageFiles,
                                      const std::string& ending);
    std::string getMACCSImageFileName(const std::vector<MACCSAnnexInformation>& maskFiles,
                                      const std::string& ending);
    bool getMACCSImageFileName(const MACCSFileInformation& fileInfo,
                                      const std::string& ending, std::string& retStr);
    std::string getMACCSImageHdrName(const std::vector<MACCSAnnexInformation>& maskFiles,
                                     const std::string& ending);
    std::string getMACCSImageHdrName(const std::vector<MACCSFileInformation>& imageFiles,
                                                          const std::string& ending);
    bool getMACCSImageHdrName(const MACCSFileInformation& fileInfo,
                              const std::string& ending, std::string &retStr);
    void ReadSpecificMACCSImgHdrFile();
    void ReadSpecificMACCSAotHdrFile();
    void ReadSpecificMACCSCldHdrFile();
    void ReadSpecificMACCSMskHdrFile();
    int getBandIndex(const std::vector<MACCSBand>& bands, const std::string& name);
    bool CheckFileExistence(std::string &fileName);

    void InitializeS2Angles();
    bool BandAvailableForCurrentResolution(unsigned int nBand);
    const MACCSResolution& GetMACCSResolutionInfo(int nResolution);
    std::vector<MACCSBand> GetAllMACCSBandsInfos();
    //virtual std::unique_ptr<itk::LightObject> GetMetadata() { return m_metadata; }

    virtual MetadataHelper::SingleBandShortImageType::Pointer GetMasksImage(MasksFlagType nMaskFlags, bool binarizeResult);

protected:
    typedef enum {S2, LANDSAT} MissionType;
    MissionType m_missionType;
    std::unique_ptr<MACCSFileMetadata> m_metadata;
    std::unique_ptr<MACCSFileMetadata> m_specificAotMetadata;
    std::unique_ptr<MACCSFileMetadata> m_specificImgMetadata;
    std::unique_ptr<MACCSFileMetadata> m_specificCldMetadata;
    std::unique_ptr<MACCSFileMetadata> m_specificMskMetadata;

    ResamplingBandExtractor2<short> m_bandsExtractor;

    NaryMaskHandlerFunctorType m_maskHandlerFunctor;
    NaryFunctorImageFilterType::Pointer m_maskHandlerFilter;

};

#endif // S2METADATAHELPER_H
