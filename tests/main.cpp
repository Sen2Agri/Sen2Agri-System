#include <QCoreApplication>
#include <QTest>

#include "testqstring.hpp"
#include "serialization.hpp"
#include "schedulertests.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Q_UNUSED(app);

    int r = 0;

    Serialization tcSer;
    r |= QTest::qExec(&tcSer, argc, argv);

    SchedulerTests tcSch;
    r |= QTest::qExec(&tcSch, argc, argv);

    return r ? 1 : 0;
}
