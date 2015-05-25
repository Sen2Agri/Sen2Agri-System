#include "archiverparameter.hpp"

#include <QDBusMetaType>

ArchiverParameter::ArchiverParameter()
    : processorId(), productId(), minAge()
{
}

ArchiverParameter::ArchiverParameter(int processorId, int productId, int minAge)
    : processorId(processorId), productId(productId), minAge(minAge)
{
}

void ArchiverParameter::registerMetaTypes()
{
    qRegisterMetaType<ArchiverParameter>("ArchiverParameter");
    qRegisterMetaType<ArchiverParameterList>("ArchiverParameterList");

    qDBusRegisterMetaType<ArchiverParameter>();
    qDBusRegisterMetaType<ArchiverParameterList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const ArchiverParameter &parameter)
{
    argument.beginStructure();
    argument << parameter.processorId << parameter.productId << parameter.minAge;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ArchiverParameter &parameter)
{
    argument.beginStructure();
    argument >> parameter.processorId >> parameter.productId >> parameter.minAge;
    argument.endStructure();

    return argument;
}
