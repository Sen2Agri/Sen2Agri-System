#pragma once

#include <QString>
#include <QElapsedTimer>

class Stopwatch
{
    QElapsedTimer timer_;
    QString operation_;

public:
    Stopwatch(QString operation);
    ~Stopwatch();
};

#define TOKENPASTE(x, y) x##y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)

#define START_STOPWATCH(message)                                                                   \
    Stopwatch TOKENPASTE2(sw, __LINE__)(QStringLiteral(message));                                  \
    Q_UNUSED(TOKENPASTE2(sw, __LINE__))
