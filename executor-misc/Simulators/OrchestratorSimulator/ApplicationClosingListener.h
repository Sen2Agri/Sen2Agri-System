#ifndef APPLICATIONCLOSINGLISTENER_H
#define APPLICATIONCLOSINGLISTENER_H

#include <QObject>
#include <QTimer>
#include "simulator.h"

class ApplicationClosingListener : public QObject
{
    Q_OBJECT
public:
    ApplicationClosingListener(QObject *parent = 0);

public slots:
    void run();
    void SendExecuteProcessor();
    void SendCancelProcessor();

signals:
    void finished();

private:
    Simulator *m_pSimulator;
};

#endif // APPLICATIONCLOSINGLISTENER_H
