#pragma once

#include <utility>

#include <QRunnable>
#include <QDBusMessage>
#include <QDBusConnection>

#include "type_traits_ext.hpp"
#include "logger.hpp"

template <typename F>
class AsyncDBusTask : public QRunnable
{
    QDBusMessage message;
    QDBusConnection connection;
    F f;

    template <typename Func>
    static auto createResponseValue(Func &&f)
        -> enable_if_t<!is_void_t<decltype(f())>::value, QVariantList>
    {
        return QVariantList() << qVariantFromValue(f());
    }

    template <typename Func>
    static auto createResponseValue(Func &&f)
        -> enable_if_t<is_void_t<decltype(f())>::value, QVariantList>
    {
        f();
        return QVariantList();
    }

public:
    template <typename Func>
    AsyncDBusTask(QDBusMessage message, QDBusConnection connection, Func &&f)
        : message(std::move(message)), connection(connection), f(std::forward<Func>(f))
    {
    }

    void run()
    {
        try {
            auto replyMessage = message.createReply();
            replyMessage.setArguments(createResponseValue(std::forward<F>(f)));
            connection.send(replyMessage);
        } catch (const std::exception &e) {
            Logger::error(e.what());

            connection.send(message.createErrorReply(QDBusError::Failed, e.what()));
        }
    }
};

template <typename M, typename C, typename F>
AsyncDBusTask<F> *makeAsyncDBusTask(M &&message, C &&connection, F &&f)
{
    // QThreadPool will free the task after it runs
    return new AsyncDBusTask<F>(std::forward<M>(message), std::forward<C>(connection),
                                std::forward<F>(f));
}
