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

#ifndef MACCSMETADATAHELPERBASE_H
#define MACCSMETADATAHELPERBASE_H

#include "MetadataHelper.h"
#include <vector>

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"
#include "itkNaryFunctorImageFilter.h"

typedef itk::MACCSMetadataReader                                   MACCSMetadataReaderType;

template <typename PixelType, typename MasksPixelType>
class MACCSMetadataHelperBase : public MetadataHelper<PixelType, MasksPixelType>
{
protected:
    template< class TInput, class TOutput>
    class MACCSMaskHandlerFunctor
    {
    public:
        MACCSMaskHandlerFunctor(){}
        void Initialize(MasksFlagType nMaskFlags, bool binarizeResult) { m_MaskFlags = nMaskFlags; m_bBinarizeResult = binarizeResult;}
        MACCSMaskHandlerFunctor& operator =(const MACCSMaskHandlerFunctor& copy) {
            m_MaskFlags=copy.m_MaskFlags;
            m_bBinarizeResult = copy.m_bBinarizeResult;
            return *this;
        }
        bool operator!=( const MACCSMaskHandlerFunctor & a) const { return (this->m_MaskFlags != a.m_MaskFlags) || (this->m_bBinarizeResult != a.m_bBinarizeResult) ;}
        bool operator==( const MACCSMaskHandlerFunctor & a ) const { return !(*this != a); }

        // The expected order in the array would be : Cloud, Saturation, Valid, (Water or Snow)
        TOutput operator()( const std::vector< TInput > & B) {
            TOutput res = computeOutput(B);
            if (m_bBinarizeResult) {
                // return 0 if LAND and 1 otherwise
                return (res != IMG_FLG_LAND);
            }
            return res;
        }

        TOutput computeOutput( const std::vector< TInput > & B) {
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
                // The order is MSK_CLOUD, MSK_SAT
                if((m_MaskFlags & MSK_CLOUD) != 0) {
                    if((m_MaskFlags & MSK_SAT) != 0) {
                        if(B[0] != 0) return IMG_FLG_CLOUD;
                        if(B[1] != 0) return IMG_FLG_SATURATION;
                    } else {
                        // The order is MSK_CLOUD, MSK_VALID
                        if((m_MaskFlags & MSK_VALID) != 0) {
                            // Normally, we should start with the check of the validity as if the validity is no data, then
                            // there is no use to check the others. Also, we could get false cloud even if validity is not good
                            if((B[1] & 0x01) != 0) return IMG_FLG_NO_DATA;
                            if(B[0] != 0) return IMG_FLG_CLOUD;
                        } else {
                            // The order is MSK_CLOUD, (MSK_SNOW/MSK_WATTER)
                            if(B[0] != 0) return IMG_FLG_CLOUD;
                            // we have water or snow
                            if(((m_MaskFlags & MSK_WATER) != 0) && ((B[1] & 0x01) != 0)) return IMG_FLG_WATER;
                            if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[1] & 0x20) != 0)) return IMG_FLG_SNOW;
                        }
                    }
                } else {
                    // The order is MSK_SAT, MSK_VALID
                    // we have no cloud mask but we might have one of the others
                    if((m_MaskFlags & MSK_SAT) != 0) {
                        if((m_MaskFlags & MSK_VALID) != 0) {
                            // Normally, we should start with the check of the validity as if the validity is no data, then
                            // there is no use to check the others. Also, we could get false saturation even if validity is not good
                            if((B[1] & 0x01) != 0) return IMG_FLG_NO_DATA;
                            if(B[0] != 0) return IMG_FLG_SATURATION;
                        } else {
                            // The order is MSK_SAT, (MSK_SNOW/MSK_WATTER)
                            if(B[0] != 0) return IMG_FLG_SATURATION;
                            // we have water or snow
                            if(((m_MaskFlags & MSK_WATER) != 0) && ((B[1] & 0x01) != 0)) return IMG_FLG_WATER;
                            if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[1] & 0x20) != 0)) return IMG_FLG_SNOW;
                        }
                    } else {
                        // The order is MSK_VALID, (MSK_SNOW/MSK_WATTER)
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
                // The order is MSK_CLOUD, MSK_SAT, MSK_VALID, (MSK_SNOW/MSK_WATTER)
                // in this case we have certainly all images
                if(((m_MaskFlags & MSK_VALID) != 0) && ((B[2] & 0x01) != 0)) return IMG_FLG_NO_DATA;
                if(((m_MaskFlags & MSK_CLOUD) != 0) && (B[0] != 0)) return IMG_FLG_CLOUD;
                if(((m_MaskFlags & MSK_SAT) != 0) && (B[1] != 0)) return IMG_FLG_SATURATION;
                if(((m_MaskFlags & MSK_WATER) != 0) && ((B[3] & 0x01) != 0)) return IMG_FLG_WATER;
                if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[3] & 0x20) != 0)) return IMG_FLG_SNOW;
                break;
            }
            return IMG_FLG_LAND;
        }

    private:
        MasksFlagType m_MaskFlags;
        bool m_bBinarizeResult;
    };

public:
    typedef MACCSMaskHandlerFunctor<typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::PixelType,
                                        typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::PixelType>    MACCSMaskHandlerFunctorType;
    typedef itk::NaryFunctorImageFilter< typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType,
                                        typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType,
                                        MACCSMaskHandlerFunctorType>                             NaryFunctorImageFilterType;

    MACCSMetadataHelperBase();

    const char * GetNameOfClass() { return "MACCSMetadataHelperBase"; }

protected:
    virtual bool DoLoadMetadata(const std::string &file);

    virtual bool LoadAndUpdateMetadataValues(const std::string &file) = 0;

    virtual std::string getCloudFileName(int res) = 0;
    virtual std::string getWaterFileName(int res) = 0;
    virtual std::string getSnowFileName(int res) = 0;
    virtual std::string getQualityFileName(int res) = 0;

    std::string getMACCSImageFileName(const std::vector<CommonFileInformation>& imageFiles,
                                      const std::string& ending);
    std::string getMACCSImageFileName(const std::vector<CommonAnnexInformation>& maskFiles,
                                      const std::string& ending);
    bool getMACCSImageFileName(const CommonFileInformation& fileInfo,
                                      const std::string& ending, std::string& retStr);
    std::string getMACCSImageHdrName(const std::vector<CommonAnnexInformation>& maskFiles,
                                     const std::string& ending);
    std::string getMACCSImageHdrName(const std::vector<CommonFileInformation>& imageFiles,
                                                          const std::string& ending);
    bool getMACCSImageHdrName(const CommonFileInformation& fileInfo,
                              const std::string& ending, std::string &retStr);
    virtual std::string GetMaccsImageExtension() { return ".DBL.TIF"; }

    int getBandIndex(const std::vector<CommonBand>& bands, const std::string& name);
    bool CheckFileExistence(std::string &fileName);

    const CommonResolution& GetMACCSResolutionInfo(int nResolution);

    virtual typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::Pointer GetMasksImage(MasksFlagType nMaskFlags, bool binarizeResult,
                                                                                                int resolution = -1);

protected:
    std::unique_ptr<MACCSFileMetadata> m_metadata;

    MACCSMaskHandlerFunctorType m_maccsMaskHandlerFunctor;
    typename NaryFunctorImageFilterType::Pointer m_maccsMaskHandlerFilter;



};

#include "MACCSMetadataHelperBase.cpp"

#endif // MACCSMETADATAHELPERBASE_H
