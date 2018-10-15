#include "PracticeReaderFactory.h"

#include "PracticeCsvReader.h"
#include "PracticeShpReader.h"

std::unique_ptr<PracticeReaderBase> PracticeReaderFactory::GetPracticeReader(
        const std::string &name)
{
    std::unique_ptr<PracticeReaderBase> csvReader(new PracticeCsvReader);
    if (csvReader->GetName() == name) {
        return csvReader;
    }

    std::unique_ptr<PracticeReaderBase> shpReader(new PracticeShpReader);
    if (shpReader->GetName() == name) {
        return shpReader;
    }

    itkExceptionMacro("Practice reader not supported: " << name);

    return NULL;
}
