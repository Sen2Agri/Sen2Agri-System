#pragma once

#include <QString>
#include <QDBusArgument>
#include <QMetaType>

class KeyedMessage
{
public:
    QString key;
    QString text;

    KeyedMessage();
    KeyedMessage(QString key, QString text);

    static void registerMetaTypes();
};

typedef QList<KeyedMessage> KeyedMessageList;

Q_DECLARE_METATYPE(KeyedMessage)
Q_DECLARE_METATYPE(KeyedMessageList)

QDBusArgument &operator<<(QDBusArgument &argument, const KeyedMessage &message);
const QDBusArgument &operator>>(const QDBusArgument &argument, KeyedMessage &message);

QString toJson(const KeyedMessageList &parameters);
