#include "CountryInfoBase.h"

CountryInfoBase::CountryInfoBase()
{
    m_SeqIdFieldIdx = -1;
    m_LandCoverFieldIdx = -1;

    m_GeomValidIdx = -1;
    m_DuplicIdx = -1;
    m_OverlapIdx = -1;
    m_Area_meterIdx = -1;
    m_ShapeIndIdx = -1;
    m_CTnumIdx = -1;
    m_CTIdx = -1;
    m_S1PixIdx = -1;
    m_S2PixIdx = -1;
}

void CountryInfoBase::InitializeIndexes(const AttributeEntry &firstOgrFeat)
{
    m_SeqIdFieldIdx = firstOgrFeat.GetFieldIndex(SEQ_UNIQUE_ID);
    m_LandCoverFieldIdx = firstOgrFeat.GetFieldIndex(LC_VAL);
    if (m_LandCoverFieldIdx == -1) {
        // Check if maybe we have the the old CR_CAT_VAL Field
        m_LandCoverFieldIdx = firstOgrFeat.GetFieldIndex(CR_CAT_VAL);
    }

    m_GeomValidIdx = firstOgrFeat.GetFieldIndex("GeomValid");
    m_DuplicIdx = firstOgrFeat.GetFieldIndex("Duplic");
    m_OverlapIdx = firstOgrFeat.GetFieldIndex("Overlap");
    m_Area_meterIdx = firstOgrFeat.GetFieldIndex("Area_meter");
    m_ShapeIndIdx = firstOgrFeat.GetFieldIndex("ShapeInd");
    m_CTnumIdx = firstOgrFeat.GetFieldIndex("CTnum");
    m_CTIdx = firstOgrFeat.GetFieldIndex("CT");
    m_S1PixIdx = firstOgrFeat.GetFieldIndex("S1Pix");
    m_S2PixIdx = firstOgrFeat.GetFieldIndex("S2Pix");

}

void CountryInfoBase::SetAdditionalFiles(const std::vector<std::string> &additionalFiles) {
    m_additionalFiles.insert(m_additionalFiles.end(), additionalFiles.begin(), additionalFiles.end());
    for(const auto &file: m_additionalFiles) {
        const std::string &extension = boost::filesystem::extension(file);
        if (boost::iequals(extension, ".csv")) {
            if (m_LineHandlerFnc == nullptr) {
                std::cout << "ERROR: Additional files provided but no handler function defined for this country" << std::endl;
                continue;
            }
            ParseCsvFile(file, m_LineHandlerFnc);
        } else if (boost::iequals(extension, ".shp")) {
            ParseShpFile(file, m_ShpFeatHandlerFnc);
        }
    }
}

int CountryInfoBase::GetSeqId(const AttributeEntry &ogrFeat) {
    if (m_SeqIdFieldIdx == -1) {
        return -1;    // we don't have the column
    }
    return (int)ogrFeat.GetFieldAsDouble(m_SeqIdFieldIdx);
}

void CountryInfoBase::SetYear(const std::string &val) { m_year = val;}
void CountryInfoBase::SetVegStart(const std::string &val) { m_vegstart = val;}
void CountryInfoBase::SetHStart(const std::string &val) { m_hstart = val;}
void CountryInfoBase::SetHWinterStart(const std::string &val) { m_hWinterStart = val;}
void CountryInfoBase::SetHEnd(const std::string &val) { m_hend = val;}
void CountryInfoBase::SetPractice(const std::string &val) { m_practice = val;}
void CountryInfoBase::SetPStart(const std::string &val) { m_pstart = val;}
void CountryInfoBase::SetPEnd(const std::string &val) { m_pend = val;}
void CountryInfoBase::SetWinterPStart(const std::string &val) { m_pWinterStart = val;}
void CountryInfoBase::SetWinterPEnd(const std::string &val) { m_pWinterEnd = val;}

std::string CountryInfoBase::GetYear() {return m_year;}
std::string CountryInfoBase::GetVegStart() {return m_vegstart;}
std::string CountryInfoBase::GetHStart(const AttributeEntry &ogrFeat) {(void)ogrFeat ; return m_hstart;}
std::string CountryInfoBase::GetHEnd(const AttributeEntry &ogrFeat) {(void)ogrFeat ; return m_hend;}
std::string CountryInfoBase::GetPractice() {return m_practice;}
std::string CountryInfoBase::GetPractice(const AttributeEntry &ogrFeat) { (void)ogrFeat ; return GetPractice(); }
std::string CountryInfoBase::GetPracticeType(const AttributeEntry &ogrFeat) {(void)ogrFeat ; return m_ptype;}
std::string CountryInfoBase::GetPStart(const AttributeEntry &ogrFeat) {(void)ogrFeat ; return m_pstart;}
std::string CountryInfoBase::GetPEnd(const AttributeEntry &ogrFeat) {(void)ogrFeat ; return m_pend;}

bool CountryInfoBase::IsMonitoringParcel(const AttributeEntry &ogrFeat) {
    if (m_LandCoverFieldIdx == -1) {
        return true;    // we don't have the column
    }
    const char* field = ogrFeat.GetFieldAsString(m_LandCoverFieldIdx);
    if (field == NULL) {
        return true;
    }
    int fieldValue = std::atoi(field);
    if (GetHasPractice(ogrFeat, CATCH_CROP_VAL) ||
        GetHasPractice(ogrFeat, FALLOW_LAND_VAL) ||
        GetHasPractice(ogrFeat, NITROGEN_FIXING_CROP_VAL)) {
        return (fieldValue > 0 && fieldValue < 5);
    }

    // does not have an EFA practice, in this case check if it is arable land
    return (fieldValue == 1);
}

void CountryInfoBase::ParseCsvFile(const std::string &filePath,
                  std::function<int(const MapHdrIdx&, const std::vector<std::string>&, int)> fnc) {
    std::cout << "Loading CSV file " << filePath << std::endl;

    std::ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        std::cout << "ERROR: cannot open CSV file " << filePath << std::endl;
        return;
    }
    std::ptrdiff_t pos = std::find(m_additionalFiles.begin(), m_additionalFiles.end(), filePath) - m_additionalFiles.begin();
    std::string line;
    size_t curLine = 0;
    MapHdrIdx headerLineMap;
    while (std::getline(fileStream, line))
    {
        // first line should be the header
        std::vector<std::string> lineItems = GetInputFileLineElements(line);
        if (curLine++ == 0) {
            for(size_t i = 0; i<lineItems.size(); i++) {
                headerLineMap[lineItems[i]] = i;
            }
            continue;
        }
        if (lineItems.size() == headerLineMap.size()) {
            fnc(headerLineMap, lineItems, pos);
        }
    }
}

void CountryInfoBase::ParseShpFile(const std::string &filePath,
                  std::function<int(OGRFeature&, int)> fnc) {
    std::cout << "Loading SHP file " << filePath << std::endl;

    std::ptrdiff_t pos = std::find(m_additionalFiles.begin(), m_additionalFiles.end(), filePath) - m_additionalFiles.begin();

    otb::ogr::DataSource::Pointer source = otb::ogr::DataSource::New(
        filePath, otb::ogr::DataSource::Modes::Read);
    for (otb::ogr::DataSource::const_iterator lb=source->begin(), le=source->end(); lb != le; ++lb)
    {
        otb::ogr::Layer const& inputLayer = *lb;
        otb::ogr::Layer::const_iterator featIt = inputLayer.begin();
        for(; featIt!=inputLayer.end(); ++featIt)
        {
            OGRFeature &ogrFeat = (*featIt).ogr();
            fnc(ogrFeat, pos);
        }
    }
}

std::vector<std::string> CountryInfoBase::GetInputFileLineElements(const std::string &line) {
    std::vector<std::string> results;
    boost::algorithm::split(results, line, [](char c){return c == ';';});
    return results;
}

std::string CountryInfoBase::GetFieldOrNA(const AttributeEntry &ogrFeat, int idx) {
    if (idx != -1) {
        const char* field = ogrFeat.GetFieldAsString(idx);
        if (field != NULL) {
            return field;
        }
    }
    return "NA";
}
