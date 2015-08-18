#pragma once

#include <QTcpSocket>
#include <QByteArray>

class SimpleTcpExecInfoConnection : public QObject
{
    Q_OBJECT

    QByteArray data;

public:
    SimpleTcpExecInfoConnection(QTcpSocket *socket, QObject *parent = nullptr);

private slots:
    void handleData();
    void handleDisconnect();

signals:
    void message(const QByteArray &data);
};
