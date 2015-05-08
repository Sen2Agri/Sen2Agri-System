#pragma once

#include <utility>

#include <QRunnable>
#include <QDBusMessage>
#include <QDBusConnection>

template <typename F>
class AsyncDBusTask : public QRunnable
{
    QDBusMessage message;
    QDBusConnection &connection;
    F f;

public:
    AsyncDBusTask(QDBusMessage message, QDBusConnection &connection, F &&f)
        : message(std::move(message)), connection(connection), f(std::forward<F>(f))
    {
    }

    void run()
    {
        try {
            connection.send(message.createReply(qVariantFromValue(f())));
        } catch (const std::exception &e) {
            connection.send(message.createErrorReply(QDBusError::Failed, e.what()));
        }
    }
};

template <typename M, typename C, typename F>
AsyncDBusTask<F> *makeAsyncDBusTask(M &&message, C &&connection, F &&f)
{
    return new AsyncDBusTask<F>(std::forward<M>(message), std::forward<C>(connection),
                                std::forward<F>(f));
}
