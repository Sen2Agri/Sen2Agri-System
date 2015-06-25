#pragma once

#include <QObject>
#include <QDBusContext>

class Orchestrator : public QObject, protected QDBusContext
{
    Q_OBJECT
public:
    explicit Orchestrator(QObject *parent = 0);

signals:

public slots:
};
