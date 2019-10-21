#ifndef DeclarationsInfoBase_H
#define DeclarationsInfoBase_H

#include "otbOGRDataSourceWrapper.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <fstream>

#include "CommonDefs.h"

#include "AttributeEntry.h"

class DeclarationsInfoBase {
public:
    DeclarationsInfoBase();
    // Allow subclasses to initialize the indexes for columns they might need in order to
    // perform a get field index at each operation
    virtual void InitializeIndexes(const AttributeEntry &firstOgrFeat);
    virtual int GetSeqId(const AttributeEntry &ogrFeat);
    virtual std::string GetOriId(const AttributeEntry &ogrFeat);
    virtual std::string GetOriCrop(const AttributeEntry &ogrFeat);
    
    virtual bool IsMonitoringParcel(const AttributeEntry &ogrFeat);

    inline std::string GetGeomValid(const AttributeEntry &ogrFeat) {return GetFieldOrNA(ogrFeat, m_GeomValidIdx);}
    inline std::string GetDuplic(const AttributeEntry &ogrFeat) {return GetFieldOrNA(ogrFeat, m_DuplicIdx);}
    inline std::string GetOverlap(const AttributeEntry &ogrFeat) {return GetFieldOrNA(ogrFeat, m_OverlapIdx);}
    inline std::string GetArea_meter(const AttributeEntry &ogrFeat) {return GetFieldOrNA(ogrFeat, m_Area_meterIdx);}
    inline std::string GetShapeInd(const AttributeEntry &ogrFeat) {return GetFieldOrNA(ogrFeat, m_ShapeIndIdx);}
    inline std::string GetCTnum(const AttributeEntry &ogrFeat) {return GetFieldOrNA(ogrFeat, m_CTnumIdx);}
    inline std::string GetCT(const AttributeEntry &ogrFeat) {return GetFieldOrNA(ogrFeat, m_CTIdx);}
    inline std::string GetLC(const AttributeEntry &ogrFeat) {return GetFieldOrNA(ogrFeat, m_LandCoverFieldIdx);}
    inline std::string GetS1Pix(const AttributeEntry &ogrFeat) {return GetFieldOrNA(ogrFeat, m_S1PixIdx);}
    inline std::string GetS2Pix(const AttributeEntry &ogrFeat) {return GetFieldOrNA(ogrFeat, m_S2PixIdx);}

protected:

    std::string GetFieldOrNA(const AttributeEntry &ogrFeat, int idx);

protected:
    int m_SeqIdFieldIdx;
    int m_OrigIdFieldIdx;
    int m_OrigCropFieldIdx;
    int m_LandCoverFieldIdx;

    int m_GeomValidIdx;
    int m_DuplicIdx;
    int m_OverlapIdx;
    int m_Area_meterIdx;
    int m_ShapeIndIdx;
    int m_CTnumIdx;
    int m_CTIdx;
    int m_S1PixIdx;
    int m_S2PixIdx;
};

#endif
