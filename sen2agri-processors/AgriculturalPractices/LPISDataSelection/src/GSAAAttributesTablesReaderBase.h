#ifndef GSAAAttributesTablesReaderBase_h
#define GSAAAttributesTablesReaderBase_h

#include <string>
#include <iostream>
#include <functional>

#include "AttributeEntry.h"

class GSAAAttributesTablesReaderBase
{
public:
    virtual void SetSource(const std::string &src) {m_source = src;}
    virtual bool ExtractAttributes(std::function<void (const AttributeEntry&)>) = 0;
    virtual std::string GetName() = 0;

protected:
    std::string m_source;
};

#endif
