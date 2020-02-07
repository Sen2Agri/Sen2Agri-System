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

#ifndef SEN2CORMETADATAHELPER_H
#define SEN2CORMETADATAHELPER_H

#include "MetadataHelper.h"
#include <vector>

#include "../../MACCSMetadata/include/SEN2CORMetadataReader.hpp"
#include "ResamplingBandExtractor.h"
#include "itkNaryFunctorImageFilter.h"
#include "itkUnaryFunctorImageFilter.h"

typedef itk::SEN2CORMetadataReader                                   SEN2CORMetadataReaderType;

template <typename PixelType, typename MasksPixelType>
class SEN2CORMetadataHelper : public MetadataHelper<PixelType, MasksPixelType>
{
    template< class TInput, class TOutput>
    class Sen2CorMaskHandlerFunctor
    {
    public:
        Sen2CorMaskHandlerFunctor(){}
        void Initialize(MasksFlagType nMaskFlags, bool binarizeResult) { m_MaskFlags = nMaskFlags; m_bBinarizeResult = binarizeResult;}
        Sen2CorMaskHandlerFunctor& operator =(const Sen2CorMaskHandlerFunctor& copy) {
            m_MaskFlags=copy.m_MaskFlags;
            m_bBinarizeResult = copy.m_bBinarizeResult;
            return *this;
        }
        bool operator!=( const Sen2CorMaskHandlerFunctor & a) const { return (this->m_MaskFlags != a.m_MaskFlags) || (this->m_bBinarizeResult != a.m_bBinarizeResult) ;}
        bool operator==( const Sen2CorMaskHandlerFunctor & a ) const { return !(*this != a); }

        TOutput operator()( const TInput & B) {
            TOutput res = GetSingleBandMask(B);
            if (m_bBinarizeResult) {
                // return 0 if LAND and 1 otherwise
                return (res != IMG_FLG_LAND);
            }
            return res;
        }

        bool IsCloud(TInput val) {
            // cloud medium, high probability but also cloud shadows
            return ((val == 8) || (val == 9) || (val == 3));
        }
        bool IsNoData(TInput val) {
            return (val == 0);
        }
        bool IsSaturation(TInput val) {
            return (val == 1);
        }
        bool IsWater(TInput val) {
            return (val == 6);
        }
        bool IsSnow(TInput val) {
            return (val == 11);
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

    private:
        MasksFlagType m_MaskFlags;
        bool m_bBinarizeResult;
    };


public:
    typedef Sen2CorMaskHandlerFunctor<typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::PixelType,
                                        typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType::PixelType>    Sen2CorMaskHandlerFunctorType;
    typedef itk::NaryFunctorImageFilter< typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType,
                                        typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType,
                                        Sen2CorMaskHandlerFunctorType>                             Sen2CorNaryFunctorImageFilterType;

    typedef itk::UnaryFunctorImageFilter< typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType,
                                        typename MetadataHelper<PixelType, MasksPixelType>::SingleBandMasksImageType,
                                        Sen2CorMaskHandlerFunctorType>                             Sen2CorUnaryFunctorImageFilterType;

    SEN2CORMetadataHelper();

    const char * GetNameOfClass() { return "SEN2CORMetadataHelper"; }

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

    virtual std::string GetAotImageFileName(int res);
    virtual float GetAotQuantificationValue(int res);
    virtual float GetAotNoDataValue(int res);
    virtual int GetAotBandIndex(int res);
    virtual std::vector<MetadataHelperViewingAnglesGrid> GetDetailedViewingAngles(int res);
    virtual bool BandAvailableForResolution(const std::string &bandId, int nRes);

protected:
    virtual bool DoLoadMetadata(const std::string& file);

    int GetRelativeBandIdx(const std::string &bandName);

    std::string GetSCLFileName(int res);

    bool is_number(const std::string& s);

    virtual void InitializeS2Angles();

    std::unique_ptr<MACCSFileMetadata> m_metadata;
    bool m_bGranuleMetadataUpdated;

    Sen2CorMaskHandlerFunctorType m_maskHandlerFunctor;
    typename Sen2CorUnaryFunctorImageFilterType::Pointer m_maskHandlerFilter;

    float m_AotQuantifVal;
    float m_AotNoDataVal;
    std::vector<MACCSBandViewingAnglesGrid> m_bandViewingAngles;

private:
    std::string GetRasterPathForBandName(const std::string &bandName, int res);
    std::string GetImageFileName(const std::string &bandName, int prefferedRes);
    bool HasBandName(const std::vector<std::string> &bandNames, const std::string &bandName);
    bool GetValidBandNames(const std::vector<std::string> &bandNames, std::vector<std::string> &validBandNames,
                           std::vector<int> &relBandIndexes, int &outRes);

    std::string GetSen2CorImageFileName(const std::vector<CommonFileInformation>& imageFiles,
                                      const std::string& ending);
    std::string GetSen2CorImageFileName(const std::vector<CommonAnnexInformation>& maskFiles,
                                      const std::string& ending);
    bool GetSen2CorImageFileName(const CommonFileInformation& fileInfo,
                                      const std::string& ending, std::string& retStr);

    virtual std::string GetRasterFileExtension() { return ".jp2"; }

    bool CheckFileExistence(std::string &fileName);
    std::string GetGranuleXmlPath(const std::unique_ptr<MACCSFileMetadata> &metadata);
    void UpdateFromGranuleMetadata();
    std::string NormalizeBandName(const std::string &bandName);

};

#include "SEN2CORMetadataHelper.cpp"

#endif // SEN2CORMETADATAHELPER_H
