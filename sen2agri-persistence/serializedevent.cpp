#include "serializedevent.hpp"

SerializedEvent::SerializedEvent(EventType type, QJsonDocument data)
    : type(type), data(std::move(data))
{
}

SerializedEvent::SerializedEvent(const TaskFinishedEvent &event)
    : type(EventType::TaskFinished), data(event.toJson())
{
}

SerializedEvent::SerializedEvent(const ProductAvailableEvent &event)
    : type(EventType::ProductAvailable), data(event.toJson())
{
}

SerializedEvent::SerializedEvent(const JobCancelledEvent &event)
    : type(EventType::JobCancelled), data(event.toJson())
{
}

SerializedEvent::SerializedEvent(const JobPausedEvent &event)
    : type(EventType::JobPaused), data(event.toJson())
{
}

SerializedEvent::SerializedEvent(const JobResumedEvent &event)
    : type(EventType::JobResumed), data(event.toJson())
{
}

SerializedEvent::SerializedEvent(const JobSubmittedEvent &event)
    : type(EventType::JobSubmitted), data(event.toJson())
{
}
