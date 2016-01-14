#include "ApplicationClosingListener.h"

#include <string>
#include <fstream>
#include <iostream>
using namespace std;

ApplicationClosingListener::ApplicationClosingListener(QObject *parent) : QObject(parent)
{
    m_pSimulator = new Simulator();
}

void ApplicationClosingListener::run()
{
    // Do processing here
    QTimer::singleShot(10000, this, SLOT(SendExecuteProcessor()));
    //QTimer::singleShot(12000, this, SLOT(SendCancelProcessor()));

    //emit finished();
}
void ApplicationClosingListener::SendExecuteProcessor()
{
    m_pSimulator->HandleSendExecuteProcessor();
}

void ApplicationClosingListener::SendCancelProcessor()
{
    m_pSimulator->HandleSendCancelProcessor();
    emit finished();
}
