#ifndef CountryInfoBase_H
#define CountryInfoBase_H

#include "otbOGRDataSourceWrapper.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <fstream>

#include "CommonDefs.h"

#include "AttributeEntry.h"

typedef std::map<std::string, size_t> MapHdrIdx;

class CountryInfoBase {
public:
    CountryInfoBase();
    // Allow subclasses to initialize the indexes for columns they might need in order to
    // perform a get field index at each operation
    virtual void InitializeIndexes(const AttributeEntry &firstOgrFeat);
    virtual void SetAdditionalFiles(const std::vector<std::string> &additionalFiles);
    virtual std::string GetName() = 0;
    virtual std::string GetUniqueId(const AttributeEntry &ogrFeat) = 0;
    virtual int GetSeqId(const AttributeEntry &ogrFeat);
    
    virtual std::string GetMainCrop(const AttributeEntry &ogrFeat) = 0;
    virtual bool GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice) = 0;

    virtual void SetYear(const std::string &val);
    virtual void SetVegStart(const std::string &val);
    virtual void SetHStart(const std::string &val);
    virtual void SetHWinterStart(const std::string &val);
    virtual void SetHEnd(const std::string &val);
    virtual void SetPractice(const std::string &val);
    virtual void SetPStart(const std::string &val);
    virtual void SetPEnd(const std::string &val);
    virtual void SetWinterPStart(const std::string &val);
    virtual void SetWinterPEnd(const std::string &val);

    virtual std::string GetYear();
    virtual std::string GetVegStart();
    virtual std::string GetHStart(const AttributeEntry &ogrFeat);
    virtual std::string GetHEnd(const AttributeEntry &ogrFeat);
    virtual std::string GetPractice();
    virtual std::string GetPractice(const AttributeEntry &ogrFeat);
    virtual std::string GetPracticeType(const AttributeEntry &ogrFeat);
    virtual std::string GetPStart(const AttributeEntry &ogrFeat);
    virtual std::string GetPEnd(const AttributeEntry &ogrFeat);

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

private:

    std::string GetFieldOrNA(const AttributeEntry &ogrFeat, int idx);

    void ParseCsvFile(const std::string &filePath,
                      std::function<int(const MapHdrIdx&, const std::vector<std::string>&, int)> fnc);

    void ParseShpFile(const std::string &filePath,
                      std::function<int(OGRFeature&, int)> fnc);

protected:
    std::vector<std::string> GetInputFileLineElements(const std::string &line);

protected:
    std::string m_year;
    std::string m_vegstart;
    std::string m_hstart;
    std::string m_hend;
    std::string m_hWinterStart;
    std::string m_practice;
    std::string m_pstart;
    std::string m_pend;
    std::string m_pWinterStart;
    std::string m_pWinterEnd;
    std::string m_ptype;

    std::vector<std::string> m_additionalFiles;

    int m_SeqIdFieldIdx;
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

    std::function<int(const MapHdrIdx&, const std::vector<std::string>&, int)> m_LineHandlerFnc;
    std::function<int(OGRFeature&, int)> m_ShpFeatHandlerFnc;
};

#endif
