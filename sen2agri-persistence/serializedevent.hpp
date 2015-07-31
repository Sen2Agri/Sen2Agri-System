#pragma once

#include <QJsonDocument>

#include "model.hpp"

class SerializedEvent
{
public:
    EventType type;
    QString data;

    SerializedEvent(EventType type, QString data);

    SerializedEvent(const TaskRunnableEvent &event);
    SerializedEvent(const TaskFinishedEvent &event);
    SerializedEvent(const ProductAvailableEvent &event);
    SerializedEvent(const JobCancelledEvent &event);
    SerializedEvent(const JobPausedEvent &event);
    SerializedEvent(const JobResumedEvent &event);
    SerializedEvent(const JobSubmittedEvent &event);
};
