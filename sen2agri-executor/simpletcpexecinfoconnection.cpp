#include "simpletcpexecinfoconnection.h"

SimpleTcpExecInfoConnection::SimpleTcpExecInfoConnection(QTcpSocket *socket, QObject *parent)
    : QObject(parent)
{
    connect(socket, &QTcpSocket::readyRead, this, &SimpleTcpExecInfoConnection::handleData);
    connect(socket, &QTcpSocket::disconnected, this, &SimpleTcpExecInfoConnection::handleDisconnect);
}

void SimpleTcpExecInfoConnection::handleData()
{
    auto socket = qobject_cast<QTcpSocket *>(sender());
    data.append(socket->readAll());
}

void SimpleTcpExecInfoConnection::handleDisconnect()
{
    emit message(data);

    auto socket = qobject_cast<QTcpSocket *>(sender());
    socket->deleteLater();

    deleteLater();
}
