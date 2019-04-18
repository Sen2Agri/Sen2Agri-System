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

#ifndef SPOT4METADATAHELPER_H
#define SPOT4METADATAHELPER_H

#include "MetadataHelper.h"
#include "ResamplingBandExtractor.h"

#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"
#include "itkNaryFunctorImageFilter.h"

template <typename PixelType, typename MasksPixelType>
class Spot4MetadataHelper : public MetadataHelper<PixelType, MasksPixelType>
{
    template< class TInput, class TOutput>
    class SpotMaskHandlerFunctor
    {
    public:
        SpotMaskHandlerFunctor(){}
        void Initialize(MasksFlagType nMaskFlags, bool binarizeResult) { m_MaskFlags = nMaskFlags; m_bBinarizeResult = binarizeResult;}
        SpotMaskHandlerFunctor& operator =(const SpotMaskHandlerFunctor& copy) {
            m_MaskFlags=copy.m_MaskFlags;
            m_bBinarizeResult = copy.m_bBinarizeResult;
            return *this;
        }
        bool operator!=( const SpotMaskHandlerFunctor & a) const { return   (this->m_MaskFlags != a.m_MaskFlags) ||
                                                                            (this->m_bBinarizeResult != a.m_bBinarizeResult) ;}
        bool operator==( const SpotMaskHandlerFunctor & a ) const { return !(*this != a); }

        TOutput operator()( const std::vector< TInput > & B) {
            switch (B.size())
            {
            case 1:
                if((m_MaskFlags & MSK_CLOUD) != 0) {
                    if(B[0] != 0) return m_bBinarizeResult ? 1 : IMG_FLG_CLOUD;
                } else if((m_MaskFlags & MSK_SAT) != 0) {
                    if(B[0] != 0) return m_bBinarizeResult ? 1 : IMG_FLG_SATURATION;
                } else {
                    // in this case we have div file (snow, water etc)
                    if((B[0] & 0x01) != 0) return m_bBinarizeResult ? 1 : IMG_FLG_NO_DATA;
                    if(((m_MaskFlags & MSK_WATER) != 0) && ((B[0] & 0x02) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_WATER;
                    if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[0] & 0x04) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_SNOW;
                }
                break;
            case 2:
                // we have values only for 2 of them and they are the first two
                // we determine them based on the flag type
                if((m_MaskFlags & MSK_CLOUD) != 0) {
                    if(B[0] != 0) return m_bBinarizeResult ? 1 : IMG_FLG_CLOUD;
                    if((m_MaskFlags & MSK_SAT) != 0) {
                        // in this case in the second value val2 we have saturation
                        if(B[1] != 0) return m_bBinarizeResult ? 1 : IMG_FLG_SATURATION;
                    } else {
                        // in this case we have div file (snow, water etc)
                        if((B[1] & 0x01) != 0) return m_bBinarizeResult ? 1 : IMG_FLG_NO_DATA;
                        if(((m_MaskFlags & MSK_WATER) != 0) && ((B[1] & 0x02) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_WATER;
                        if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[1] & 0x04) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_SNOW;
                    }
                } else {
                    // it means that we have in val1 and val2 the value from div file and from saturation
                    if((B[0] & 0x01) != 0) return m_bBinarizeResult ? 1 : IMG_FLG_NO_DATA;
                    if(((m_MaskFlags & MSK_WATER) != 0) && ((B[0] & 0x02) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_WATER;
                    if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[0] & 0x04) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_SNOW;

                    if(B[1] != 0) return m_bBinarizeResult ? 1 : IMG_FLG_SATURATION;
                }
                break;
            case 3:
                if((B[1] & 0x01) != 0) return m_bBinarizeResult ? 1 : IMG_FLG_NO_DATA;
                if(((m_MaskFlags & MSK_WATER) != 0) && ((B[1] & 0x02) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_WATER;
                if(((m_MaskFlags & MSK_SNOW) != 0) && ((B[1] & 0x04) != 0)) return m_bBinarizeResult ? 1 : IMG_FLG_SNOW;

                // if we have cloud,
                if(B[0] != 0) return m_bBinarizeResult ? 1 : IMG_FLG_CLOUD;
                if(B[2] != 0) return m_bBinarizeResult ? 1 : IMG_FLG_SATURATION;
            }
            return m_bBinarizeResult ? 0 : IMG_FLG_LAND;
        }
    private:
        MasksFlagType m_MaskFlags;
        bool m_bBinarizeResult;
    };


public:
    typedef SpotMaskHandlerFunctor<typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::PixelType,
                                        typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::PixelType>    SpotMaskHandlerFunctorType;
    typedef itk::NaryFunctorImageFilter< typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType,
                                        typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType,
                                        SpotMaskHandlerFunctorType>                             NaryFunctorImageFilterType;

    Spot4MetadataHelper();

    const char * GetNameOfClass() { return "Spot4MetadataHelper"; }

    virtual typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer GetImage(const std::vector<std::string> &bandNames,
                                                                                                  int outRes = -1);
    virtual typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer GetImage(const std::vector<std::string> &bandNames,
                                                   std::vector<int> *pRetRelBandIdxs, int outRes = -1);
    virtual typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer GetImageList(const std::vector<std::string> &bandNames,
                                                typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer outImgList, int outRes = -1);

    virtual std::vector<std::string> GetBandNamesForResolution(int);
    virtual std::vector<std::string> GetAllBandNames();
    virtual std::vector<std::string> GetPhysicalBandNames();
    virtual int GetResolutionForBand(const std::string &bandName);

    virtual typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::Pointer GetMasksImage(MasksFlagType nMaskFlags, bool binarizeResult, int resolution);

    virtual std::string GetAotImageFileName(int) {return m_AotFileName;}
    virtual float GetAotQuantificationValue(int res);
    virtual float GetAotNoDataValue(int res);
    virtual int GetAotBandIndex(int res);

protected:
    virtual bool DoLoadMetadata(const std::string& file);

    std::string DeriveFileNameFromImageFileName(const std::string& replacement);
    int GetRelativeBandIdx(const std::string &bandName);

    std::string getImageFileName();
    std::string getAotFileName();
    std::string getCloudFileName();
    std::string getWaterFileName();
    std::string getSnowFileName();
    std::string getSaturationFileName();

    std::unique_ptr<SPOT4Metadata> m_metadata;

    SpotMaskHandlerFunctorType m_maskHandlerFunctor;
    typename NaryFunctorImageFilterType::Pointer m_maskHandlerFilter;

    std::string m_AotFileName;

};

#include "Spot4MetadataHelper.cpp"

#endif // SPOT4METADATAHELPER_H
