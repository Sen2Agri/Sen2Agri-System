#ifndef APPLICATIONCLOSINGLISTENER_H
#define APPLICATIONCLOSINGLISTENER_H

#include <QObject>

class ApplicationClosingListener : public QObject
{
    Q_OBJECT
public:
    ApplicationClosingListener(QObject *parent = 0) : QObject(parent) {}

public slots:
    void run()
    {
        // Do processing here

        emit finished();
    }

signals:
    void finished();
};

#endif // APPLICATIONCLOSINGLISTENER_H
