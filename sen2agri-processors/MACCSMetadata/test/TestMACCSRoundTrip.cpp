#include <cassert>
#include <sstream>

#include "otb_tinyxml.h"
#include "MACCSMetadataReader.hpp"
#include "MACCSMetadataWriter.hpp"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        return EXIT_FAILURE;
    }

    auto reader = itk::MACCSMetadataReader::New();
    auto writer = itk::MACCSMetadataWriter::New();

    TiXmlDocument doc;
    if (!doc.LoadFile(argv[1]))
    {
        return EXIT_FAILURE;
    }

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
