#pragma once

#include <QJsonDocument>

#include <model.hpp>

class SerializedEvent
{
public:
    EventType type;
    QJsonDocument data;

    SerializedEvent(EventType type, QJsonDocument data);

    SerializedEvent(const TaskFinishedEvent &event);
    SerializedEvent(const ProductAvailableEvent &event);
    SerializedEvent(const JobCancelledEvent &event);
    SerializedEvent(const JobPausedEvent &event);
    SerializedEvent(const JobResumedEvent &event);
    SerializedEvent(const JobSubmittedEvent &event);
};
