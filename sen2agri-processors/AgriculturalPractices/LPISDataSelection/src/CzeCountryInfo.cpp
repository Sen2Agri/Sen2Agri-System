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

    if (m_year == "2018") {
        m_NKOD_DPB_FieldIdx = firstOgrFeat.GetFieldIndex("NKOD_DPB");
    } else {
        m_NKOD_DPB_FieldIdx = firstOgrFeat.GetFieldIndex("ori_id");
    }

}

std::string CzeCountryInfo::GetOriId(const AttributeEntry &ogrFeat) {
  return ogrFeat.GetFieldAsString(m_NKOD_DPB_FieldIdx);
}

std::string CzeCountryInfo::GetMainCrop(const AttributeEntry &ogrFeat) {
    if (m_year == "2018") {
        const std::string &plod2 = GetLpisInfos(GetOriId(ogrFeat)).plod2;
        // Ignore items that have plod2 filled
        if (plod2.size() > 0 && std::atoi(plod2.c_str()) > 0) {
            return "NA";
        }
        return GetLpisInfos(GetOriId(ogrFeat)).plod1;
    }
    return GetOriCrop(ogrFeat);
}

bool CzeCountryInfo::GetHasPractice(const AttributeEntry &ogrFeat,
                                    const std::string &practice) {
  bool bCheckVymera = false;
  const std::string &uid = GetOriId(ogrFeat);
  int seqId = GetSeqId(ogrFeat);
  if (seqId == 9744) {
      int i = 0;
      i++;
  }

  const EfaInfosType &efaInfos = GetEfaInfos(uid);
  const std::string &typEfa = efaInfos.typ_efa;
  if (practice == CATCH_CROP_VAL && typEfa == "MPL") {
      if(m_year == "2018") {
        bCheckVymera = true;
      } else {
          return true;
      }
  } else if (practice == NITROGEN_FIXING_CROP_VAL && typEfa == "PVN") {
      if(m_year == "2018") {
        bCheckVymera = true;
      } else {
          return true;
      }
  } else if ((practice == FALLOW_LAND_VAL) && (typEfa == "UHOZ" || typEfa == "UHOM")) {
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
    const std::string &mpl = GetEfaInfos(GetOriId(ogrFeat)).var_mpl;
    if (mpl == "L") {
      return "Summer";
    }
    return "Winter";
  } else if (m_practice == FALLOW_LAND_VAL) {
      const std::string &uid = GetOriId(ogrFeat);
      const EfaInfosType &efaInfos = GetEfaInfos(uid);
      const std::string &typEfa = efaInfos.typ_efa;
      if (typEfa == "UHOZ" || typEfa == "UHOM") {
         return typEfa;
      }
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

std::string CzeCountryInfo::GetHEnd(const AttributeEntry &ogrFeat) {
    if (m_practice == CATCH_CROP_VAL && m_year != "2018") {
        const std::string &pType = GetPracticeType(ogrFeat);
        if (pType == "Winter") {
            return m_hWinterEnd;
        }
    }
    return m_hend;
}

int CzeCountryInfo::Handle2018FileLine(const MapHdrIdx &header,
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

int CzeCountryInfo::Handle2019FileLine(const MapHdrIdx &header,
                                   const std::vector<std::string> &line,
                                   int) {
    MapHdrIdx::const_iterator itMap;
    EfaInfosType efaInfos;
    itMap = header.find("TYP_EFA");
    if (itMap != header.end() && itMap->second < line.size()) {
      efaInfos.typ_efa = line[itMap->second];
    }
    itMap = header.find("VAR_MPL");
    if (itMap != header.end() && itMap->second < line.size()) {
      efaInfos.var_mpl = line[itMap->second];
    }

    itMap = header.find("KOD_PB");
    if (itMap != header.end() && itMap->second < line.size()) {
      efaInfosMap[line[itMap->second]] = efaInfos;
    }
    return true;
}

int CzeCountryInfo::HandleFileLine(const MapHdrIdx &header,
                                   const std::vector<std::string> &line,
                                   int fileIdx) {
    if (m_year == "2018") {
        return Handle2018FileLine(header, line, fileIdx);
    } else if (m_year == "2019" || m_year == "2020") {
        return Handle2019FileLine(header, line, fileIdx);
    }
    return false;
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
