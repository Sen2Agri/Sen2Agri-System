#pragma once

#include <string>

#include "otb_tinyxml.h"

std::string GetAttribute(const TiXmlElement *element, const char *attributeName);
std::string GetText(const TiXmlElement *element);
std::string GetChildText(const TiXmlElement *element, const char *childName);
std::string GetChildAttribute(const TiXmlElement *element, const char *childName, const char *attributeName);
