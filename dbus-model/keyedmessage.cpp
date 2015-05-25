#include <utility>

#include <QDBusMetaType>
#include <QJsonDocument>
#include <QJsonObject>

#include "keyedmessage.hpp"

using std::move;

KeyedMessage::KeyedMessage()
{
}

KeyedMessage::KeyedMessage(QString key, QString message) : key(move(key)), text(move(message))
{
}

void KeyedMessage::registerMetaTypes()
{
    qRegisterMetaType<KeyedMessage>("KeyedMessage");
    qRegisterMetaType<KeyedMessageList>("KeyedMessageList");

    qDBusRegisterMetaType<KeyedMessage>();
    qDBusRegisterMetaType<KeyedMessageList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const KeyedMessage &message)
{
    argument.beginStructure();
    argument << message.key << message.text;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, KeyedMessage &message)
{
    argument.beginStructure();
    argument >> message.key >> message.text;
    argument.endStructure();

    return argument;
}
