#ifndef PracticeReaderBase_h
#define PracticeReaderBase_h

#include <string>
#include <iostream>
#include <functional>

class FeatureDescription
{
public:
    virtual std::string GetFieldId() const = 0;
    virtual std::string GetFieldSeqId() const = 0;
    virtual std::string GetMainCrop() const = 0;
    virtual std::string GetCountryCode() const = 0;
    virtual std::string GetYear() const = 0;
    virtual std::string GetVegetationStart() const = 0;
    virtual std::string GetHarvestStart() const = 0;
    virtual std::string GetHarvestEnd() const = 0;
    virtual std::string GetPractice() const = 0;
    virtual std::string GetPracticeType() const = 0;
    virtual std::string GetPracticeStart() const = 0;
    virtual std::string GetPracticeEnd() const = 0;
    virtual std::string GetS1Pix() const = 0;
};

class PracticeReaderBase
{
public:
    virtual void SetSource(const std::string &src) {m_source = src;}
    virtual bool ExtractFeatures(std::function<bool (const FeatureDescription&)>) = 0;
    virtual std::string GetName() = 0;

protected:
    std::string m_source;
};

#endif
