#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"
#include "../DKV2/booking.h"

#include "../DKV2/helper.h"
#include "../DKV2/dkdbhelper.h"

#include "test_booking.h"

void test_booking::initTestCase()
{   LOG_CALL;
    init_DKDBStruct();
}

void test_booking::init()
{   LOG_CALL;
    initTestDb();
    create_DK_TablesAndContent();
}
void test_booking::cleanup()
{   LOG_CALL;
    cleanupTestDb();
}

void test_booking::test_createBooking()
{   LOG_CALL;
    double deposit = 1000.00;
    contract c = saveRandomContract(saveRandomCreditor().id());
    QVERIFY(booking::makeDeposit(c.id(), QDate::currentDate(), deposit));

    QCOMPARE(bookings(c.id(), booking::Type::deposit).sumBookings(), deposit);
    QVector<booking> b = bookings(c.id(), booking::Type::deposit).getBookings();
    QVERIFY( b.size() == 1);
    QVERIFY( b[0].amount == 1000.);
}

void test_booking::test_createDeposits()
{   LOG_CALL;
    double deposit = 1000.00;
    contract c = saveRandomContract(saveRandomCreditor().id());
    QVERIFY(booking::makeDeposit(c.id(), QDate::currentDate().addDays(-6), deposit));
    QVERIFY(booking::makeDeposit(c.id(), QDate::currentDate().addDays(-5), deposit));
    QVERIFY(booking::makeDeposit(c.id(), QDate::currentDate().addDays(-4), deposit));
    QVERIFY(booking::makeDeposit(c.id(), QDate::currentDate().addDays(-3), deposit));

    QCOMPARE(bookings(c.id(), booking::Type::deposit).sumBookings(), 4.*deposit);
    QVector<booking> b = bookings(c.id(), booking::Type::deposit).getBookings();
    QVERIFY( b.size() == 4);
    for( int i = 0; i<4; i++)
        QVERIFY( b[i].amount == 1000.);
}

void test_booking::test_randomActivations()
{
    int count = 90;
    dbgTimer t(QString::number(count) + " contracts");
    saveRandomCreditors(50);
    saveRandomContracts(count);
    activateRandomContracts(90);
}
