#ifndef CountryInfoBase_H
#define CountryInfoBase_H

#include "otbOGRDataSourceWrapper.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <fstream>

#include "CommonDefs.h"

#include "../../Common/include/AttributeEntry.h"
#include "../../Common/include/DeclarationsInfoBase.h"

struct ci_less
{
    // case-independent (ci) compare_less binary function
    struct nocase_compare
    {
      bool operator() (const unsigned char& c1, const unsigned char& c2) const {
          return tolower (c1) < tolower (c2);
      }
    };
    bool operator() (const std::string & s1, const std::string & s2) const {
      return std::lexicographical_compare
        (s1.begin (), s1.end (),   // source range
        s2.begin (), s2.end (),   // dest range
        nocase_compare ());  // comparison
    }
};

typedef std::map<std::string, size_t, ci_less> MapHdrIdx;

class CountryInfoBase : public DeclarationsInfoBase {
public:
    CountryInfoBase();
    virtual std::string GetName() = 0;
    virtual bool IsMonitoringParcel(const AttributeEntry &ogrFeat);
    // Allow subclasses to initialize the indexes for columns they might need in order to
    // perform a get field index at each operation
    virtual void SetAdditionalFiles(const std::vector<std::string> &additionalFiles);
    
    virtual std::string GetMainCrop(const AttributeEntry &ogrFeat) = 0;
    virtual bool GetHasPractice(const AttributeEntry &ogrFeat, const std::string &practice) = 0;

    virtual void SetYear(const std::string &val);
    virtual void SetVegStart(const std::string &val);
    virtual void SetHStart(const std::string &val);
    virtual void SetHWinterStart(const std::string &val);
    virtual void SetHEnd(const std::string &val);
    virtual void SetHWinterEnd(const std::string &val);
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

private:

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
    std::string m_hWinterEnd;
    std::string m_practice;
    std::string m_pstart;
    std::string m_pend;
    std::string m_pWinterStart;
    std::string m_pWinterEnd;
    std::string m_ptype;

    std::vector<std::string> m_additionalFiles;

    std::function<int(const MapHdrIdx&, const std::vector<std::string>&, int)> m_LineHandlerFnc;
    std::function<int(OGRFeature&, int)> m_ShpFeatHandlerFnc;
};

#endif
