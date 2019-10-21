#include "../../Common/include/DeclarationsInfoBase.h"
#include "CommonFunctions.h"

DeclarationsInfoBase::DeclarationsInfoBase()
{
    m_SeqIdFieldIdx = -1;
    m_OrigIdFieldIdx = -1;
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

void DeclarationsInfoBase::InitializeIndexes(const AttributeEntry &firstOgrFeat)
{
    m_SeqIdFieldIdx = firstOgrFeat.GetFieldIndex(SEQ_UNIQUE_ID);
    m_OrigIdFieldIdx = firstOgrFeat.GetFieldIndex(ORIG_UNIQUE_ID);
    m_OrigCropFieldIdx = firstOgrFeat.GetFieldIndex(ORIG_CROP);
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

int DeclarationsInfoBase::GetSeqId(const AttributeEntry &ogrFeat) {
    if (m_SeqIdFieldIdx == -1) {
        return -1;    // we don't have the column
    }
    return (int)ogrFeat.GetFieldAsDouble(m_SeqIdFieldIdx);
}

std::string DeclarationsInfoBase::GetOriId(const AttributeEntry &ogrFeat) {
    return GetFieldOrNA(ogrFeat, m_OrigIdFieldIdx);
}

std::string DeclarationsInfoBase::GetOriCrop(const AttributeEntry &ogrFeat) {
    return GetFieldOrNA(ogrFeat, m_OrigCropFieldIdx);
}

bool DeclarationsInfoBase::IsMonitoringParcel(const AttributeEntry &ogrFeat) {
    if (m_LandCoverFieldIdx == -1) {
        return true;    // we don't have the column
    }
    const char* field = ogrFeat.GetFieldAsString(m_LandCoverFieldIdx);
    if (field == NULL) {
        return true;
    }
    int fieldValue = std::atoi(field);
    return (fieldValue > 0 && fieldValue < 5);
}

std::string DeclarationsInfoBase::GetFieldOrNA(const AttributeEntry &ogrFeat, int idx) {
    if (idx != -1) {
        const char* field = ogrFeat.GetFieldAsString(idx);
        if (field != NULL) {
            return field;
        }
    }
    return "NA";
}
