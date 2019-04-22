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

#ifndef MAJAMETADATAHELPER_H
#define MAJAMETADATAHELPER_H

#include "MACCSS2MetadataHelper.h"
#include <vector>

#include "ResamplingBandExtractor.h"
#include "itkUnaryFunctorImageFilter.h"

template <typename PixelType, typename MasksPixelType>
class MAJAMetadataHelper : public MACCSS2MetadataHelper<PixelType, MasksPixelType>
{
protected:
    template< class TInput, class TOutput>
    class MAJAMaskHandlerFunctor
    {
    public:
        MAJAMaskHandlerFunctor(){}
        void Initialize(MasksFlagType nMaskFlags, bool binarizeResult) { m_MaskFlags = nMaskFlags; m_bBinarizeResult = binarizeResult;}
        MAJAMaskHandlerFunctor& operator =(const MAJAMaskHandlerFunctor& copy) {
            m_MaskFlags=copy.m_MaskFlags;
            m_bBinarizeResult = copy.m_bBinarizeResult;
            return *this;
        }
        bool operator!=( const MAJAMaskHandlerFunctor & a) const { return (this->m_MaskFlags != a.m_MaskFlags) || (this->m_bBinarizeResult != a.m_bBinarizeResult) ;}
        bool operator==( const MAJAMaskHandlerFunctor & a ) const { return !(*this != a); }

        TOutput operator()( const TInput & B) {
            TOutput res = GetSingleBandMask(B);
            if (m_bBinarizeResult) {
                // return 0 if LAND and 1 otherwise
                return (res != IMG_FLG_LAND);
            }
            return res;
        }

        // The expected order in the array would be : Cloud/Water/Snow, Saturation, Valid
        TOutput operator()( const std::vector< TInput > & B) {
            TOutput res = computeOutput(B);
            if (m_bBinarizeResult) {
                // return 0 if LAND and 1 otherwise
                return (res != IMG_FLG_LAND);
            }
            return res;
        }

        bool IsCloud(TInput val) {
            return (((val & 0x02) != 0) || ((val & 0x08) != 0));
        }
        bool IsNoData(TInput val) {
            return val != 0;
        }
        bool IsSaturation(TInput val) {
            return val != 0;
        }
        bool IsWater(TInput val) {
            return ((val & 0x01) != 0);
        }
        bool IsSnow(TInput val) {
            return ((val & 0x04) != 0);
        }

        TOutput GetSingleBandMask(const TInput & B) {
            if(((m_MaskFlags & MSK_VALID) != 0) && IsNoData(B)) return IMG_FLG_NO_DATA;
            // check bit 2 of MG2 (cloud_mask_all_cloud, result of a “logical OR” for all the cloud masks) and
            // and bit 4 of MG2 (logical OR between CM7 and CM8): shadow masks of clouds)
            if(((m_MaskFlags & MSK_CLOUD) != 0) && IsCloud(B)) return IMG_FLG_CLOUD;
            // check MG2 mask bit 1
            if(((m_MaskFlags & MSK_WATER) != 0) && IsWater(B)) return IMG_FLG_WATER;
            // check MG2 mask bit 3
            if(((m_MaskFlags & MSK_SNOW) != 0) && IsSnow(B)) return IMG_FLG_SNOW;
            // saturation - here is not quite correct as we have the saturation distinct for each band but we do
            // not have an API for making the distinction so we  consider it globally
            if(((m_MaskFlags & MSK_SAT) != 0) && IsSaturation(B)) return IMG_FLG_SATURATION;

            // default
            return IMG_FLG_LAND;
        }

        TOutput computeOutput( const std::vector< TInput > & B) {
            // The order is  (MSK_SNOW/MSK_WATTER/MSK_CLOUD), MSK_SAT, MSK_VALID
            switch (B.size())
            {
            case 1:
                return GetSingleBandMask(B[0]);
            case 2:
                if((m_MaskFlags & MSK_VALID) != 0) {
                    // EDG[1] + (CLD/WAT/SNOW  OR SAT)[0]
                    if(IsNoData(B[1])) return IMG_FLG_NO_DATA;

                    // EDG[1] + SAT[0]
                    if((m_MaskFlags & MSK_SAT) != 0) {
                        if(IsSaturation(B[0]) != 0) return IMG_FLG_SATURATION;
                    } else {
                        //EDG[1] + CLD/WAT/SNOW [0]
                        if(((m_MaskFlags & MSK_CLOUD) != 0) && IsCloud(B[0])) return IMG_FLG_CLOUD;
                        if(((m_MaskFlags & MSK_WATER) != 0) && IsWater(B[0])) return IMG_FLG_WATER;
                        if(((m_MaskFlags & MSK_SNOW) != 0) && IsSnow(B[0])) return IMG_FLG_SNOW;
                    }
                } else {
                    // CLD/WAT/SNOW[0] + SAT[1]
                    if(((m_MaskFlags & MSK_CLOUD) != 0) && IsCloud(B[0])) return IMG_FLG_CLOUD;
                    if(((m_MaskFlags & MSK_WATER) != 0) && IsWater(B[0])) return IMG_FLG_WATER;
                    if(((m_MaskFlags & MSK_SNOW) != 0) && IsSnow(B[0])) return IMG_FLG_SNOW;
                    if(IsSaturation(B[1]) != 0) return IMG_FLG_SATURATION;
                }
                break;
            case 3:
                if(IsNoData(B[2])) return IMG_FLG_NO_DATA;
                if(((m_MaskFlags & MSK_CLOUD) != 0) && IsCloud(B[0])) return IMG_FLG_CLOUD;
                if(((m_MaskFlags & MSK_WATER) != 0) && IsWater(B[0])) return IMG_FLG_WATER;
                if(((m_MaskFlags & MSK_SNOW) != 0) && IsSnow(B[0])) return IMG_FLG_SNOW;
                if(IsSaturation(B[1]) != 0) return IMG_FLG_SATURATION;
                break;
            }
            return IMG_FLG_LAND;
        }

    private:
        MasksFlagType m_MaskFlags;
        bool m_bBinarizeResult;
    };

public:
    typedef MAJAMaskHandlerFunctor<typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::PixelType,
                                        typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::PixelType>    MAJAMaskHandlerFunctorType;
    typedef itk::NaryFunctorImageFilter< typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType,
                                        typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType,
                                        MAJAMaskHandlerFunctorType>                             MAJANaryFunctorImageFilterType;

    typedef itk::UnaryFunctorImageFilter< typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType,
                                        typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType,
                                        MAJAMaskHandlerFunctorType>                             MAJAUnaryFunctorImageFilterType;

public:
    MAJAMetadataHelper();

    const char * GetNameOfClass() { return "MAJAMetadataHelper"; }

    virtual typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer GetImage(const std::vector<std::string> &bandNames, int outRes = -1);
    virtual typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer GetImage(const std::vector<std::string> &bandNames,
                                                std::vector<int> *pRetRelBandIdxs, int outRes = -1);
    virtual typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer GetImageList(const std::vector<std::string> &bandNames,
                                                typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer outImgList, int outRes = -1);

    virtual std::string GetAotImageFileName(int res);
    virtual float GetAotQuantificationValue(int res);
    virtual float GetAotNoDataValue(int res);
    virtual int GetAotBandIndex(int res);
    virtual typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::Pointer GetMasksImage(MasksFlagType nMaskFlags, bool binarizeResult,
                                                                                                int resolution = -1);
protected:
    virtual bool LoadAndCheckMetadata(const std::string &file);
    virtual bool BandAvailableForResolution(const std::string &bandName, int nRes);
    virtual std::string GetMaccsImageExtension() { return ".TIF"; }

    virtual std::string getCloudFileName(int res);
    virtual std::string getWaterFileName(int res);
    virtual std::string getSnowFileName(int res);
    virtual std::string getQualityFileName(int res);
    virtual std::string getSaturationFileName(int res);
    virtual std::string getEdgeFileName(int res);

private:
    std::string GetImageFileName(const std::string &bandName);
    bool HasBandName(const std::vector<std::string> &bandNames, const std::string &bandName);
    bool GetValidBandNames(const std::vector<std::string> &bandNames, std::vector<std::string> &validBandNames,
                           std::vector<int> &relBandIndexes, int &outRes);
    float m_AotQuantifVal;
    float m_AotNoDataVal;

    MAJAMaskHandlerFunctorType m_majaMaskHandlerFunctor;
    // We are using 2 filters here as Nary functor needs at least two inputs while
    // we might have only one (MG2 for example)
    typename MAJAUnaryFunctorImageFilterType::Pointer m_majaSingleMaskHandlerFilter;
    typename MAJANaryFunctorImageFilterType::Pointer m_majaNMaskHandlerFilter;
};

#include "MAJAMetadataHelper.cpp"

#endif // MACCSMETADATAHELPERNEW_H
