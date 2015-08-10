#include <cassert>
#include <sstream>

#include "otb_tinyxml.h"
#include "MACCSMetadataReader.hpp"
#include "MACCSMetadataWriter.hpp"

int main(int argc, char *argv[])
{
    assert(argc == 2);

    auto reader = itk::MACCSMetadataReader::New();
    auto writer = itk::MACCSMetadataWriter::New();

    TiXmlDocument doc;
    assert(doc.LoadFile(argv[1]));
    TiXmlUnknown unk;
    std::istringstream("<?xml-stylesheet type=\"text/xsl\" href=\"DISPLAY/display.xsl\"?>") >> unk;
    doc.ReplaceChild(doc.FirstChild()->NextSibling(), unk);

    std::ostringstream original, ours;

    original << doc;
    auto m = reader->ReadMetadataXml(doc);
    assert(m);
    ours << writer->CreateMetadataXml(*m);

    std::cerr << original.str() << '\n' << ours.str() << '\n';
    assert(original.str() == ours.str());
}
