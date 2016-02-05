#ifndef PROCESSORHANDLERHELPER_H
#define PROCESSORHANDLERHELPER_H


class ProcessorHandlerHelper
{
public:
    ProcessorHandlerHelper();

    static QString GetTileId(const QStringList &xmlFileNames);
};

#endif // PROCESSORHANDLERHELPER_H
