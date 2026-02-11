#include "test_booking.h"

#include "testhelper.h"
#include "../DKV2/helper_core.h"

#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"
#include "../DKV2/booking.h"

#include <QtTest/QTest>

void test_booking::init()
{   LOG_CALL;
    initTestDkDb_InMemory ();
}
void test_booking::cleanup()
{   LOG_CALL;
    cleanupTestDb_InMemory ();
}

void test_booking::test_defaults()
{
    {
        booking b;
        QCOMPARE(b.amount, 0.);
        QCOMPARE(b.contId, Invalid_contract_id);
        QCOMPARE(b.type, bookingType::non);
        QCOMPARE(b.date, EndOfTheFuckingWorld);
    }
    {
        booking b(contractId_t{13}, bookingType::annualInterestDeposit, QDate::currentDate (), 42.);
        QCOMPARE(b.amount, 42.);
        QCOMPARE(b.contId.v, 13);
        QCOMPARE(b.type, bookingType::annualInterestDeposit);
        QCOMPARE(b.date, QDate::currentDate ());
    }
}

void test_booking::test_bookDeposit()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    QDate validDate (2020, 4, 28);
    double amount {111.11};
    {
        QVERIFY( not bookDeposit(cont.id(), validDate, -1 *amount));
        QDate invalidDate;
        QVERIFY( not bookDeposit(cont.id(), invalidDate, amount));
    }
    {
        // no booking w/o creditor
        QVERIFY( not bookDeposit(Invalid_contract_id, validDate, amount));
    }
    {
        QVERIFY(bookDeposit(cont.id (), validDate, amount));
        QVector<booking> v= getBookings(cont.id());
        QCOMPARE(v.size (), 1);
        QCOMPARE( v[0], booking(cont.id(), bookingType::deposit, validDate, amount));
    }
}
void test_booking::test_bookPayout()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    QDate validDate (2020, 4, 28);
    double amount {111.11};
    {
        QVERIFY( bookPayout(cont.id(), validDate, amount));
        QVERIFY( bookPayout(cont.id(), validDate, -1 *amount));
        QVector<booking> v= getBookings(cont.id());
        QCOMPARE(v.size (), 2);
        QCOMPARE( v[0], booking(cont.id(), bookingType::payout, validDate, -1 *amount));
        QCOMPARE( v[1], booking(cont.id(), bookingType::payout, validDate, -1 *amount));
    }
}
void test_booking::test_bookReInvest()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    QDate validDate (2020, 4, 28);
    double amount {111.11};
    {
        QVERIFY( not bookReInvestInterest (cont.id(), validDate, -1 *amount));
    }
    {
        QVERIFY( bookReInvestInterest (cont.id(), validDate, amount));
        QVector<booking> v= getBookings(cont.id());
        QCOMPARE(v.size(), 1);
        QCOMPARE( v[0], booking(cont.id(), bookingType::reInvestInterest, validDate, amount));
    }
}
void test_booking::test_bookAnnualReInvest()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    QDate validDate (2020, 4, 28);
    double amount {111.11};
    {
        QVERIFY( not bookAnnualInterestDeposit (cont.id(), validDate, -1 *amount));
    }
    {
        QVERIFY( bookAnnualInterestDeposit( cont.id(), validDate, amount));
        QVector<booking> v= getBookings(cont.id());
        QCOMPARE(v.size(), 1);
        QCOMPARE( v[0], booking(cont.id(), bookingType::annualInterestDeposit, validDate, amount));
    }
}
void test_booking::test_bookInterestActive()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    QDate validDate (2020, 4, 28);

    QVERIFY (bookInterestActive (cont.id(), validDate));
    QVector<booking> v= getBookings(cont.id());
    QCOMPARE(v.size(), 1);
    QCOMPARE( v[0], booking(cont.id(), bookingType::setInterestActive, validDate, 0.));
}
void test_booking::test_getNbrOfBookings()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));

    QCOMPARE( 0, getNbrOfBookings (Invalid_contract_id));
    QCOMPARE( 0, getNbrOfBookings (cont.id()));

    QDate d1 (2020, 1, 1);
    QDate later  ( d1.addDays ( 2));
    QDate earlier( d1.addDays (-2));
    QDate muchEarlier( d1.addDays ( -4));
    QVERIFY( bookDeposit (cont.id(), d1, 100.));
    QCOMPARE( 1, getNbrOfBookings (cont.id ()));
    QCOMPARE( 0, getNbrOfBookings (cont.id (), later));
    QCOMPARE( 1, getNbrOfBookings (cont.id (), earlier, d1));
    QCOMPARE( 0, getNbrOfBookings (cont.id (), muchEarlier, earlier));

    QVERIFY(bookPayout (cont.id(), d1.addDays (1), 50.));
    QCOMPARE( 2, getNbrOfBookings (cont.id ()));
    QCOMPARE( 1, getNbrOfBookings (cont.id (), earlier, d1));
    QCOMPARE( 2, getNbrOfBookings (cont.id (), earlier, d1.addDays (1)));
    QCOMPARE( 0, getNbrOfBookings (cont.id (), muchEarlier, earlier));
}
void test_booking::test_getBookings()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));

    qlonglong invalidCreditorId {42};
    QCOMPARE(getBookings(Invalid_contract_id), QVector<booking>());
    QCOMPARE(getBookings(cont.id()), QVector<booking>());

    QDate d1 (2020, 1, 1);
    QDate d2 =d1.addDays (5);
    QDate d3 =d2.addDays (3);
    double deposit_amount =100.;
    double payout_amount =50.;
    QVERIFY( bookDeposit (cont.id(), d1, deposit_amount));

    {
        QVector<booking>  bookings =getBookings(cont.id());
        QVERIFY( bookings.size() == 1);
        QCOMPARE( bookings[0], booking(cont.id(), bookingType::deposit, d1, deposit_amount));
    }

    {
        QVERIFY( bookPayout (cont.id(), d2, payout_amount));
        QVector<booking> bookings =getBookings(cont.id());
        QVERIFY( bookings.size() == 2);
        QCOMPARE( bookings[1], booking(cont.id(), bookingType::deposit, d1, deposit_amount));
        QCOMPARE( bookings[0], booking(cont.id(), bookingType::payout,  d2, -1 *fabs(payout_amount)));
    }

    {
        QVector<booking> bookings =getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, "DATUM ASC");
        QVERIFY( bookings.size() == 2);
        QCOMPARE( bookings[0], booking(cont.id(), bookingType::deposit, d1, deposit_amount));
        QCOMPARE( bookings[1], booking(cont.id(), bookingType::payout,  d2, -1 *fabs(payout_amount)));

        QVERIFY(bookInterestActive (cont.id(), d3));
        bookings =getBookings(cont.id());
        QVERIFY (bookings.size() == 3);
        QVERIFY ( getBookings(cont.id(), d1, d2).size() == 2);
        QVERIFY ( getBookings(cont.id(), d2, d3).size() == 2);
        QVERIFY ( getBookings(cont.id(), d3).size() == 1);
        QVERIFY ( getBookings(cont.id(), d3.addDays (1)).isEmpty());
    }
    {
        // provided dates are included
        QVector<booking> twobookings =getBookings(Invalid_contract_id, d1.addDays(1), d3, "DATUM ASC");
        QCOMPARE(twobookings.size(), 2);

        QVector<booking> allbookings =getBookings(Invalid_contract_id, d1, d3, "DATUM ASC");
        QCOMPARE(allbookings.size(), 3);

        twobookings.clear();
        twobookings =getBookings(Invalid_contract_id, d1, d3.addDays(-1), "DATUM ASC");
        QCOMPARE(twobookings.size(), 2);
    }
}
void test_booking::test_yearsWAnnualBookings()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));

    QCOMPARE( yearsWithAnnualBookings().size(), 0);

    bookDeposit (cont.id(), QDate( 2020, 4, 14), 30.);
    QCOMPARE( yearsWithAnnualBookings().size(), 0);

    bookAnnualInterestDeposit (cont.id(), QDate(2019, 12, 31), 1.);
    QVector<int> annualI_bookings =yearsWithAnnualBookings();
    QCOMPARE( annualI_bookings.size(), 1);
    QCOMPARE( annualI_bookings[0], 2019);

    contract cont1(saveRandomContract (c.id()));
    bookAnnualInterestDeposit (cont1.id(), QDate(2019, 12, 31), 1.);
    annualI_bookings =yearsWithAnnualBookings();
    QCOMPARE( annualI_bookings.size(), 1);
    QCOMPARE( annualI_bookings[0], 2019);

    bookAnnualInterestDeposit (cont.id(), QDate(2020, 12, 31), 1.);
    bookAnnualInterestDeposit (cont.id(), QDate(2020, 12, 31), 1.);
    annualI_bookings =yearsWithAnnualBookings();
    QCOMPARE( annualI_bookings.size(), 2);
    QCOMPARE (annualI_bookings[0], 2020);
    QCOMPARE (annualI_bookings[1], 2019);
}
void test_booking::test_changeBookingValue() {
    contract cont;
    cont.initRandom(saveRandomCreditor().id());
    cont.saveNewContract();
    cont.bookInitialPayment(cont.conclusionDate().addDays(1), cont.plannedInvest() +1.);
    writeBookingUpdate( bookingId_t{1}, cont.plannedInvest());
    booking b =cont.latestBooking();
    QCOMPARE(b.amount*100., cont.plannedInvest());
}
void test_booking::test_bookingTypeFunctions()
{
    for (int i=0; i< 20; i++) {
        switch (i)
        {
        case 0:{
            QString bt =bookingTypeDisplayString(bookingtypeFromInt(i));
            QCOMPARE(bt, qsl("Alle Buchungstypen"));
            break;
        }
        case 1:{
            QString bt =bookingTypeDisplayString(bookingtypeFromInt(i));
            QCOMPARE(bt, qsl("Einzahlung"));
            break;
        }
        case 2:{
            QString bt =bookingTypeDisplayString(bookingtypeFromInt(i));
            QCOMPARE(bt, qsl("Auszahlung"));
            break;
        }
        case 4:{
            QString bt =bookingTypeDisplayString(bookingtypeFromInt(i));
            QCOMPARE(bt, qsl("Zinsanrechnung"));
            break;
        }
        case 8:{
            QString bt =bookingTypeDisplayString(bookingtypeFromInt(i));
            QCOMPARE( bt, qsl("Jahreszins"));
            break;
        }
        case 16:{
            QString bt =bookingTypeDisplayString(bookingtypeFromInt(i));
            QCOMPARE(bt, qsl("Aktivierung d. Zinszahlung"));
            break;
        }
        default:{
           QTest::ignoreMessage(QtCriticalMsg, "invalid booking type");
            QString bt =bookingTypeDisplayString(bookingtypeFromInt(i));
            QCOMPARE(bt, qsl("Alle Buchungstypen"));
            break;
        }
        }
    }
}
