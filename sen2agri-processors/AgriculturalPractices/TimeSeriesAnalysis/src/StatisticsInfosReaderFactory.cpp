#include "StatisticsInfosReaderFactory.h"
#include "StatisticsInfosFolderFilesReader.h"
#include "StatisticsInfosXmlReader.h"
#include "StatisticsInfosSingleCsvReader.h"

std::unique_ptr<StatisticsInfosReaderBase> StatisticsInfosReaderFactory::GetInfosReader(const std::string &name)
{
    std::unique_ptr<StatisticsInfosReaderBase> foldersReader(new StatisticsInfosFolderFilesReader);
    if (foldersReader->GetName() == name) {
        return foldersReader;
    }

    std::unique_ptr<StatisticsInfosReaderBase> xmlReader(new StatisticsInfosXmlReader);
    if (xmlReader->GetName() == name) {
        return xmlReader;
    }

    std::unique_ptr<StatisticsInfosReaderBase> csvReader(new StatisticsInfosSingleCsvReader);
    if (csvReader->GetName() == name) {
        return csvReader;
    }

    itkExceptionMacro("Statistics infos reader not supported: " << name);

    return NULL;
}
