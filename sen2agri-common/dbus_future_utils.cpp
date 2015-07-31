#include "dbus_future_utils.hpp"

void throw_on_dbus_error(const QDBusPendingCall &promise)
{
    if (promise.isError()) {
        throw std::runtime_error(promise.error().message().toStdString());
    }
}

void WaitForResponseAndThrow(QDBusPendingReply<> promise)
{
    promise.waitForFinished();
    throw_on_dbus_error(promise);
}
