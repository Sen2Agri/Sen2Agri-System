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

#ifndef MACCSL8METADATAHELPER_H
#define MACCSL8METADATAHELPER_H

#include "ResamplingBandExtractor.h"
#include "MACCSMetadataHelperBase.h"
#include <vector>


template <typename PixelType, typename MasksPixelType>
class MACCSL8MetadataHelper: public MACCSMetadataHelperBase<PixelType, MasksPixelType>
{
public:
    MACCSL8MetadataHelper();

    const char * GetNameOfClass() { return "MACCSL8MetadataHelper"; }

    virtual typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer GetImage(const std::vector<std::string> &bandNames, int outRes = -1);
    virtual typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer GetImage(const std::vector<std::string> &bandNames,
                                                std::vector<int> *pRetRelBandIdxs, int outRes = -1);
    virtual typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer GetImageList(const std::vector<std::string> &bandNames,
                                               typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer outImgList, int outRes = -1);

    virtual std::vector<std::string> GetBandNamesForResolution(int);
    virtual std::vector<std::string> GetAllBandNames();
    virtual std::vector<std::string> GetPhysicalBandNames();
    virtual int GetResolutionForBand(const std::string &bandName);

    virtual std::string GetAotImageFileName(int) { return m_aotFileName;}
    virtual float GetAotQuantificationValue(int res);
    virtual float GetAotNoDataValue(int res);
    virtual int GetAotBandIndex(int res);

protected:

    virtual bool LoadAndUpdateMetadataValues(const std::string &file);

    std::string getCloudFileName(int res);
    std::string getWaterFileName(int res);
    std::string getSnowFileName(int res);
    std::string getQualityFileName(int res);

//    void ReadSpecificMACCSImgHdrFile();
    void ReadSpecificMACCSAotHdrFile();
//    void ReadSpecificMACCSCldHdrFile();
    void ReadSpecificMACCSMskHdrFile();

    bool BandAvailableForCurrentResolution(unsigned int nBand);
    std::vector<CommonBand> GetAllMACCSBandsInfos();

private:
    int GetRelativeBandIdx(const std::string &bandName);
    std::unique_ptr<MACCSFileMetadata> m_specificAotMetadata;
    std::unique_ptr<MACCSFileMetadata> m_specificImgMetadata;
    std::unique_ptr<MACCSFileMetadata> m_specificCldMetadata;
    std::unique_ptr<MACCSFileMetadata> m_specificMskMetadata;

    float m_fAotQuantificationValue;
    float m_fAotNoDataVal;
    int m_nAotBandIndex;

    std::string m_aotFileName;

};

#include "MACCSL8MetadataHelper.cpp"

#endif // MACCSL8METADATAHELPER_H
