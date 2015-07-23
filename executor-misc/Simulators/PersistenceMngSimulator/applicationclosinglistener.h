#ifndef APPLICATIONCLOSINGLISTENER_H
#define APPLICATIONCLOSINGLISTENER_H

#include <QObject>

#include "simulator.h"

class ApplicationClosingListener : public QObject
{
    Q_OBJECT
public:
    ApplicationClosingListener(QObject *parent = 0);

public slots:
    void run()
    {
        // Do processing here
        //emit finished();
    }
signals:
    void finished();

private:
    Simulator *m_pSimulator;
};

#endif // APPLICATIONCLOSINGLISTENER_H
