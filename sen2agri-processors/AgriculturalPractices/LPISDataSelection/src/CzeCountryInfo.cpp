#include "CzeCountryInfo.h"

CzeCountryInfo::CzeCountryInfo() {
    m_NKOD_DPB_FieldIdx = -1;
    using namespace std::placeholders;
    m_LineHandlerFnc =
        std::bind(&CzeCountryInfo::HandleFileLine, this, _1, _2, _3);
}

std::string CzeCountryInfo::GetName() { return "CZE"; }

void CzeCountryInfo::InitializeIndexes(const AttributeEntry &firstOgrFeat)
{
    CountryInfoBase::InitializeIndexes(firstOgrFeat);

    m_NKOD_DPB_FieldIdx = firstOgrFeat.GetFieldIndex("NKOD_DPB");
}

std::string CzeCountryInfo::GetUniqueId(const AttributeEntry &ogrFeat) {
  return ogrFeat.GetFieldAsString(m_NKOD_DPB_FieldIdx);
}

std::string CzeCountryInfo::GetMainCrop(const AttributeEntry &ogrFeat) {
  const std::string &plod2 = GetLpisInfos(GetUniqueId(ogrFeat)).plod2;
  // Ignore items that have plod2 filled
  if (plod2.size() > 0 && std::atoi(plod2.c_str()) > 0) {
    return "NA";
  }
  return GetLpisInfos(GetUniqueId(ogrFeat)).plod1;
}

bool CzeCountryInfo::GetHasPractice(const AttributeEntry &ogrFeat,
                                    const std::string &practice) {
  bool bCheckVymera = false;
  const std::string &uid = GetUniqueId(ogrFeat);
  const EfaInfosType &efaInfos = GetEfaInfos(uid);
  const std::string &typEfa = efaInfos.typ_efa;
  if (practice == CATCH_CROP_VAL && typEfa == "MPL") {
    bCheckVymera = true;
  } else if (practice == NITROGEN_FIXING_CROP_VAL && typEfa == "PVN") {
    bCheckVymera = true;
  } else if (practice == FALLOW_LAND_VAL && typEfa == "UHOZ") {
    return true;
  }
  if (bCheckVymera) {
    const LpisInfosType &lpisInfos = GetLpisInfos(uid);
    if (lpisInfos.vymera == efaInfos.vym_efa) {
      return true;
    }
  }
  return false;
}

std::string CzeCountryInfo::GetPracticeType(const AttributeEntry &ogrFeat) {
  if (m_practice == CATCH_CROP_VAL) {
    const std::string &mpl = GetEfaInfos(GetUniqueId(ogrFeat)).var_mpl;
    if (mpl == "L") {
      return "Summer";
    }
    return "Winter";
  }
  return "NA";
}
std::string CzeCountryInfo::GetPStart(const AttributeEntry &ogrFeat) {
  if (m_practice == CATCH_CROP_VAL) {
    const std::string &pType = GetPracticeType(ogrFeat);
    if (pType == "Winter") {
      return m_pWinterStart;
    } else if (pType == "Summer") {
      return m_pstart;
    }
  } else if (m_practice == NITROGEN_FIXING_CROP_VAL) {
    return m_pstart;
  } else if (m_practice == FALLOW_LAND_VAL) {
    return m_pstart;
  }
  return "NA";
}
std::string CzeCountryInfo::GetPEnd(const AttributeEntry &ogrFeat) {
  if (m_practice == CATCH_CROP_VAL) {
    const std::string &pType = GetPracticeType(ogrFeat);
    if (pType == "Winter") {
      return m_pWinterEnd;
    } else if (pType == "Summer") {
      return m_pend;
    }
  } else if (m_practice == NITROGEN_FIXING_CROP_VAL ||
             m_practice == FALLOW_LAND_VAL) {
    return m_pend;
  }
  return "NA";
}

int CzeCountryInfo::HandleFileLine(const MapHdrIdx &header,
                                   const std::vector<std::string> &line,
                                   int fileIdx) {
  MapHdrIdx::const_iterator itMap;
  LpisInfosType lpisInfos;
  EfaInfosType efaInfos;
  switch (fileIdx) {
    case 0:  // LPIS file
      itMap = header.find("PLOD1");
      if (itMap != header.end() && itMap->second < line.size()) {
        lpisInfos.plod1 = line[itMap->second];
      }
      itMap = header.find("PLOD2");
      if (itMap != header.end() && itMap->second < line.size()) {
        lpisInfos.plod2 = line[itMap->second];
      }
      itMap = header.find("VYMERA");
      if (itMap != header.end() && itMap->second < line.size()) {
        lpisInfos.vymera = line[itMap->second];
        std::replace(lpisInfos.vymera.begin(), lpisInfos.vymera.end(), ',',
                     '.');
      }
      itMap = header.find("NKOD_DPB");
      if (itMap != header.end() && itMap->second < line.size()) {
        lpisInfosMap[line[itMap->second]] = lpisInfos;
      }
      break;
    case 1:  // EFA file
      itMap = header.find("TYP_EFA");
      if (itMap != header.end() && itMap->second < line.size()) {
        efaInfos.typ_efa = line[itMap->second];
      }
      itMap = header.find("VYM_EFA");
      if (itMap != header.end() && itMap->second < line.size()) {
        efaInfos.vym_efa = line[itMap->second];
        std::replace(efaInfos.vym_efa.begin(), efaInfos.vym_efa.end(), ',',
                     '.');
      }

      itMap = header.find("VAR_MPL");
      if (itMap != header.end() && itMap->second < line.size()) {
        efaInfos.var_mpl = line[itMap->second];
      }

      itMap = header.find("NKOD_DPB");
      if (itMap != header.end() && itMap->second < line.size()) {
        efaInfosMap[line[itMap->second]] = efaInfos;
      }
      break;
    default:
      return false;
  }
  return true;
}

CzeCountryInfo::LpisInfosType CzeCountryInfo::GetLpisInfos(
    const std::string &fid) {
  std::map<std::string, LpisInfosType>::const_iterator itMap =
      lpisInfosMap.find(fid);
  if (itMap != lpisInfosMap.end()) {
    return itMap->second;
  }
  return LpisInfosType();
}

CzeCountryInfo::EfaInfosType CzeCountryInfo::GetEfaInfos(
    const std::string &fid) {
  std::map<std::string, EfaInfosType>::const_iterator itMap =
      efaInfosMap.find(fid);
  if (itMap != efaInfosMap.end()) {
    return itMap->second;
  }
  return EfaInfosType();
}
