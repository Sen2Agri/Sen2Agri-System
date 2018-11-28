#include "stopwatch.hpp"

#include <utility>

#include "stopwatch.hpp"
#include "logger.hpp"

Stopwatch::Stopwatch(QString operation) : operation_(std::move(operation)) { timer_.start(); }

Stopwatch::~Stopwatch()
{
    auto elapsed = timer_.elapsed();
    if (elapsed > 100) {
        Logger::warn(QStringLiteral("%1 took %2 ms").arg(operation_).arg(timer_.elapsed()));
    }
}
