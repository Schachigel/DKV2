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

void test_booking::test_isInterestBooking()
{
    QVERIFY( ! booking::isInterestBooking(booking::Type::non));
    QVERIFY( ! booking::isInterestBooking(booking::Type::deposit));
    QVERIFY( ! booking::isInterestBooking(booking::Type::payout));
    QVERIFY( booking::isInterestBooking(booking::Type::interestDeposit));
    QVERIFY( booking::isInterestBooking(booking::Type::interestPayout));
}

void test_booking::test_dateONSettlement_noContracts()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    QCOMPARE( QDate(), bookings::dateOfnextSettlement());
}
void test_booking::test_dateONSettlement_activatedContracts()
{
    creditor c(saveRandomCreditor());
    contract cont2(saveRandomContract(c.id()));
    cont2.activate(1000., QDate( 2000,12,31));
    QCOMPARE( bookings::dateOfnextSettlement(), QDate( 2001, 1, 1));

    contract cont(saveRandomContract(c.id()));
    cont.activate(1000., QDate( 2000, 1, 1));
    QCOMPARE( bookings::dateOfnextSettlement(), QDate( 2001, 1, 1));

    contract cont3(saveRandomContract(c.id()));
    cont3.activate(1000., QDate( 1999,12,31));
    QCOMPARE( bookings::dateOfnextSettlement(), QDate( 2000, 1, 1));
}

void test_booking::test_dateONSettelment_contractsW_interestBookings00()
{
    creditor cred(saveRandomCreditor());
    contract cont(saveRandomContract(cred.id()));
    cont.setInterest100th(100);
    cont.activate(1000., QDate( 2000,6,1));
    booking::investInterest(cred.id(), QDate(2001,1,1), 5.);
    QCOMPARE( bookings::dateOfnextSettlement(), QDate( 2002, 1, 1));
}

void test_booking::test_dateONSettelment_contractsW_and_wo_interestBookings01()
{
    creditor cred(saveRandomCreditor());
    contract cont(saveRandomContract(cred.id()));
    cont.setInterest100th(100);
    cont.activate(1000., QDate( 2000,6,1));
    booking::investInterest(cred.id(), QDate(2001,1,1), 5.);

    contract cont2(saveRandomContract(cred.id()));
    cont2.activate(1000., QDate(2000, 6,1)); // EARLIER

    QCOMPARE( bookings::dateOfnextSettlement(), QDate( 2001, 1, 1));
}

void test_booking::test_dateONSettelment_contractsW_and_wo_interestBookings02()
{
    creditor cred(saveRandomCreditor());
    contract cont(saveRandomContract(cred.id()));
    cont.setInterest100th(100);
    cont.activate(1000., QDate( 2000,6,1));
    booking::investInterest(cred.id(), QDate(2001,1,1), 5.); // EARLIER

    contract cont2(saveRandomContract(cred.id()));
    cont2.activate(1000., QDate(2002, 6,1));

    QCOMPARE( bookings::dateOfnextSettlement(), QDate( 2002, 1, 1));
}

void test_booking::test_dateONSettelment_contractsW_and_wo_interestBookings03()
{
    {
        dbgTimer timer("create many contracts");
        saveRandomCreditors(70);
        saveRandomContracts(100);    // contract date: 2 years back or less
        activateRandomContracts(90);// activation date: > contract date
    }
    {
        dbgTimer timer("calculate settlement year");
        QCOMPARE( bookings::dateOfnextSettlement(), QDate(QDate::currentDate().addYears(-1).year(), 1, 1));
    }
}

void test_booking::test_getBookings()
{
//    creditor c(saveRandomCreditor());
//    contract cont(saveRandomContract(c.id()));
//    QDate aDate = QDate(2020, 5, 1);
//    cont.activate(1000., aDate);
//    cont.deposit(1000., aDate.addMonths(6));
//    booking::investInterest(cont.id(), QDate(2021, 1, 1), 10.);
//    cont.deposit(1000., QDate(2021, 2, 1));
//    booking::payoutInterest(cont.id(), QDate(2022, 1, 1), 10.);
//    cont.deposit(1000., QDate(2022, 2, 1));

//    QVector<booking> v=bookings::getBookings(cont.id(), QDate(2020, 5, 1));
//    QCOMPARE(v.size(), 6);

//    QCOMPARE(v[5].type, booking::Type::deposit);
//    QCOMPARE(v[4].type, booking::Type::deposit);
//    QCOMPARE(v[3].type, booking::Type::interestDeposit);
//    QCOMPARE(v[2].type, booking::Type::deposit);
//    QCOMPARE(v[1].type, booking::Type::interestPayout);
//    QCOMPARE(v[0].type, booking::Type::deposit);

//    QCOMPARE(v[5].date, aDate);
//    QCOMPARE(v[4].date, aDate.addMonths(6));
//    QCOMPARE(v[3].date, QDate(2021, 1, 1));
//    QCOMPARE(v[2].date, QDate(2021, 2, 1));
//    QCOMPARE(v[1].date, QDate(2022, 1, 1));
//    QCOMPARE(v[0].date, QDate(2022, 2, 1));

//    QCOMPARE(v[5].amount, 1000.);
//    QCOMPARE(v[4].amount, 1000.);
//    QCOMPARE(v[3].amount, 10.);
//    QCOMPARE(v[2].amount, 1000.);
//    QCOMPARE(v[0].amount, 1000.);
//    QCOMPARE(v[1].amount, -10);


}
