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

template <typename F>
class AsyncDBusTaskNoResult : public QRunnable
{
    QDBusMessage message;
    QDBusConnection &connection;
    F f;

public:
    AsyncDBusTaskNoResult(QDBusMessage message, QDBusConnection &connection, F &&f)
        : message(std::move(message)), connection(connection), f(std::forward<F>(f))
    {
    }

    void run()
    {
        try {
            f();
            connection.send(message.createReply());
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

template <typename M, typename C, typename F>
AsyncDBusTaskNoResult<F> *makeAsyncDBusTaskNoResult(M &&message, C &&connection, F &&f)
{
    return new AsyncDBusTaskNoResult<F>(std::forward<M>(message), std::forward<C>(connection),
                                        std::forward<F>(f));
}
