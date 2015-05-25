#pragma once

#include <QMetaType>
#include <QDBusArgument>

class ArchiverParameter
{
public:
    int processorId;
    int productId;
    int minAge;

    ArchiverParameter();
    ArchiverParameter(int processorId, int productId, int minAge);

    static void registerMetaTypes();
};

typedef QList<ArchiverParameter> ArchiverParameterList;

Q_DECLARE_METATYPE(ArchiverParameter)
Q_DECLARE_METATYPE(ArchiverParameterList)

QDBusArgument &operator<<(QDBusArgument &argument, const ArchiverParameter &message);
const QDBusArgument &operator>>(const QDBusArgument &argument, ArchiverParameter &message);
