#include <QCoreApplication>
#include <QTest>

#include "testqstring.hpp"
#include "serialization.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Serialization tc;
    return QTest::qExec(&tc, argc, argv);
}
