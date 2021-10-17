#include <QtTest>

#include "../DKV2/helper.h"
#include "../DKV2/helpersql.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"

#include "test_contract.h"

void test_contract::initTestCase()
{   LOG_CALL;
    createTestDbTemplate();
}

void test_contract::cleanupTestCase()
{   LOG_CALL;
    cleanupTestDbTemplate();
}

void test_contract::init()
{   LOG_CALL;
    initTestDbFromTemplate();
}

void test_contract::cleanup()
{   LOG_CALL;
    cleanupTestDb();
}

void test_contract::test_createUninit_Contract()
{   LOG_CALL;
    contract c;
}

void test_contract::test_set_get_interest()
{   LOG_CALL;
    contract c;
    c.setInterestRate(1.5);
    QCOMPARE( c.interestRate(), 1.5);
    c.setInterestRate(1.49);
    QCOMPARE( c.interestRate(), 1.49);
    creditor cre = saveRandomCreditor();
    c.setCreditorId(cre.id());
    c.saveNewContract();
    contract d(c.id());
    QCOMPARE(d.interestRate(), 1.49);
}

void test_contract::test_activateContract()
{   LOG_CALL;
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));

    QVERIFY(cont.initialBookingReceived() == false);
    QVERIFY( not cont.bookInitialPayment(QDate(), 1000.));
    double amount =cont.plannedInvest();
    QVERIFY(cont.bookInitialPayment(QDate::currentDate(), amount));
    QVERIFY(cont.initialBookingReceived() == true);
    QCOMPARE(cont.latestBooking().date, QDate::currentDate());
    QCOMPARE(cont.latestBooking().amount, cont.plannedInvest());
    QCOMPARE(cont.latestBooking().type, booking::Type::deposit);
    QCOMPARE(cont.value(), amount);
    QCOMPARE(cont.investedValue(), amount);
    // activating an active contract fails:
    QVERIFY(false == cont.bookInitialPayment(QDate::currentDate(), cont.plannedInvest()));
}

void test_contract::test_readContractFromDb()
{
    LOG_CALL;
    creditor c(saveRandomCreditor());
    {
        contract cont_fixed;
        cont_fixed.initRandom(c.id());
        cont_fixed.setInterestModel(interestModel::fixed);
        cont_fixed.setInterestActive(true);
        cont_fixed.saveNewContract();
        cont_fixed.bookInitialPayment(QDate::currentDate(), cont_fixed.plannedInvest());
        contract newC_fixed(cont_fixed.id());
        QCOMPARE(cont_fixed, newC_fixed);
    }
    {
        contract cont_fixed;
        cont_fixed.initRandom(c.id());
        cont_fixed.setInterestModel(interestModel::fixed);
        cont_fixed.setInterestActive(false);
        cont_fixed.saveNewContract();
        cont_fixed.bookInitialPayment(QDate::currentDate(), cont_fixed.plannedInvest());
        contract newC_fixed(cont_fixed.id());
        QCOMPARE(cont_fixed, newC_fixed);
    }
    {
        contract cont_reinv;
        cont_reinv.initRandom(c.id());
        cont_reinv.setInterestModel(interestModel::reinvest);
        cont_reinv.setInterestActive(true);
        cont_reinv.saveNewContract();
        cont_reinv.bookInitialPayment(QDate::currentDate(), cont_reinv.plannedInvest());
        contract newC_reinv(cont_reinv.id());
        QCOMPARE(cont_reinv, newC_reinv);
    }
    {
        contract cont_reinv;
        cont_reinv.initRandom(c.id());
        cont_reinv.setInterestModel(interestModel::reinvest);
        cont_reinv.setInterestActive(false);
        cont_reinv.saveNewContract();
        cont_reinv.bookInitialPayment(QDate::currentDate(), cont_reinv.plannedInvest());
        contract newC_reinv(cont_reinv.id());
        QCOMPARE(cont_reinv, newC_reinv);
    }
    {
        contract cont_payout;
        cont_payout.initRandom(c.id());
        cont_payout.setInterestModel(interestModel::fixed);
        cont_payout.setInterestActive(true);
        cont_payout.saveNewContract();
        cont_payout.bookInitialPayment(QDate::currentDate(), cont_payout.plannedInvest());
        contract newC_payout(cont_payout.id());
        QCOMPARE(cont_payout, newC_payout);
    }
    {
        contract cont_payout;
        cont_payout.initRandom(c.id());
        cont_payout.setInterestModel(interestModel::fixed);
        cont_payout.setInterestActive(false);
        cont_payout.saveNewContract();
        cont_payout.bookInitialPayment(QDate::currentDate(), cont_payout.plannedInvest());
        contract newC_payout(cont_payout.id());
        QCOMPARE(cont_payout, newC_payout);
    }
}

void test_contract::test_randomContract()
{   LOG_CALL;
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    QCOMPARE(rowCount("Vertraege"), 1);
}

void test_contract::test_randomContracts()
{   LOG_CALL;
    int count = 50;
    dbgTimer t(QString::number(count) + " contracts");
    saveRandomCreditors(count/3);
    saveRandomContracts(count);
    QCOMPARE(rowCount("Vertraege"), count);
}

interestModel nextInterestModel (interestModel m) {
    int i =toInt(m);
    i = (i+1)%toInt(interestModel::maxId);
    return fromInt(i);
}

void test_contract::test_write_read_contract()
{   LOG_CALL;
    for( int i=0; i<50; i++) {
        creditor c(saveRandomCreditor());
        contract cont_write(saveRandomContract(c.id()));
        contract cont_read(cont_write.id());
        QCOMPARE(cont_write, cont_read);
        // test comparison
        cont_read.setInterestModel( nextInterestModel(cont_read.iModel()));
        QVERIFY(cont_write not_eq cont_read);
    }
}

void test_contract::test_get_set_interest()
{
    for (int i=0; i<10; i++) {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));

        const double expected =cont.interestRate();
        for (int j=0; j<3; j++) {
            cont.setInterestRate(cont.interestRate());
            QCOMPARE(cont.interestRate(), expected);
        }
    }
}

void test_contract::deposit_inactive_contract_fails()
{
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestModel(interestModel::fixed);
        cont.setInterestActive(true);

        QVERIFY( not cont.deposit(QDate::currentDate(), 1000.));
        QVERIFY( not cont.payout(QDate::currentDate(), 1000.));
        QDate aDate = QDate(2019, 1, 1);
        cont.bookInitialPayment(aDate, 1000.);
        QVERIFY(cont.deposit(aDate.addMonths(6), 1000.));
        QVERIFY(cont.payout(aDate.addMonths(6).addDays(1), 100.));
        QCOMPARE(cont.investedValue(), 1900.);
        double newValue =1000. +r2(cont.interestRate() *0.005 *1000.) +1000.;
        newValue =newValue +r2(cont.interestRate() /36000. *newValue) -100.;
        QCOMPARE(cont.value(), r2(newValue));
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestModel(interestModel::fixed);
        cont.setInterestActive(false);

        QVERIFY( not cont.deposit(QDate::currentDate(), 1000.));
        QVERIFY( not cont.payout(QDate::currentDate(), 1000.));
        QDate aDate = QDate(2019, 1, 1);
        cont.bookInitialPayment(aDate, 1000.);
        QVERIFY(cont.deposit(aDate.addMonths(6), 1000.));
        QVERIFY(cont.payout(aDate.addMonths(6).addDays(1), 100.));
        QCOMPARE(cont.investedValue(), 1900.);
        QCOMPARE(cont.value(),1900.);
    }
}

void test_contract::too_high_payout_fails()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    cont.setInterestActive(true);

    cont.bookInitialPayment(QDate::currentDate(), 1000.);
    QVERIFY( not cont.payout(QDate::currentDate().addDays(1), 1001.));
}

void test_contract::unsequenced_bookings_fail()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
   // cont.setInterestActive(true);

    QDate aDate = QDate::currentDate();
    cont.bookInitialPayment(aDate, 1000.);
    QVERIFY( not cont.deposit(aDate, 1000.));
    QVERIFY( not cont.deposit(aDate.addDays(-1), 1000.));
    QVERIFY( not cont.payout(aDate, 100.));
    QVERIFY( not cont.payout(aDate.addDays(-1), 100.));
}

void test_contract::test_annualSettlement_inactive_fails()
{
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(1.);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(true);
        QVERIFY( not cont.annualSettlement(2019));

        cont.bookInitialPayment(QDate(2019, 6, 30), 1000.);
        QCOMPARE(cont.annualSettlement(2019), 2019);
        QCOMPARE(cont.value(), 1005.);
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(1.);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(false);
        QVERIFY( not cont.annualSettlement(2019));

        cont.bookInitialPayment(QDate(2019, 6, 30), 1000.);
        QCOMPARE(cont.annualSettlement(2019), 2019);
        QCOMPARE(cont.value(), 1000.);
    }
}

void test_contract::test_annualSettlement_fullYear()
{
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(1.);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(true);

        cont.bookInitialPayment(QDate(2018, 12, 30), 1000.);
        QCOMPARE(cont.annualSettlement(2019), 2019);
        QCOMPARE(cont.value(), 1010.);
        QCOMPARE(cont.annualSettlement(2020), 2020);
        QCOMPARE(cont.value(), 1020.1);
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(1.);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(false);

        cont.bookInitialPayment(QDate(2018, 12, 30), 1000.);
        QCOMPARE(cont.annualSettlement(2019), 2019);
        QCOMPARE(cont.value(), 1000.);
        QCOMPARE(cont.annualSettlement(2020), 2020);
        QCOMPARE(cont.value(), 1000);
    }
}

void test_contract::test_annualSettlement_twoYear()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    cont.setInterestRate(1.);
    cont.setInterestModel(interestModel::reinvest);
    cont.setInterestActive(true);

    cont.bookInitialPayment(QDate(2018, 12, 30), 1000.);
//    2years in one go:
    QCOMPARE(cont.annualSettlement(2020), 2020);
    QCOMPARE(cont.value(), 1020.1);
}

void test_contract::test_deposit01()
{
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(2.0);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(true);

        // booking 1: deposit / activation
        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        // booking 2: interest deposit
        // booking 3: deposit
        cont.deposit(QDate(2019, 7, 1), 1000.);
        QCOMPARE(cont.value(), 2010.);
        QVERIFY(bookings::getBookings(cont.id()).count() == 3);
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(2.0);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(false);

        // booking 1: deposit / activation
        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        // booking 2: interest deposit
        // booking 3: deposit
        cont.deposit(QDate(2019, 7, 1), 1000.);
        QCOMPARE(cont.value(), 2000.);
        QVERIFY(bookings::getBookings(cont.id()).count() == 3);
    }
}

void test_contract::test_depositSwitches_31_12()
{
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(2.0);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(true);

        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        QVERIFY( cont.deposit(QDate(2020, 12, 31), 1000.));
        QVERIFY( cont.latestBooking().date == QDate(2020, 12, 30));
        QVERIFY( cont.payout(QDate(2021, 12, 31), 100));
        QVERIFY( cont.latestBooking().date == QDate(2021, 12, 30));
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(2.0);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(false);

        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        QVERIFY( cont.deposit(QDate(2020, 12, 31), 1000.));
        QVERIFY( cont.latestBooking().date == QDate(2020, 12, 30));
        QVERIFY( cont.payout(QDate(2021, 12, 31), 100));
        QVERIFY( cont.latestBooking().date == QDate(2021, 12, 30));
    }
}

void test_contract::test_deposit_wSettlement_reinvesting()
{
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestRate(2.0);
        cont.setInterestActive(true);

        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.deposit(QDate(2020, 7, 1), 1000.);
        QCOMPARE(cont.value(), 2030.2);
        // booking 1: activation (deposit)
        // booking 2: annual settlement 2019 (1.1.2020)
        // booking 3: interest deposit (1.7. 2020)
        // booking 4: deposit
        QCOMPARE(bookings::getBookings(cont.id()).count(), 4);
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestRate(2.0);
        cont.setInterestActive(false);

        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.deposit(QDate(2020, 7, 1), 1000.);
        QCOMPARE(cont.value(), 2000.);
        // booking 1: activation (deposit)
        // booking 2: annual settlement 2019 (1.1.2020)
        // booking 3: interest deposit (1.7. 2020)
        // booking 4: deposit
        QCOMPARE(bookings::getBookings(cont.id()).count(), 4);
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestRate(2.0);
        cont.setInterestActive(false);

        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.bookActivateInterest(QDate(2020, 6, 30));
        cont.deposit(QDate(2020, 12, 30), 1000.);
        QCOMPARE(cont.value(), 2010.);
        // booking 1: 1.1.2019 activation (deposit 1000)
        // booking 2: 1.1.2020 annual settlement 2019 (0)
        // booking 3: 30.6.2020 interest activation 30.6.2020
        // booking 4: 30.6.2020 interest deposit (0)
        // booking 4: 30.12.2020 interest deposit (10)
        // booking 5: deposit
        QCOMPARE(bookings::getBookings(cont.id()).count(), 6);
    }
}

void test_contract::test_deposit_wSettlement_wPayout()
{
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestModel(interestModel::payout);
        cont.setInterestRate(2.0);
        cont.setInterestActive(true);

        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.deposit(QDate(2020, 6, 30), 1000.);
        QCOMPARE(cont.value(), 2010);
        // booking 1: activation (deposit)
        // booking 2: annual settlement 2019 (1.1.2020) (reinv.)
        // booking 3: payout of annual settlement
        // booking 4: interest deposit (1.7. 2020)
        // booking 5: deposit
        QCOMPARE(bookings::getBookings(cont.id()).count(), 5);
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestModel(interestModel::payout);
        cont.setInterestRate(2.0);
        cont.setInterestActive(false);
        QVERIFY(not cont.bookActivateInterest(QDate(2019,1,1)));
        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.deposit(QDate(2020, 6, 30), 1000.);
        QVERIFY(not cont.bookActivateInterest(QDate(2020, 6, 29)));
        QVERIFY(cont.bookActivateInterest(QDate(2020, 7, 1)));
        QCOMPARE(cont.value(), 2000);
        // booking 1: activation (deposit)
        // booking 2: annual settlement 2019 (1.1.2020) (reinv.)
        // booking 3: payout of annual settlement
        // booking 4: interest deposit (1.7. 2020)
        // booking 5: deposit
        QCOMPARE(bookings::getBookings(cont.id()).count(), 7);
    }
}

void test_contract::test_payout()
{
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(2.0);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(true);

        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.payout(QDate(2019, 7, 1), 500.);
        QCOMPARE(cont.value(), 510.);
        QCOMPARE(bookings::getBookings(cont.id()).count(), 3);
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(2.0);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(false);

        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.payout(QDate(2019, 7, 1), 500.);
        QCOMPARE(cont.value(), 500.);
        QCOMPARE(bookings::getBookings(cont.id()).count(), 3);
    }
}

void test_contract::test_payout_wSettlement_reinvesting()
{
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(2.0);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(true);

        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.payout(QDate(2020, 7, 1), 500.);
        QCOMPARE(cont.value(), 530.2);
        QCOMPARE(bookings::getBookings(cont.id()).count(), 4);
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(2.0);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(false);

        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.payout(QDate(2020, 7, 1), 500.);
        QCOMPARE(cont.value(), 500.);
        QCOMPARE(bookings::getBookings(cont.id()).count(), 4);
    }
}
void test_contract::test_payout_wSettlement_wPayout()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    cont.setInterestRate(2.0);
    cont.setInterestModel(interestModel::payout);
    cont.setInterestActive(true);

    cont.bookInitialPayment(QDate(2019, 12, 30), 1000.);
    cont.payout(QDate(2020, 6, 30), 500.);
    // activation deposit 1000.
    // annual settlement (2020/1/1) wo payout (impl. AS -> no payout)
    // interest deposit 2020/1/1 - 2020/7/1
    // payout 500
    QCOMPARE(cont.value(), 510);
    QCOMPARE(bookings::getBookings(cont.id()).count(), 5);
}

void test_contract::test_activationDate()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    cont.setInterestActive(true);

    QDate aDate = QDate(2020, 5, 1);
    QCOMPARE(cont.initialBookingDate(), QDate());
    cont.bookInitialPayment(aDate, 1000.);
    QCOMPARE(cont.initialBookingDate(), aDate);
    cont.deposit(aDate.addMonths(6), 1000.);
    QCOMPARE(cont.initialBookingDate(), aDate);
    booking::bookReInvestInterest(cont.id(), QDate(2021, 1, 1), 10.);
    QCOMPARE(cont.initialBookingDate(), aDate);
}

void test_contract::test_activate_interest_on_same_date_will_fail()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    QDate d (cont.conclusionDate());
    cont.bookInitialPayment(d.addDays(1), cont.plannedInvest());
    QVERIFY(not cont.bookActivateInterest(d.addDays(1)));
    QVERIFY(cont.bookActivateInterest(d.addDays(2)));
}

void test_contract::test_getValue_byDate()
{
    dbgTimer t("8x contract::getValue()");
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    cont.setInterestRate(1.0);
    cont.setInterestModel();
    cont.setInterestActive(true);

    QDate aDate = QDate(2020, 4, 30);

    cont.bookInitialPayment(aDate, 1000.);
    QCOMPARE(cont.value(aDate), 1000.);
    QCOMPARE(cont.value(aDate.addMonths(1)), 1000.);

    cont.deposit(aDate.addMonths(6), 1000.); // 1.11.
    QCOMPARE(cont.value(aDate.addMonths(7)), 2005.);
    cont.payout(aDate.addYears(2), 100);
    QCOMPARE(cont.value(), 1935.18); // verified in excel

    QCOMPARE(cont.value(aDate), 1000.);
    QCOMPARE(cont.value(aDate.addMonths(1)), 1000.);
    QCOMPARE(cont.value(aDate.addMonths(7)), 2005.);
}

void test_contract::test_contract_cv_wInterestPayout()
{
    dbgTimer t(QStringLiteral("8x contract::getValue()"));
    creditor c(saveRandomCreditor());
    contract cont;
    cont.initRandom(c.id());
    cont.setInterestRate(1.0);
    cont.setInterestModel(interestModel::payout);
    cont.setInterestActive(true);

    cont.saveNewContract();
    QDate anydate = QDate(2020, 4, 30);

    // booking 1
    cont.bookInitialPayment(anydate, 1000.); // 1.5.2020
    QCOMPARE(cont.value(anydate), 1000.);
    QCOMPARE(cont.value(anydate.addMonths(1)), 1000.); // 1.6.2020
    QCOMPARE(cont.latestBooking().date, anydate);
    QCOMPARE(cont.latestBooking().type, booking::Type::deposit);

    cont.deposit(anydate.addMonths(6), 1000.); // 1.11.2020
    // deposit forces interest depoits, even for not reinvesting contracts
    // booking 2: interest deposit
    // booking 3: deposit
    QCOMPARE(cont.value(anydate.addMonths(7)), 2005.); // 1.12.2020
    QCOMPARE(bookings::getBookings(cont.id()).count(), 3);
    QCOMPARE(cont.latestBooking().date, anydate.addMonths(6));
    QCOMPARE(cont.latestBooking().type, booking::Type::deposit);

    // payout forces settlement of 2021
    // bookint 4: interest deposit 1.11.2020 - 1.1.2021
    // booking 5: payout of interest (3.34 Euro)
    // booking 6: interest deposit 1.1.2021 - 1.1.2022
    // booking 7: payout of interest (20.05)
    // booking 8: interest deposit 1.1.2022 - 1.5.2022
    // booking 9: payout 100 (4 month -> 6.68 Euro)
    cont.payout(anydate.addYears(2), 100); // 1.5.2022
    QCOMPARE(cont.value(), 1911.68); // verified in excel
    QCOMPARE(bookings::getBookings(cont.id()).count(), 9);
    QCOMPARE(cont.latestBooking().date, anydate.addYears(2));
    QCOMPARE(cont.latestBooking().type, booking::Type::payout);

    // perform settlement of year 2022, 1.1.2023
    cont.annualSettlement(2022);
    QCOMPARE(cont.value(), 1911.68); // no change due to payout
    QCOMPARE(bookings::getBookings(cont.id()).count(), 11);
    QCOMPARE(cont.latestBooking().date, QDate(2022, 12, 31));
    QCOMPARE(cont.latestBooking().type, booking::Type::annualInterestDeposit);
    QCOMPARE(cont.latestBooking().amount, 12.74);
}

void test_contract::test_contract_cv_reInvesting()
{
    dbgTimer t(QStringLiteral("8x contract::getValue()"));
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    cont.setInterestRate(1.0);
    cont.setInterestModel(interestModel::reinvest);
    cont.setInterestActive(true);

    QDate anydate = QDate(2020, 4, 30);
    // booking 1
    cont.bookInitialPayment(anydate, 1000.); // 1.5.2020
    QCOMPARE(cont.value(anydate), 1000.);
    QCOMPARE(cont.value(anydate.addMonths(1)), 1000.); // 1.6.2020
    QCOMPARE(cont.latestBooking().date, anydate);
    QCOMPARE(cont.latestBooking().type, booking::Type::deposit);

    cont.deposit(anydate.addMonths(6), 1000.); // 1.11.2020
    // deposit forces interest depoit
    // booking 2: interest deposit
    // booking 3: deposit
    QCOMPARE(cont.value(anydate.addMonths(7)), 2005.); // 1.12.2020
    QCOMPARE(bookings::getBookings(cont.id()).count(), 3);
    QCOMPARE(cont.latestBooking().date, anydate.addMonths(6));
    QCOMPARE(cont.latestBooking().type, booking::Type::deposit);

    // payout forces settlement of 2021
    // bookint 4: interest deposit 1.11.2020 - 1.1.2021 (2005€, 1m -> 3.34€)
    // booking 5: interest deposit 1.1.2021 - 1.1.2022 (2008.34€, 1y -> 20.08€)
    // booking 6: interest deposit 1.1.2022 - 1.5.2022 (2028,42€, 4 month -> 6.76 Euro)
    // booking 7: payout 100
    cont.payout(anydate.addYears(2), 100); // 1.5.2022
    QCOMPARE(cont.value(), 1935.18); // verified in excel
    QCOMPARE(bookings::getBookings(cont.id()).count(), 7);
    QCOMPARE(cont.latestBooking().date, anydate.addYears(2));
    QCOMPARE(cont.latestBooking().type, booking::Type::payout);

    // perform settlement of year 2022, 1.1.2023
    // 7Monate
    // booking 8
    cont.annualSettlement(2022);
    QCOMPARE(cont.value(), 1948.08);
    QCOMPARE(bookings::getBookings(cont.id()).count(), 8);
    QCOMPARE(cont.latestBooking().date, QDate(2022, 12, 31));
    QCOMPARE(cont.latestBooking().type, booking::Type::annualInterestDeposit);
    QCOMPARE(cont.latestBooking().amount, 12.90);
}

void test_contract::test_finalize()
{
    creditor creditor(saveRandomCreditor());
    contract cont(saveRandomContract(creditor.id()));
    cont.setInterestActive(true);

    QDate aDate = QDate(2020, 5, 1);
    cont.bookInitialPayment(aDate, 1000.);
    cont.deposit(aDate.addMonths(1), 1000.);
    QCOMPARE(tableRecordCount("Vertraege"), 1);
    QCOMPARE(tableRecordCount("Buchungen"), 3);
    QString contractId = QString::number(cont.id());
    double fi =0., fp =0.;
    QVERIFY(cont.finalize(false, aDate.addMonths(2), fi, fp));
    // finalize should reset the cont object
    QCOMPARE(cont.id(), -1);
    QCOMPARE(tableRecordCount("Vertraege"), 0);
    QCOMPARE(tableRecordCount("Buchungen"), 0);
    QCOMPARE(tableRecordCount("exVertraege"), 1);
    QCOMPARE(tableRecordCount("exBuchungen"), 5);
    QCOMPARE(executeSingleValueSql(
             contract::getTableDef_deletedContracts()["LaufzeitEnde"],
             "id=" +contractId), QDate(aDate.addMonths(2)));
}
