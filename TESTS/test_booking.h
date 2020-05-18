#ifndef TEST_BOOKING_H
#define TEST_BOOKING_H

#include <QSqlDatabase>
#include <QObject>

#include "testhelper.h"

class test_booking : public QObject
{
    Q_OBJECT
public:
    explicit test_booking(QObject* p=nullptr) : QObject(p){}
    ~test_booking(){}
private:
    // helper
signals:
private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    // the actual tests
    void test_createCreditor();

};

#endif // TEST_BOOKING_H
