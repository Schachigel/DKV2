#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"
#include "../DKV2/booking.h"

#include "../DKV2/helper.h"
//#include "../DKV2/sqlhelper.h"
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
    QVERIFY(booking::makeDeposit(c.id(), deposit, QDate::currentDate()));

    QCOMPARE(bookings(c.id(), booking::type::deposit).sumBookings(), deposit);
    QVector<bookings::data> b = bookings(c.id(), booking::type::deposit).getBookings();
    QVERIFY( b.size() == 1);
    QVERIFY( b[0].amount == 1000.);
}

void test_booking::test_createDeposits()
{   LOG_CALL;
    double deposit = 1000.00;
    contract c = saveRandomContract(saveRandomCreditor().id());
    QVERIFY(booking::makeDeposit(c.id(), deposit, QDate::currentDate().addDays(-6)));
    QVERIFY(booking::makeDeposit(c.id(), deposit, QDate::currentDate().addDays(-5)));
    QVERIFY(booking::makeDeposit(c.id(), deposit, QDate::currentDate().addDays(-4)));
    QVERIFY(booking::makeDeposit(c.id(), deposit, QDate::currentDate().addDays(-3)));

    QCOMPARE(bookings(c.id(), booking::type::deposit).sumBookings(), 4.*deposit);
    QVector<bookings::data> b = bookings(c.id(), booking::type::deposit).getBookings();
    QVERIFY( b.size() == 4);
    for( int i = 0; i<4; i++)
        QVERIFY( b[i].amount == 1000.);
}
