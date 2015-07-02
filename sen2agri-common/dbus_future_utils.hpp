#pragma once

#include <QDBusPendingCall>
#include <QDBusPendingReply>

void throw_on_dbus_error(const QDBusPendingCall &promise);
void WaitForResponseAndThrow(QDBusPendingReply<> promise);

template <typename T>
T WaitForResponseAndThrow(QDBusPendingReply<T> promise)
{
    promise.waitForFinished();
    throw_on_dbus_error(promise);

    return promise.value();
}
