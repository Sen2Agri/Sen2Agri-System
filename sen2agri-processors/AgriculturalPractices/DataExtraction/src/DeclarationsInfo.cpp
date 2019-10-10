#include "DeclarationsInfo.h"
#include "CommonFunctions.h"
#include "../../Common/include/PracticeReaderFactory.h"

DeclarationsInfo::DeclarationsInfo()
{
}

void DeclarationsInfo::SetCCIdsFile(const std::string &file) {
    LoadIdsFromPracticesFile(file, &m_ccIds);
}

void DeclarationsInfo::SetFLIdsFile(const std::string &file) {
    LoadIdsFromPracticesFile(file, &m_flIds);
}

void DeclarationsInfo::SetNFCIdsFile(const std::string &file) {
    LoadIdsFromPracticesFile(file, &m_nfcIds);
}

void DeclarationsInfo::SetNAIdsFile(const std::string &file) {
    LoadIdsFromPracticesFile(file, &m_naIds);
}

bool DeclarationsInfo::IsMonitoringParcel(const AttributeEntry &ogrFeat) {
    if (m_LandCoverFieldIdx == -1) {
        return true;    // we don't have the column
    }
    const char* field = ogrFeat.GetFieldAsString(m_LandCoverFieldIdx);
    if (field == NULL) {
        return true;
    }
    int fieldValue = std::atoi(field);
    if (HasPractice(ogrFeat, CATCH_CROP_VAL_ID) ||
        HasPractice(ogrFeat, FALLOW_LAND_VAL_ID) ||
        HasPractice(ogrFeat, NITROGEN_FIXING_CROP_VAL_ID)) {
        return (fieldValue > 0 && fieldValue < 5);
    }

    // does not have an EFA practice, in this case check if it is arable land
    return (fieldValue == 1);
}

bool DeclarationsInfo::HasPractice(const AttributeEntry &ogrFeat, int practice) {
    MapIds *pMap = NULL;
    switch(practice) {
        case CATCH_CROP_VAL_ID:
            pMap = &m_ccIds;
            break;
        case FALLOW_LAND_VAL_ID:
            pMap = &m_flIds;
            break;
        case NITROGEN_FIXING_CROP_VAL_ID:
            pMap = &m_nfcIds;
            break;
    }
    if (pMap == NULL || pMap->size() == 0) {
        return false;
    }
    int seqId = GetSeqId(ogrFeat);
    MapIds::const_iterator itMap = pMap->find(seqId);
    if (itMap == pMap->end()) {
        return false;
    }
    return true;
}

void DeclarationsInfo::LoadIdsFromPracticesFile(const std::string &file, MapIds *pMapToUpdate) {
    boost::filesystem::path practicesInfoPath(file);
    std::string pfFormat = practicesInfoPath.extension().c_str();
    pfFormat.erase(pfFormat.begin(), std::find_if(pfFormat.begin(), pfFormat.end(), [](int ch) {
            return ch != '.';
        }));

    auto practiceReadersFactory = PracticeReaderFactory::New();
    std::unique_ptr<PracticeReaderBase> practiceReader = practiceReadersFactory->GetPracticeReader(pfFormat);
    practiceReader->SetSource(file);

    using namespace std::placeholders;
    std::function<bool(const FeatureDescription&, void*)> f = std::bind(&DeclarationsInfo::HandleFeature, this, _1, _2);
    practiceReader->ExtractFeatures(f, (void*)pMapToUpdate);
}

bool DeclarationsInfo::HandleFeature(const FeatureDescription& feature, void *payload) {
    MapIds *pMap = (MapIds*)payload;
    if (pMap == NULL) {
        return false;
    }
    int fieldId = std::atoi(feature.GetFieldSeqId().c_str());
    (*pMap)[fieldId] = fieldId;
    return true;
}
