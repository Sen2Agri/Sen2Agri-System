#ifndef PROCESSORWRAPPERFACTORY_H
#define PROCESSORWRAPPERFACTORY_H

#include <QString>

#include <map>
using namespace std;

/**
 * @brief The ProcessorWrapperFactory class
 * This class is in charge with the creation of the processor wrappers.
 */
class ProcessorWrapperFactory
{
public:
    ~ProcessorWrapperFactory();

    static ProcessorWrapperFactory *GetInstance();
    bool GetProcessorPath(QString &processorName, QString &retCmd);
    bool GetProcessorDescription(QString &processorName, QString &retDescr);
    void Register(QString &procName, QString &procCmd, QString& descr);

private:
    ProcessorWrapperFactory();

    class ProcessorInfos {
    public:
        ProcessorInfos() { }

        ProcessorInfos(QString &strName, QString &strCmd, QString &strDescr) {
            name = strName;
            command = strCmd;
            description = strDescr;
        }

        ProcessorInfos &operator=(const ProcessorInfos &procInfos) {
            if (this != &procInfos) {
                this->name = procInfos.name;
                this->command = procInfos.command;
                this->description = procInfos.description;
            }
            return *this;
        }

        QString name;
        QString command;
        QString description;
    } ;

    typedef map<QString, ProcessorInfos> ProcessorsInfosMap;
    ProcessorsInfosMap m_ProcessorsInfosMap;
};

#endif // PROCESSORWRAPPERFACTORY_H
