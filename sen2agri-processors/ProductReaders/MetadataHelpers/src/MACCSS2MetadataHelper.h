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

#ifndef MACCSS2METADATAHELPER_H
#define MACCSS2METADATAHELPER_H

#include "MACCSMetadataHelperBase.h"
#include <vector>

#include "ResamplingBandExtractor.h"

template <typename PixelType, typename MasksPixelType>
class MACCSS2MetadataHelper : public MACCSMetadataHelperBase<PixelType, MasksPixelType>
{
public:
    MACCSS2MetadataHelper();

    const char * GetNameOfClass() { return "MACCSS2MetadataHelper"; }

    virtual typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer GetImage(const std::vector<std::string> &bandNames, int outRes = -1);
    virtual typename MetadataHelper<PixelType, MasksPixelType>::VectorImageType::Pointer GetImage(const std::vector<std::string> &bandNames,
                                                std::vector<int> *pRetRelBandIdxs, int outRes = -1);
    virtual typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer GetImageList(const std::vector<std::string> &bandNames,
                                                typename MetadataHelper<PixelType, MasksPixelType>::ImageListType::Pointer outImgList, int outRes = -1);

    virtual std::vector<std::string> GetBandNamesForResolution(int res);
    virtual std::vector<std::string> GetAllBandNames();
    virtual std::vector<std::string> GetPhysicalBandNames();
    virtual int GetResolutionForBand(const std::string &bandName);

    virtual std::string GetAotImageFileName(int res);
    virtual float GetAotQuantificationValue(int res);
    virtual float GetAotNoDataValue(int res);
    virtual int GetAotBandIndex(int res);

    virtual std::vector<MetadataHelperViewingAnglesGrid> GetDetailedViewingAngles(int res);

protected:
    virtual bool LoadAndUpdateMetadataValues(const std::string &file);

    virtual std::string getCloudFileName(int res);
    virtual std::string getWaterFileName(int res);
    virtual std::string getSnowFileName(int res);
    virtual std::string getQualityFileName(int res);

//    void ReadSpecificMACCSImgHdrFile(int res);
    typename MetadataHelper<PixelType, MasksPixelType>::AotInfos *ReadSpecificMACCSAotHdrFile(int res);
//    void ReadSpecificMACCSCldHdrFile(int res);
//    void ReadSpecificMACCSMskHdrFile(int res);

    virtual void InitializeS2Angles();
    virtual bool BandAvailableForResolution(const std::string &bandId, int nRes);
    const CommonResolution& GetMACCSResolutionInfo(int nResolution);
    std::vector<CommonBand> GetAllMACCSBandsInfos();
    std::map<int, std::vector<int>> GroupBandsByResolution(const std::vector<std::string> &bandNames, int &minRes,
                                                           std::map<int, std::vector<std::string> > &mapBandNames);
    int GetRelativeBandIdx(const std::string &bandName);

    // Function to be overwritten if a different format similar with MACCS
    virtual bool LoadAndCheckMetadata(const std::string &file);

protected:
    std::unique_ptr<MACCSFileMetadata> m_specificImgMetadata10M;
    std::unique_ptr<MACCSFileMetadata> m_specificImgMetadata20M;
    std::unique_ptr<MACCSFileMetadata> m_specificCldMetadata10M;
    std::unique_ptr<MACCSFileMetadata> m_specificCldMetadata20M;
    std::unique_ptr<MACCSFileMetadata> m_specificMskMetadata10M;
    std::unique_ptr<MACCSFileMetadata> m_specificMskMetadata20M;

private:
    std::string getImageFileName(int res);
    bool is_number(const std::string& s);

    typename MetadataHelper<PixelType, MasksPixelType>::AotInfos aotInfos10M;
    typename MetadataHelper<PixelType, MasksPixelType>::AotInfos aotInfos20M;

    std::vector<MACCSBandViewingAnglesGrid> m_maccsBandViewingAngles;
};

#include "MACCSS2MetadataHelper.cpp"

#endif // MACCSMETADATAHELPERNEW_H
