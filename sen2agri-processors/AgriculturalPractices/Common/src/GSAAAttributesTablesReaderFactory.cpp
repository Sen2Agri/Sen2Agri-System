#include "GSAAAttributesTablesReaderFactory.h"

#include "GSAACsvAttributesTablesReader.h"
#include "GSAAShpAttributesTablesReader.h"

std::unique_ptr<GSAAAttributesTablesReaderBase> GSAAAttributesTablesReaderFactory::GetPracticeReader(
        const std::string &name)
{
    std::unique_ptr<GSAAAttributesTablesReaderBase> csvReader(new GSAACsvAttributesTablesReader);
    if (csvReader->GetName() == name) {
        return csvReader;
    }

    std::unique_ptr<GSAAAttributesTablesReaderBase> shpReader(new GSAAShpAttributesTablesReader);
    if (shpReader->GetName() == name) {
        return shpReader;
    }

    itkExceptionMacro("GSAA Attributes table reader not supported: " << name);

    return NULL;
}
