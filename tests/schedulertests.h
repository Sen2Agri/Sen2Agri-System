#ifndef SCHEDULERTESTS_H
#define SCHEDULERTESTS_H

#include <QObject>

class SchedulerTests : public QObject
{
    Q_OBJECT
public:
    explicit SchedulerTests(QObject *parent = 0);

signals:

public slots:

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testNoReadyTasks();
    void testFirstReadyTask();
    void testSecondRetryTask();
    void testPriority();
    void testEstimationTime();
};


template <typename T>
void compare(const QList<T> &v1, const QList<T> &v2)
{
    QCOMPARE(v1.count(), v2.count());

    auto n = v1.count();
    for (int i = 0; i < n; i++) {
        compare(v1.at(i), v2.at(i));
    }
}

template <typename T>
void compare(const T &v1, const T &v2)
{
    QCOMPARE(v1, v2);
}

#endif // SCHEDULERTESTS_H
