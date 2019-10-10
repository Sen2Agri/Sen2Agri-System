#ifndef GSAAShpAttributesTablesReader_h
#define GSAAShpAttributesTablesReader_h

#include "otbOGRDataSourceWrapper.h"

#include "GSAAAttributesTablesReaderBase.h"

class GSAAShpAttributesTablesReader : public GSAAAttributesTablesReaderBase
{
public:
    virtual std::string GetName() {return "shp";}
    virtual bool ExtractAttributes(std::function<void (const AttributeEntry&)> fnc);

private:
    class OgrFeatureDescription : public AttributeEntry
    {
        OgrFeatureDescription() {
        }
        virtual int GetFieldIndex(const char *pszName) const;
        virtual const char *GetFieldAsString(int idx) const;
        virtual double GetFieldAsDouble(int idx) const;
        virtual int GetFieldAsInteger(int idx) const;

    private:
        otb::ogr::Layer::const_iterator featIt;

        friend class GSAAShpAttributesTablesReader;
    };
};

#endif
