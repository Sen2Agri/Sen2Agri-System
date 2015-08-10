#include "tinyxml_utils.hpp"

std::string GetAttribute(const TiXmlElement *element, const char *attributeName)
{
    if (const char *at = element->Attribute(attributeName)) {
        return at;
    }

    return std::string();
}

std::string GetText(const TiXmlElement *element)
{
    if (const char *text = element->GetText()) {
        return text;
    }

    return std::string();
}

std::string GetChildText(const TiXmlElement *element, const char *childName)
{
    if (auto el = element->FirstChildElement(childName)) {
        if (const char *text = el->GetText())
            return text;
    }

    return std::string();
}

std::string
 GetChildAttribute(const TiXmlElement *element, const char *childName, const char *attributeName)
{
    if (auto el = element->FirstChildElement(childName)) {
        if (const char *at = el->Attribute(attributeName)) {
            return at;
        }
    }

    return std::string();
}
