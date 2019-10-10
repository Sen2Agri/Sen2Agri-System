#ifndef AttributeEntry_h
#define AttributeEntry_h

#include <string>

class AttributeEntry
{
public:
    virtual int GetFieldIndex(const char *pszName) const = 0;
    virtual const char* GetFieldAsString(int idx) const = 0;
    virtual double GetFieldAsDouble(int idx) const = 0;
    virtual int GetFieldAsInteger(int idx) const = 0;


};

#endif
