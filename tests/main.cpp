#include <QCoreApplication>
#include <QTest>

#include "testqstring.hpp"
#include "serialization.hpp"
#include "schedulertests.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Q_UNUSED(app);

    //Serialization tc;
    SchedulerTests tc;
    return QTest::qExec(&tc, argc, argv);
}
