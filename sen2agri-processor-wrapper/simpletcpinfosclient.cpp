#include <QDataStream>

#include "logger.hpp"
#include "simpletcpinfosclient.h"

bool SimpleTcpInfosClient::Initialize(const QString &strIpAddr, int nPortNo)
{
    m_hostAddrStr = strIpAddr;
    m_nPortNo = nPortNo;

    return true;
}

static void msleep(long msecs)
{
    if (msecs <= 0) {
        return;
    }
    QMutex mutex;
    mutex.lock();

    QWaitCondition waitCondition;
    waitCondition.wait(&mutex, msecs);

    mutex.unlock();
}

bool SimpleTcpInfosClient::SendMessage(const QString &strMsg)
{
    bool ret = SendMessage(m_hostAddrStr, strMsg);
    if (!ret) {
        for (int i = 0; i < m_nSendRetriesNo; i++) {
            msleep(m_nTimeoutBetweenRetries);
            ret = SendMessage(m_hostAddrStr, strMsg);
            // if successfully sent, then exit
            if (ret) {
                break;
            }
        }
    }
    if (!ret && (m_nExecutesLocal == 1)) {
        // try to send also to localhost but only once
        ret = SendMessage("127.0.0.1", strMsg);
    }

    return ret;
}

bool SimpleTcpInfosClient::SendMessage(const QString &strHostAddr, const QString &strMsg)
{
    m_socket.connectToHost(strHostAddr, m_nPortNo);
    if (m_socket.state() != QAbstractSocket::ConnectedState && !m_socket.waitForConnected()) {
        Logger::error(QStringLiteral("Unable to connect to %1:%2: %3")
                          .arg(m_hostAddrStr)
                          .arg(m_nPortNo)
                          .arg(m_socket.errorString()));
        return false;
    }

    const auto &ba = strMsg.toUtf8();
    auto start = ba.constData();
    for (auto size = ba.size(); size;) {
        auto len = m_socket.write(start, size);
        if (len <= 0) { // can write() return 0?
            Logger::error(QStringLiteral("Unable to send message: %1").arg(m_socket.errorString()));
            m_socket.disconnectFromHost();
            return false;
        }
        start += len;
        size -= len;
    }

    if (!m_socket.waitForBytesWritten()) {
        Logger::error(QStringLiteral("Unable to send message: %1").arg(m_socket.errorString()));
    }

    m_socket.disconnectFromHost();
    if (m_socket.state() != QAbstractSocket::UnconnectedState && !m_socket.waitForDisconnected()) {
        Logger::error(
            QStringLiteral("Unable to disconnect from host: %1").arg(m_socket.errorString()));
        return false;
    }

    return true;
}
