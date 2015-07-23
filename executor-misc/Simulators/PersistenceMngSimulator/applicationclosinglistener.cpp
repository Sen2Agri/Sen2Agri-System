#include "ApplicationClosingListener.h"

#include <string>
#include <fstream>
#include <iostream>
using namespace std;

ApplicationClosingListener::ApplicationClosingListener(QObject *parent) : QObject(parent)
{
    m_pSimulator = new Simulator();
}

