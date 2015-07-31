#include <QCoreApplication>
#include <QTest>

#include "testqstring.hpp"
#include "serialization.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Q_UNUSED(app);

    Serialization tc;
    return QTest::qExec(&tc, argc, argv);
}
