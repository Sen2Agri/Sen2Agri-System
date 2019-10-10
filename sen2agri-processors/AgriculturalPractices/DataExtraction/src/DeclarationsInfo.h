#ifndef DeclarationsInfo_H
#define DeclarationsInfo_H

#include "otbOGRDataSourceWrapper.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <fstream>

#include "CommonDefs.h"

#include "../../Common/include/AttributeEntry.h"
#include "../../Common/include/DeclarationsInfoBase.h"
#include "../../Common/include/PracticeReaderBase.h"

typedef std::map<int, int> MapIds;

class DeclarationsInfo : public DeclarationsInfoBase {
public:
    DeclarationsInfo();
    virtual bool IsMonitoringParcel(const AttributeEntry &ogrFeat);
    // Allow subclasses to initialize the indexes for columns they might need in order to
    // perform a get field index at each operation
    virtual void SetCCIdsFile(const std::string &file);
    virtual void SetFLIdsFile(const std::string &file);
    virtual void SetNFCIdsFile(const std::string &file);
    virtual void SetNAIdsFile(const std::string &file);

protected:
    bool HasPractice(const AttributeEntry &ogrFeat, int practice);
    void LoadIdsFromPracticesFile(const std::string &file, MapIds *pMapToUpdate);
    bool HandleFeature(const FeatureDescription& feature, void *payload);

protected:
    std::string m_ccFile;
    std::string m_flFile;
    std::string m_nfcFile;
    std::string m_naFile;

    MapIds m_ccIds;
    MapIds m_flIds;
    MapIds m_nfcIds;
    MapIds m_naIds;

};

#endif
