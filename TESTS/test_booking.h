#ifndef TEST_BOOKING_H
#define TEST_BOOKING_H

#include <QSqlDatabase>
#include <QObject>
#include <QTest>

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
    void init();
    void cleanup();
    // the actual tests
    void test_dateONSettlement_noContracts();
    void test_dateONSettlement_activatedContracts();
    void test_dateONSettelment_contractsW_interestBookings00();
    void test_dateONSettelment_contractsW_and_wo_interestBookings01();
    void test_dateONSettelment_contractsW_and_wo_interestBookings02();
    void test_dateONSettelment_contractsW_and_wo_interestBookings03();
};

#endif // TEST_BOOKING_H
