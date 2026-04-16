#include "test_contract.h"
#include "testhelper.h"

#include "../DKV2/helper_core.h"
#include "../DKV2/helperfin.h"
#include "../DKV2/helpersql.h"
#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"
#include "../DKV2/appconfig.h"

#include "testhelper.h"

#include <QtTest/QTest>

namespace {

struct referenceBookingStep
{
    enum class kind { deposit, payout, annualSettlement };

    kind type;
    QDate date;
    double amount{0.0};
    bool payoutInterest{false};
    contract::midYearInterestMode midYearInterest{contract::undecided};
    int settlementYear{0};
};

struct expectedBooking
{
    bookingType type;
    QDate date;
    double amount{0.0};
};

struct expectedBreakdown
{
    QDate periodEnd;
    contract::midYearInterestMode mode{contract::undecided};
    QVector<contract::interestSlice::kind> sliceKinds;
    QVector<double> sliceInterests;
};

struct referenceCase
{
    QString zinsusance;
    interestModel model{interestModel::reinvest};
    double rate{0.0};
    bool interestActive{true};
    QDate conclusionDate;
    QDate initialPaymentDate;
    double initialPaymentAmount{0.0};
    QVector<referenceBookingStep> steps;
    QVector<expectedBooking> expectedBookings;
    QVector<expectedBreakdown> expectedBreakdowns;
};

void runReferenceCase(const referenceCase& c)
{
    dbConfig::writeValue(ZINSUSANCE, c.zinsusance);

    creditor cred(saveRandomCreditor());
    contract cont(saveRandomContract(cred.id()));
    cont.setInterestRate(c.rate);
    cont.setInterestModel(c.model);
    cont.updateInterestActive(c.interestActive);
    cont.updateConclusionDate(c.conclusionDate);

    QVERIFY(cont.bookInitialPayment(c.initialPaymentDate, c.initialPaymentAmount));

    for (const referenceBookingStep& step : c.steps) {
        switch (step.type)
        {
        case referenceBookingStep::kind::deposit:
            QVERIFY(cont.deposit(step.date, step.amount, step.payoutInterest, step.midYearInterest));
            break;
        case referenceBookingStep::kind::payout:
            QVERIFY(cont.payout(step.date, step.amount, step.payoutInterest, step.midYearInterest));
            break;
        case referenceBookingStep::kind::annualSettlement:
            QCOMPARE(cont.annualSettlement(step.settlementYear), step.settlementYear);
            break;
        }
    }

    const QVector<booking> bookings{getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"))};
    QCOMPARE(bookings.size(), c.expectedBookings.size());
    for (int i = 0; i < c.expectedBookings.size(); ++i) {
        const expectedBooking& expected{c.expectedBookings[i]};
        QCOMPARE(bookings[i], booking(cont.id(), expected.type, expected.date, expected.amount));
    }

    for (const expectedBreakdown& expected : c.expectedBreakdowns) {
        const contract::interestBreakdown actual{cont.interestBreakdownUntilDate(expected.periodEnd)};
        QVERIFY(actual.ok);
        QCOMPARE(actual.mode, expected.mode);
        QCOMPARE(actual.slices.size(), expected.sliceKinds.size());
        QCOMPARE(actual.slices.size(), expected.sliceInterests.size());
        for (int i = 0; i < actual.slices.size(); ++i) {
            QCOMPARE(actual.slices[i].type, expected.sliceKinds[i]);
            QCOMPARE(actual.slices[i].interest, expected.sliceInterests[i]);
        }
    }
}

} // namespace

Q_DECLARE_METATYPE(referenceCase)

void test_contract::init()
{
    initTestDkDb_InMemory ();
}

void test_contract::cleanup()
{
    cleanupTestDb_InMemory ();
}

void test_contract::test_createUninit_Contract()
{
    contract c;
}

void test_contract::test_set_get_interest()
{
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
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));

    QVERIFY(cont.initialPaymentReceived() == false);
    QVERIFY( not cont.bookInitialPayment(QDate(), 1000.)); // invalid date
    double amount =cont.plannedInvest();
    QVERIFY(cont.bookInitialPayment(QDate::currentDate(), amount)); // valid date

    // Vertragsabschluss ist innerhalb der letzten 2 Jahre in zufälligen Verträgen
    QVERIFY(cont.initialPaymentReceived() == true);
    booking latest =cont.latestBooking ();
    QCOMPARE(latest.date, QDate::currentDate());
    QCOMPARE(latest.amount, amount);
    QCOMPARE(latest.type, bookingType::deposit);
    QCOMPARE(cont.value(), amount);
    QCOMPARE(cont.investedValue(), amount);
    // activating an active contract fails:
    QVERIFY(false == cont.bookInitialPayment(QDate::currentDate(), cont.plannedInvest()));
}

void test_contract::test_readContractFromDb()
{
    creditor c(saveRandomCreditor());
    {
        contract cont_fixed;
        cont_fixed.initRandom(c.id());
        cont_fixed.setInterestModel(interestModel::fixed); // zinsen werden nicht verzinst, nicht ausbezahlt
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
        cont_fixed.setInterestActive(false);  // "Verzinsung ab Einzug" ...
        cont_fixed.saveNewContract();
        cont_fixed.bookInitialPayment(QDate::currentDate(), cont_fixed.plannedInvest());
        contract newC_fixed(cont_fixed.id());
        QCOMPARE(cont_fixed, newC_fixed);
    }
    {
        contract cont_reinv;
        cont_reinv.initRandom(c.id());
        cont_reinv.setInterestModel(interestModel::reinvest); // thesaurierend
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
        cont_reinv.setInterestActive(false); // "Verzinsung ab Einzug" ...
        cont_reinv.saveNewContract();
        cont_reinv.bookInitialPayment(QDate::currentDate(), cont_reinv.plannedInvest());
        contract newC_reinv(cont_reinv.id());
        QCOMPARE(cont_reinv, newC_reinv);
    }
    {
        contract cont_payout;
        cont_payout.initRandom(c.id());
        cont_payout.setInterestModel(interestModel::payout); // Zinsen auszahlend
        cont_payout.setInterestActive(true);
        cont_payout.saveNewContract();
        cont_payout.bookInitialPayment(QDate::currentDate(), cont_payout.plannedInvest());
        contract newC_payout(cont_payout.id());
        QCOMPARE(cont_payout, newC_payout);
    }
    {
        contract cont_payout;
        cont_payout.initRandom(c.id());
        cont_payout.setInterestModel(interestModel::payout);
        cont_payout.setInterestActive(false); // verzögerte verzinsung
        cont_payout.saveNewContract();
        cont_payout.bookInitialPayment(QDate::currentDate(), cont_payout.plannedInvest());
        contract newC_payout(cont_payout.id());
        QCOMPARE(cont_payout, newC_payout);
    }
    {
        contract cont_payout;
        cont_payout.initRandom(c.id());
        cont_payout.setInterestModel(interestModel::zero); // Zinslos
        cont_payout.setInterestActive(true);
        cont_payout.saveNewContract();
        cont_payout.bookInitialPayment(QDate::currentDate(), cont_payout.plannedInvest());
        contract newC_payout(cont_payout.id());
        QCOMPARE(cont_payout, newC_payout);
    }
    {
        contract cont_payout;
        cont_payout.initRandom(c.id());
        cont_payout.setInterestModel(interestModel::zero);
        cont_payout.setInterestActive(false); // verzögerte verzinsung
        cont_payout.saveNewContract();
        cont_payout.bookInitialPayment(QDate::currentDate(), cont_payout.plannedInvest());
        contract newC_payout(cont_payout.id());
        QCOMPARE(cont_payout, newC_payout);
    }
}

void test_contract::test_randomContract()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    QCOMPARE(rowCount("Vertraege"), 1);
    contract cont2(saveRandomContract(c.id()));
    QCOMPARE(rowCount("Vertraege"), 2);
}

void test_contract::test_randomContracts()
{
    int count = 50;
    saveRandomCreditors(count/3);
    saveRandomContracts(count);
    QCOMPARE(rowCount("Vertraege"), count);
}

interestModel nextInterestModel (interestModel m) {
    int i =toInt(m);
    i = (i+1)%toInt(interestModel::maxId);
    return interestModelFromInt(i);
}

void test_contract::test_write_read_contract()
{
    for( int i=0; i<10; i++) {
        creditor c(saveRandomCreditor());
        contract cont_write(saveRandomContract(c.id()));
        contract cont_read(cont_write.id());
        QCOMPARE(cont_write, cont_read);
        // test comparison
        cont_read.setInterestModel( nextInterestModel(cont_read.iModel()));
        QVERIFY(cont_write not_eq cont_read);
    }
}

void test_contract::deposit_inactive_contract_fails()
{
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.updateConclusionDate(QDate::currentDate().addDays(-1));
        cont.setInterestModel(interestModel::fixed);
        cont.setInterestActive(true);

        QVERIFY( not cont.deposit(QDate::currentDate(), 1000.));
        QVERIFY( not cont.payout(QDate::currentDate(), 1000.));
        QDate aDate = QDate::currentDate();
        cont.bookInitialPayment(aDate, 1000.);
        QVERIFY(cont.deposit(aDate.addMonths(6), 1000.));
        QVERIFY(cont.payout(aDate.addMonths(6).addDays(1), 100.));
        QCOMPARE(cont.investedValue(), 1900.);
        double newValue =1000. +r2(cont.interestRate() /100. *1000. /2) +1000.;
        newValue =newValue +r2(cont.interestRate() /100. /360. *newValue) -100.;
        QCOMPARE(cont.value(), r2(newValue));
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.updateConclusionDate(QDate::currentDate().addDays(-1));
        cont.setInterestModel(interestModel::fixed);
        cont.setInterestActive(false);

        QVERIFY( not cont.deposit(QDate::currentDate(), 1000.));
        QVERIFY( not cont.payout(QDate::currentDate(), 1000.));
        QDate aDate = QDate::currentDate().addDays(5);
        QVERIFY(cont.bookInitialPayment(aDate, 1000.));
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
        cont.updateConclusionDate(QDate(2019,6,29));
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
        cont.updateConclusionDate( QDate(2019,1,1));
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
        cont.updateConclusionDate( QDate(2018, 12, 29));
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
        cont.updateConclusionDate(QDate(2018,12,28));

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
    cont.updateConclusionDate(QDate(2018, 12,29));

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
        cont.updateConclusionDate(QDate(2018,12,31));
        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        // booking 2: interest deposit
        // booking 3: deposit
        cont.deposit(QDate(2019, 7, 1), 1000.);
        QCOMPARE(cont.value(), 2010.);
        QCOMPARE(getNbrOfBookings(cont.id()), 3);
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(2.0);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(false);

        // booking 1: deposit / activation
        cont.updateConclusionDate(QDate(2018,12,31));
        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        // booking 2: interest deposit
        // booking 3: deposit
        cont.deposit(QDate(2019, 7, 1), 1000.);
        QCOMPARE(cont.value(), 2000.);
        QVERIFY(getNbrOfBookings(cont.id()) == 3);
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

        cont.updateConclusionDate(QDate(2018,12,31));
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

        cont.updateConclusionDate(QDate(2018,12,31));
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

        cont.updateConclusionDate(QDate(2018,12,31));
        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.deposit(QDate(2020, 7, 1), 1000.);
        QCOMPARE(cont.value(), 2030.2);
        // booking 1: activation (deposit)
        // booking 2: annual settlement 2019 (1.1.2020)
        // booking 3: interest deposit (1.7. 2020)
        // booking 4: deposit
        QCOMPARE(getNbrOfBookings(cont.id()), 4);
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestRate(2.0);
        cont.setInterestActive(false);

        cont.updateConclusionDate(QDate(2018,12,31));
        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.deposit(QDate(2020, 7, 1), 1000.);
        QCOMPARE(cont.value(), 2000.);
        // booking 1: activation (deposit)
        // booking 2: annual settlement 2019 (1.1.2020)
        // booking 3: interest deposit (1.7. 2020)
        // booking 4: deposit
        QCOMPARE(getNbrOfBookings(cont.id()), 4);
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestRate(2.0);
        cont.setInterestActive(false);

        cont.updateConclusionDate(QDate(2018,12,31));
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
        QCOMPARE(getNbrOfBookings(cont.id()), 6);
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

        cont.updateConclusionDate(QDate(2018,12,31));
        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.deposit(QDate(2020, 6, 30), 1000.);
        QCOMPARE(cont.value(), 2010);
        // booking 1: activation (deposit)
        // booking 2: annual settlement 2019 (1.1.2020) (reinv.)
        // booking 3: payout of annual settlement
        // booking 4: interest deposit (1.7. 2020)
        // booking 5: deposit
        QCOMPARE(getNbrOfBookings(cont.id()), 5);
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestModel(interestModel::payout);
        cont.setInterestRate(2.0);
        cont.setInterestActive(false);

        cont.updateConclusionDate(QDate(2018,12,31));
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
        QCOMPARE(getNbrOfBookings(cont.id()), 7);
    }
}

void test_contract::test_deposit_choosesDeferredMidYearInterestMode()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    cont.setInterestRate(1.0);
    cont.setInterestModel(interestModel::reinvest);
    cont.setInterestActive(true);

    const QDate initialDate(2020, 1, 15);
    const QDate depositDate(2020, 7, 1);
    cont.updateConclusionDate(initialDate.addDays(-1));

    QVERIFY(cont.bookInitialPayment(initialDate, 1000.));
    QVERIFY(cont.deposit(depositDate, 1000., false, contract::deferred));

    const QVector<booking> bookings = getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("Datum ASC, id ASC"));
    QCOMPARE(bookings.size(), 3);
    QCOMPARE(bookings[0], booking(cont.id(), bookingType::deposit, initialDate, 1000.));
    QCOMPARE(bookings[1], booking(cont.id(), bookingType::deferredMidYearInterest, depositDate, 0.));
    QCOMPARE(bookings[2], booking(cont.id(), bookingType::deposit, depositDate, 1000.));
    QCOMPARE(cont.yearlyMidYearInterestMode(2020), contract::deferred);
    QCOMPARE(cont.value(), 2000.);
}

void test_contract::test_payout()
{
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(2.0);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(true);

        cont.updateConclusionDate(QDate(2018,12,31));
        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.payout(QDate(2019, 7, 1), 500.);
        QCOMPARE(cont.value(), 510.);
        QCOMPARE(getNbrOfBookings(cont.id()), 3);
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(2.0);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(false);

        cont.updateConclusionDate(QDate(2018,12,31));
        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.payout(QDate(2019, 7, 1), 500.);
        QCOMPARE(cont.value(), 500.);
        QCOMPARE(getNbrOfBookings(cont.id()), 3);
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

        cont.updateConclusionDate(QDate(2018,12,31));
        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.payout(QDate(2020, 7, 1), 500.);
        QCOMPARE(cont.value(), 530.2);
        QCOMPARE(getNbrOfBookings(cont.id()), 4);
    }
    {
        creditor c(saveRandomCreditor());
        contract cont(saveRandomContract(c.id()));
        cont.setInterestRate(2.0);
        cont.setInterestModel(interestModel::reinvest);
        cont.setInterestActive(false);

        cont.updateConclusionDate(QDate(2018,12,31));
        cont.bookInitialPayment(QDate(2019, 1, 1), 1000.);
        cont.payout(QDate(2020, 7, 1), 500.);
        QCOMPARE(cont.value(), 500.);
        QCOMPARE(getNbrOfBookings(cont.id()), 4);
    }
}
void test_contract::test_payout_wSettlement_wPayout()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    cont.setInterestRate(2.0);
    cont.setInterestModel(interestModel::payout);
    cont.setInterestActive(true);

    cont.updateConclusionDate(QDate(2018,12,31));
    cont.bookInitialPayment(QDate(2019, 12, 30), 1000.);
    cont.payout(QDate(2020, 6, 30), 500.);
    // activation deposit 1000.
    // annual settlement (2020/1/1) wo payout (impl. AS -> no payout)
    // interest deposit 2020/1/1 - 2020/7/1
    // payout 500
    QCOMPARE(cont.value(), 510);
    QCOMPARE(getNbrOfBookings(cont.id()), 5);
}

QDate initialBookingDate(contractId_t cId)
{
    QString where = qsl("%1.%2=%3").arg(booking::tn_Buchungen, booking::fn_bVertragsId);
    return executeSingleValueSql(qsl("MIN(%1)").arg(booking::fn_bDatum), booking::tn_Buchungen, where.arg(cId.v)).toDate();
}

void test_contract::test_activationDate()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    cont.setInterestActive(true);
    cont.updateConclusionDate(QDate(2020,4,20));

    QDate aDate = QDate(2020, 5, 1);
    QCOMPARE(initialBookingDate(cont.id ()), QDate());
    QVERIFY(cont.bookInitialPayment(aDate, 1000.));
    QCOMPARE(initialBookingDate(cont.id ()), aDate);
    cont.deposit(aDate.addMonths(6), 1000.);

    // test
    QCOMPARE(initialBookingDate(cont.id ()), aDate);
    QVERIFY(bookReInvestInterest(cont.id(), QDate(2021, 1, 1), 10.));
    // test
    QCOMPARE(initialBookingDate(cont.id ()), aDate);
}

void test_contract::test_activate_interest_on_same_date_will_not_fail()
{
    creditor c {saveRandomCreditor()};
    contract cont {saveRandomContract(c.id())};
    cont.updateInterestActive(false);
    QDate Vertragsdatum {cont.conclusionDate()};

    // interest activation fails before initial payment
    QVERIFY(not cont.bookActivateInterest(Vertragsdatum.addDays(-1)));
    QVERIFY(not cont.bookActivateInterest(Vertragsdatum));
    QVERIFY(not cont.bookActivateInterest(Vertragsdatum.addDays(1)));
    // init payment only AT or AFTER contract date
    QVERIFY(not cont.bookInitialPayment(Vertragsdatum.addDays(-1), cont.plannedInvest()));
    QVERIFY(cont.bookInitialPayment(Vertragsdatum, cont.plannedInvest()));
    // activ. Interest only after initial payment
    QVERIFY(not cont.bookActivateInterest(Vertragsdatum.addDays(-1)));
    QVERIFY(cont.bookActivateInterest(Vertragsdatum));
}
void test_contract::test_getValue_byDate()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    cont.setInterestRate(1.0);
    cont.setInterestModel();
    cont.setInterestActive(true);

    QDate aDate = QDate(2020, 4, 30);
    cont.updateConclusionDate(aDate.addDays(-1));
    QVERIFY(cont.bookInitialPayment(aDate, 1000.));
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
    creditor c(saveRandomCreditor());
    contract cont;
    cont.initRandom(c.id());
    cont.setInterestRate(1.0);
    cont.setInterestModel(interestModel::payout);
    cont.updateInterestActive(true);
    QDate anydate = QDate(2020, 4, 30);
    cont.updateConclusionDate(anydate.addDays(-1));
    cont.saveNewContract();

    // booking 1
    cont.bookInitialPayment(anydate, 1000.); // 1.5.2020
    QCOMPARE(cont.value(anydate), 1000.);
    QCOMPARE(cont.value(anydate.addMonths(1)), 1000.); // 1.6.2020
    booking latest =cont.latestBooking ();
    QCOMPARE(latest.date, anydate);
    QCOMPARE(latest.type, bookingType::deposit);

    cont.deposit(anydate.addMonths(6), 1000.); // 1.11.2020
    // deposit forces interest depoits, even for not reinvesting contracts
    // booking 2: interest deposit
    // booking 3: deposit
    QCOMPARE(cont.value(anydate.addMonths(7)), 2005.); // 1.12.2020
    QCOMPARE(getNbrOfBookings(cont.id()), 3);
    latest =cont.latestBooking ();
    QCOMPARE(latest.date, anydate.addMonths(6));
    QCOMPARE(latest.type, bookingType::deposit);

    // payout forces settlement of 2021
    // bookint 4: interest deposit 1.11.2020 - 1.1.2021
    // booking 5: payout of interest (3.34 Euro)
    // booking 6: interest deposit 1.1.2021 - 1.1.2022
    // booking 7: payout of interest (20.05)
    // booking 8: interest deposit 1.1.2022 - 1.5.2022
    // booking 9: payout 100 (4 month -> 6.68 Euro)
    cont.payout(anydate.addYears(2), 100); // 1.5.2022
    QCOMPARE(cont.value(), 1911.68); // verified in excel
    QCOMPARE(getNbrOfBookings(cont.id()), 9);
    latest =cont.latestBooking ();
    QCOMPARE(latest.date, anydate.addYears(2));
    QCOMPARE(latest.type, bookingType::payout);

    // perform settlement of year 2022, 1.1.2023
    cont.annualSettlement(2022);
    QCOMPARE(cont.value(), 1911.68); // no change due to payout
    QCOMPARE(getNbrOfBookings(cont.id()), 11);
    latest =cont.latestBooking ();
    QCOMPARE(latest.date, QDate(2022, 12, 31));
    QCOMPARE(latest.type, bookingType::annualInterestDeposit);
    QCOMPARE(latest.amount, 12.74);
}

void test_contract::test_contract_cv_reInvesting()
{
    creditor c(saveRandomCreditor());
    contract cont(saveRandomContract(c.id()));
    cont.setInterestRate(1.0);
    cont.setInterestModel(interestModel::reinvest);
    cont.updateInterestActive(true);
    QDate anydate = QDate(2020, 4, 30);
    cont.updateConclusionDate(anydate.addDays(-1));

    // booking 1
    cont.bookInitialPayment(anydate, 1000.); // 1.5.2020
    QCOMPARE(cont.value(anydate), 1000.);
    QCOMPARE(cont.value(anydate.addMonths(1)), 1000.); // 1.6.2020
    QCOMPARE(cont.latestBooking().date, anydate);
    QCOMPARE(cont.latestBooking().type, bookingType::deposit);

    cont.deposit(anydate.addMonths(6), 1000.); // 1.11.2020
    // deposit forces interest depoit
    // booking 2: interest deposit
    // booking 3: deposit
    QCOMPARE(cont.value(anydate.addMonths(7)), 2005.); // 1.12.2020
    QCOMPARE(getNbrOfBookings(cont.id()), 3);
    QCOMPARE(cont.latestBooking().date, anydate.addMonths(6));
    QCOMPARE(cont.latestBooking().type, bookingType::deposit);

    // payout forces settlement of 2021
    // bookint 4: interest deposit 1.11.2020 - 1.1.2021 (2005€, 1m -> 3.34€)
    // booking 5: interest deposit 1.1.2021 - 1.1.2022 (2008.34€, 1y -> 20.08€)
    // booking 6: interest deposit 1.1.2022 - 1.5.2022 (2028,42€, 4 month -> 6.76 Euro)
    // booking 7: payout 100
    cont.payout(anydate.addYears(2), 100); // 1.5.2022
    QCOMPARE(cont.value(), 1935.18); // verified in excel
    QCOMPARE(getNbrOfBookings(cont.id()), 7);
    QCOMPARE(cont.latestBooking().date, anydate.addYears(2));
    QCOMPARE(cont.latestBooking().type, bookingType::payout);

    // perform settlement of year 2022, 1.1.2023
    // 7Monate
    // booking 8
    cont.annualSettlement(2022);
    QCOMPARE(cont.value(), 1948.08);
    QCOMPARE(getNbrOfBookings(cont.id()), 8);
    QCOMPARE(cont.latestBooking().date, QDate(2022, 12, 31));
    QCOMPARE(cont.latestBooking().type, bookingType::annualInterestDeposit);
    QCOMPARE(cont.latestBooking().amount, 12.90);
}

void test_contract::test_yearlyMidYearInterestMode()
{
    creditor cred(saveRandomCreditor());

    {
        contract cont(saveRandomContract(cred.id()));
        QCOMPARE(cont.yearlyMidYearInterestMode(2025), contract::undecided);
    }

    {
        contract cont(saveRandomContract(cred.id()));
        QVERIFY(writeBookingToDB(bookingType::reInvestInterest, cont.id(), QDate(2025, 6, 1), 12.34));
        QCOMPARE(cont.yearlyMidYearInterestMode(2025), contract::immediate);
        QCOMPARE(cont.yearlyMidYearInterestMode(2024), contract::undecided);
    }

    {
        contract cont(saveRandomContract(cred.id()));
        QVERIFY(writeBookingToDB(bookingType::deferredMidYearInterest, cont.id(), QDate(2025, 3, 15), 0.));
        QCOMPARE(cont.yearlyMidYearInterestMode(2025), contract::deferred);
        QCOMPARE(cont.yearlyMidYearInterestMode(2026), contract::undecided);
    }

    {
        contract cont(saveRandomContract(cred.id()));
        cont.setInterestRate(1.0);
        cont.setInterestModel(interestModel::reinvest);
        cont.updateInterestActive(false);
        cont.updateConclusionDate(QDate(2025, 1, 14));
        QVERIFY(cont.bookInitialPayment(QDate(2025, 1, 15), 1000.));
        QVERIFY(cont.bookActivateInterest(QDate(2025, 3, 1)));
        QCOMPARE(cont.yearlyMidYearInterestMode(2025), contract::undecided);
    }
}

void test_contract::test_manual_referenceCase_payoutImmediateThenDeferred()
{
    dbConfig::writeValue(ZINSUSANCE, qsl("30/360"));

    creditor cred{saveRandomCreditor()};
    contract cont{saveRandomContract(cred.id())};
    cont.setInterestModel(interestModel::payout);
    cont.setInterestRate(1.5);
    cont.updateInterestActive(true);
    cont.setPlannedInvest(1000.);
    cont.updateConclusionDate(QDate(2024, 12, 1));

    const QDate initialDate{2024, 12, 30};
    const QDate deposit2025{2025, 3, 15}; // 3,13 EUR at 30/360 with 1,5%
    const QDate payout2026{2026, 6, 1};
    const QDate deposit2026{2026, 10, 1};

    QVERIFY(cont.bookInitialPayment(initialDate, 1000.));
    QVERIFY(cont.deposit(deposit2025, 500., true));

    QCOMPARE(cont.getAnnualInterest(2024), 0.0);
    QCOMPARE(cont.value(), 1500.0);
    QCOMPARE(cont.interestBearingValue(), 1500.0);

    const contract::interestBreakdown breakdown2024{cont.interestBreakdownUntilDate(QDate(2024, 12, 31))};
    QVERIFY(breakdown2024.ok);
    QCOMPARE(breakdown2024.slices.size(), 1);
    QCOMPARE(breakdown2024.slices[0].type, contract::interestSlice::kind::annualInterest);
    QCOMPARE(breakdown2024.slices[0].interest, 0.0);
    // todo: should this be 2 sliczes: 8, 2?

    const QVector<booking> bookingsAfterDeposit2025{
        getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"))};
    QCOMPARE(bookingsAfterDeposit2025.size(), 6);
    QCOMPARE(bookingsAfterDeposit2025[0], booking(cont.id(), bookingType::deposit, initialDate, 1000.));
    QCOMPARE(bookingsAfterDeposit2025[1], booking(cont.id(), bookingType::payout, QDate(2024, 12, 31), 0.));
    QCOMPARE(bookingsAfterDeposit2025[2], booking(cont.id(), bookingType::annualInterestDeposit, QDate(2024, 12, 31), 0.));
    QCOMPARE(bookingsAfterDeposit2025[3], booking(cont.id(), bookingType::payout, deposit2025, -3.13));
    QCOMPARE(bookingsAfterDeposit2025[4], booking(cont.id(), bookingType::reInvestInterest, deposit2025, 3.13));
    QCOMPARE(bookingsAfterDeposit2025[5], booking(cont.id(), bookingType::deposit, deposit2025, 500.));

    QCOMPARE(cont.annualSettlement(2025), 2025);
    QCOMPARE(cont.getAnnualInterest(2025), 17.81);

    const contract::interestBreakdown breakdown2025{cont.interestBreakdownUntilDate(QDate(2025, 12, 31))};
    QVERIFY(breakdown2025.ok);
    QCOMPARE(breakdown2025.slices.size(), 2);
    QCOMPARE(breakdown2025.slices[0].recognitionDate, deposit2025);
    QCOMPARE(breakdown2025.slices[0].type, contract::interestSlice::kind::interimInterest);
    // DKV2 uses the prior day as the slice start so that "first day free, last day counts"
    // covers the full 2025 period without losing 01.01.2025.
    QCOMPARE(breakdown2025.slices[0].from, QDate(2024, 12, 31));
    QCOMPARE(breakdown2025.slices[0].to, deposit2025);
    QCOMPARE(breakdown2025.slices[0].contractValue, 1000.0);
    QCOMPARE(breakdown2025.slices[0].baseAmount, 1000.0);
    QCOMPARE(breakdown2025.slices[0].interest, 3.13);
    QCOMPARE(breakdown2025.slices[1].recognitionDate, QDate(2025, 12, 31));
    QCOMPARE(breakdown2025.slices[1].type, contract::interestSlice::kind::annualInterest);
    QCOMPARE(breakdown2025.slices[1].from, deposit2025);
    QCOMPARE(breakdown2025.slices[1].to, QDate(2025, 12, 31));
    QCOMPARE(breakdown2025.slices[1].contractValue, 1500.0);
    QCOMPARE(breakdown2025.slices[1].baseAmount, 1500.0);
    QCOMPARE(breakdown2025.slices[1].interest, 17.81);

    QVERIFY(cont.payout(payout2026, 500., false, contract::deferred));
    const QVector<booking> bookingsAfterPayout2026{
        getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"))};
    QCOMPARE(bookingsAfterPayout2026[8], booking(cont.id(), bookingType::deferredMidYearInterest, payout2026, 0.));
    QCOMPARE(bookingsAfterPayout2026[9], booking(cont.id(), bookingType::payout, payout2026, -500.));

    QVERIFY(cont.deposit(deposit2026, 100.));
    const QVector<booking> bookingsAfterDeposit2026{
        getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"))};
    QCOMPARE(bookingsAfterDeposit2026.size(), 11);
    QCOMPARE(bookingsAfterDeposit2026[10], booking(cont.id(), bookingType::deposit, deposit2026, 100.));

    QCOMPARE(cont.annualSettlement(2026), 2026);
    QCOMPARE(cont.getAnnualInterest(2026), 18.52);

    const contract::interestBreakdown breakdown2026{cont.interestBreakdownUntilDate(QDate(2026, 12, 31))};
    QVERIFY(breakdown2026.ok);
    QCOMPARE(breakdown2026.mode, contract::deferred);
    QCOMPARE(breakdown2026.slices.size(), 4);
    QCOMPARE(breakdown2026.slices[0].type, contract::interestSlice::kind::openingBalance);
    QCOMPARE(breakdown2026.slices[0].interest, 22.50);
    QCOMPARE(breakdown2026.slices[1].type, contract::interestSlice::kind::payout);
    QCOMPARE(breakdown2026.slices[1].interest, -4.35);
    QCOMPARE(breakdown2026.slices[2].type, contract::interestSlice::kind::deposit);
    QCOMPARE(breakdown2026.slices[2].interest, 0.37);
    QCOMPARE(breakdown2026.slices[3].type, contract::interestSlice::kind::payout);
    QCOMPARE(breakdown2026.slices[3].interest, 0.0);
}

void test_contract::test_manual_referenceCase_payoutImmediateThenDeferred_actAct()
{
    dbConfig::writeValue(ZINSUSANCE, qsl("act/act"));

    creditor cred(saveRandomCreditor());
    contract cont(saveRandomContract(cred.id()));
    cont.setInterestRate(1.5);
    cont.setInterestModel(interestModel::payout);
    cont.updateInterestActive(true);

    const QDate conclusionDate{2024, 12, 1};
    const QDate initialDate{2024, 12, 30};
    const QDate deposit2025{2025, 3, 15};
    const QDate payout2026{2026, 6, 1};
    const QDate deposit2026{2026, 10, 1};

    cont.updateConclusionDate(conclusionDate);
    QVERIFY(cont.bookInitialPayment(initialDate, 1000.));
    QVERIFY(cont.deposit(deposit2025, 500., true));

    QCOMPARE(cont.getAnnualInterest(2024), 0.04);
    QCOMPARE(cont.value(), 1500.0);
    QCOMPARE(cont.interestBearingValue(), 1500.0);

    const contract::interestBreakdown breakdown2024{cont.interestBreakdownUntilDate(QDate(2024, 12, 31))};
    QVERIFY(breakdown2024.ok);
    QCOMPARE(breakdown2024.slices.size(), 1);
    QCOMPARE(breakdown2024.slices[0].type, contract::interestSlice::kind::annualInterest);
    QCOMPARE(breakdown2024.slices[0].interest, 0.04);

    const QVector<booking> bookingsAfterDeposit2025{
        getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"))};
    QCOMPARE(bookingsAfterDeposit2025.size(), 6);
    QCOMPARE(bookingsAfterDeposit2025[0], booking(cont.id(), bookingType::deposit, initialDate, 1000.));
    QCOMPARE(bookingsAfterDeposit2025[1], booking(cont.id(), bookingType::payout, QDate(2024, 12, 31), -0.04));
    QCOMPARE(bookingsAfterDeposit2025[2], booking(cont.id(), bookingType::annualInterestDeposit, QDate(2024, 12, 31), 0.04));
    QCOMPARE(bookingsAfterDeposit2025[3], booking(cont.id(), bookingType::payout, deposit2025, -3.04));
    QCOMPARE(bookingsAfterDeposit2025[4], booking(cont.id(), bookingType::reInvestInterest, deposit2025, 3.04));
    QCOMPARE(bookingsAfterDeposit2025[5], booking(cont.id(), bookingType::deposit, deposit2025, 500.));

    QCOMPARE(cont.annualSettlement(2025), 2025);
    QCOMPARE(cont.getAnnualInterest(2025), 17.94);

    const contract::interestBreakdown breakdown2025{cont.interestBreakdownUntilDate(QDate(2025, 12, 31))};
    QVERIFY(breakdown2025.ok);
    QCOMPARE(breakdown2025.slices.size(), 2);
    QCOMPARE(breakdown2025.slices[0].recognitionDate, deposit2025);
    QCOMPARE(breakdown2025.slices[0].type, contract::interestSlice::kind::interimInterest);
    QCOMPARE(breakdown2025.slices[0].from, QDate(2024, 12, 31));
    QCOMPARE(breakdown2025.slices[0].to, deposit2025);
    QCOMPARE(breakdown2025.slices[0].contractValue, 1000.0);
    QCOMPARE(breakdown2025.slices[0].baseAmount, 1000.0);
    QCOMPARE(breakdown2025.slices[0].interest, 3.04);
    QCOMPARE(breakdown2025.slices[1].recognitionDate, QDate(2025, 12, 31));
    QCOMPARE(breakdown2025.slices[1].type, contract::interestSlice::kind::annualInterest);
    QCOMPARE(breakdown2025.slices[1].from, deposit2025);
    QCOMPARE(breakdown2025.slices[1].to, QDate(2025, 12, 31));
    QCOMPARE(breakdown2025.slices[1].contractValue, 1500.0);
    QCOMPARE(breakdown2025.slices[1].baseAmount, 1500.0);
    QCOMPARE(breakdown2025.slices[1].interest, 17.94);

    QVERIFY(cont.payout(payout2026, 500., false, contract::deferred));
    const QVector<booking> bookingsAfterPayout2026{
        getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"))};
    QCOMPARE(bookingsAfterPayout2026[8], booking(cont.id(), bookingType::deferredMidYearInterest, payout2026, 0.));
    QCOMPARE(bookingsAfterPayout2026[9], booking(cont.id(), bookingType::payout, payout2026, -500.));

    QVERIFY(cont.deposit(deposit2026, 100.));
    const QVector<booking> bookingsAfterDeposit2026{
        getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"))};
    QCOMPARE(bookingsAfterDeposit2026.size(), 11);
    QCOMPARE(bookingsAfterDeposit2026[10], booking(cont.id(), bookingType::deposit, deposit2026, 100.));

    QCOMPARE(cont.annualSettlement(2026), 2026);
    QCOMPARE(cont.getAnnualInterest(2026), 18.49);
    QCOMPARE(cont.value(), 1100.0);
    QCOMPARE(cont.interestBearingValue(), 1100.0);

    const contract::interestBreakdown breakdown2026{cont.interestBreakdownUntilDate(QDate(2026, 12, 31))};
    QVERIFY(breakdown2026.ok);
    QCOMPARE(breakdown2026.mode, contract::deferred);
    QCOMPARE(breakdown2026.slices.size(), 4);
    QCOMPARE(breakdown2026.slices[0].type, contract::interestSlice::kind::openingBalance);
    QCOMPARE(breakdown2026.slices[0].interest, 22.50);
    QCOMPARE(breakdown2026.slices[1].type, contract::interestSlice::kind::payout);
    QCOMPARE(breakdown2026.slices[1].interest, -4.38);
    QCOMPARE(breakdown2026.slices[2].type, contract::interestSlice::kind::deposit);
    QCOMPARE(breakdown2026.slices[2].interest, 0.37);
    QCOMPARE(breakdown2026.slices[3].type, contract::interestSlice::kind::payout);
    QCOMPARE(breakdown2026.slices[3].interest, 0.0);
}

void test_contract::test_manual_referenceCase_reinvestImmediateThenDeferred()
{
    dbConfig::writeValue(ZINSUSANCE, qsl("30/360"));

    creditor cred(saveRandomCreditor());
    contract cont(saveRandomContract(cred.id()));
    cont.setInterestRate(1.5);
    cont.setInterestModel(interestModel::reinvest);
    cont.updateInterestActive(true);

    const QDate conclusionDate{2024, 12, 1};
    const QDate initialDate{2024, 12, 30};
    const QDate deposit2025{2025, 3, 15};
    const QDate payout2026{2026, 6, 1};
    const QDate deposit2026{2026, 10, 1};

    cont.updateConclusionDate(conclusionDate);
    QVERIFY(cont.bookInitialPayment(initialDate, 1000.));
    QVERIFY(cont.deposit(deposit2025, 500.));

    QCOMPARE(cont.value(), 1503.13);
    QCOMPARE(cont.interestBearingValue(), 1503.13);

    const contract::interestBreakdown breakdown2024{cont.interestBreakdownUntilDate(QDate(2024, 12, 31))};
    QVERIFY(breakdown2024.ok);
    QCOMPARE(breakdown2024.slices.size(), 1);
    QCOMPARE(breakdown2024.slices[0].type, contract::interestSlice::kind::annualInterest);
    QCOMPARE(breakdown2024.slices[0].interest, 0.0);

    const QVector<booking> bookingsAfterDeposit2025{
        getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"))};
    QCOMPARE(bookingsAfterDeposit2025.size(), 4);
    QCOMPARE(bookingsAfterDeposit2025[0], booking(cont.id(), bookingType::deposit, initialDate, 1000.));
    QCOMPARE(bookingsAfterDeposit2025[1], booking(cont.id(), bookingType::annualInterestDeposit, QDate(2024, 12, 31), 0.0));
    QCOMPARE(bookingsAfterDeposit2025[2], booking(cont.id(), bookingType::reInvestInterest, deposit2025, 3.13));
    QCOMPARE(bookingsAfterDeposit2025[3], booking(cont.id(), bookingType::deposit, deposit2025, 500.));

    QCOMPARE(cont.annualSettlement(2025), 2025);
    QCOMPARE(cont.latestBooking().type, bookingType::annualInterestDeposit);
    QCOMPARE(cont.latestBooking().date, QDate(2025, 12, 31));
    QCOMPARE(cont.latestBooking().amount, 17.85);
    QCOMPARE(cont.value(), 1520.98);
    QCOMPARE(cont.interestBearingValue(), 1520.98);

    const contract::interestBreakdown breakdown2025{cont.interestBreakdownUntilDate(QDate(2025, 12, 31))};
    QVERIFY(breakdown2025.ok);
    QCOMPARE(breakdown2025.slices.size(), 2);
    QCOMPARE(breakdown2025.slices[0].recognitionDate, deposit2025);
    QCOMPARE(breakdown2025.slices[0].type, contract::interestSlice::kind::interimInterest);
    QCOMPARE(breakdown2025.slices[0].from, QDate(2024, 12, 31));
    QCOMPARE(breakdown2025.slices[0].to, deposit2025);
    QCOMPARE(breakdown2025.slices[0].contractValue, 1000.0);
    QCOMPARE(breakdown2025.slices[0].baseAmount, 1000.0);
    QCOMPARE(breakdown2025.slices[0].interest, 3.13);
    QCOMPARE(breakdown2025.slices[1].recognitionDate, QDate(2025, 12, 31));
    QCOMPARE(breakdown2025.slices[1].type, contract::interestSlice::kind::annualInterest);
    QCOMPARE(breakdown2025.slices[1].from, deposit2025);
    QCOMPARE(breakdown2025.slices[1].to, QDate(2025, 12, 31));
    QCOMPARE(breakdown2025.slices[1].contractValue, 1503.13);
    QCOMPARE(breakdown2025.slices[1].baseAmount, 1503.13);
    QCOMPARE(breakdown2025.slices[1].interest, 17.85);

    QVERIFY(cont.payout(payout2026, 500., false, contract::deferred));
    const QVector<booking> bookingsAfterPayout2026{
        getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"))};
    QCOMPARE(bookingsAfterPayout2026[5], booking(cont.id(), bookingType::deferredMidYearInterest, payout2026, 0.));
    QCOMPARE(bookingsAfterPayout2026[6], booking(cont.id(), bookingType::payout, payout2026, -500.));

    QVERIFY(cont.deposit(deposit2026, 100.));
    const QVector<booking> bookingsAfterDeposit2026{
        getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"))};
    QCOMPARE(bookingsAfterDeposit2026.size(), 8);
    QCOMPARE(bookingsAfterDeposit2026[7], booking(cont.id(), bookingType::deposit, deposit2026, 100.));

    QCOMPARE(cont.annualSettlement(2026), 2026);
    QCOMPARE(cont.latestBooking().type, bookingType::annualInterestDeposit);
    QCOMPARE(cont.latestBooking().date, QDate(2026, 12, 31));
    QCOMPARE(cont.latestBooking().amount, 18.83);
    QCOMPARE(cont.value(), 1139.81);
    QCOMPARE(cont.interestBearingValue(), 1139.81);

    const contract::interestBreakdown breakdown2026{cont.interestBreakdownUntilDate(QDate(2026, 12, 31))};
    QVERIFY(breakdown2026.ok);
    QCOMPARE(breakdown2026.mode, contract::deferred);
    QCOMPARE(breakdown2026.slices.size(), 3);
    QCOMPARE(breakdown2026.slices[0].type, contract::interestSlice::kind::openingBalance);
    QCOMPARE(breakdown2026.slices[0].interest, 22.81);
    QCOMPARE(breakdown2026.slices[1].type, contract::interestSlice::kind::payout);
    QCOMPARE(breakdown2026.slices[1].interest, -4.35);
    QCOMPARE(breakdown2026.slices[2].type, contract::interestSlice::kind::deposit);
    QCOMPARE(breakdown2026.slices[2].interest, 0.37);
}

void test_contract::test_manual_referenceCases_data()
{
    QTest::addColumn<referenceCase>("c");
    qRegisterMetaType<referenceCase>("referenceCase");

    const QDate conclusionDate{2024, 12, 1};
    const QDate initialDate{2024, 12, 30};
    const QDate deposit2025{2025, 3, 15};
    const QDate payout2026{2026, 6, 1};
    const QDate deposit2026{2026, 10, 1};

    {
        referenceCase c;
        c.zinsusance = qsl("30/360");
        c.model = interestModel::payout;
        c.rate = 1.5;
        c.conclusionDate = conclusionDate;
        c.initialPaymentDate = initialDate;
        c.initialPaymentAmount = 1000.0;
        c.steps = {
            {referenceBookingStep::kind::deposit, deposit2025, 500.0, true},
            {referenceBookingStep::kind::annualSettlement, {}, 0.0, false, contract::undecided, 2025},
            {referenceBookingStep::kind::payout, payout2026, 500.0, false, contract::deferred},
            {referenceBookingStep::kind::deposit, deposit2026, 100.0},
            {referenceBookingStep::kind::annualSettlement, {}, 0.0, false, contract::undecided, 2026}
        };
        c.expectedBookings = {
            {bookingType::deposit, initialDate, 1000.0},
            {bookingType::payout, QDate(2024, 12, 31), 0.0},
            {bookingType::annualInterestDeposit, QDate(2024, 12, 31), 0.0},
            {bookingType::payout, deposit2025, -3.13},
            {bookingType::reInvestInterest, deposit2025, 3.13},
            {bookingType::deposit, deposit2025, 500.0},
            {bookingType::payout, QDate(2025, 12, 31), -17.81},
            {bookingType::annualInterestDeposit, QDate(2025, 12, 31), 17.81},
            {bookingType::deferredMidYearInterest, payout2026, 0.0},
            {bookingType::payout, payout2026, -500.0},
            {bookingType::deposit, deposit2026, 100.0},
            {bookingType::payout, QDate(2026, 12, 31), -18.52},
            {bookingType::annualInterestDeposit, QDate(2026, 12, 31), 18.52}
        };
        c.expectedBreakdowns = {
            {QDate(2024, 12, 31), contract::immediate, {contract::interestSlice::kind::annualInterest}, {0.0}},
            {QDate(2025, 12, 31), contract::immediate, {contract::interestSlice::kind::interimInterest, contract::interestSlice::kind::annualInterest}, {3.13, 17.81}},
            {QDate(2026, 12, 31), contract::deferred, {contract::interestSlice::kind::openingBalance, contract::interestSlice::kind::payout, contract::interestSlice::kind::deposit, contract::interestSlice::kind::payout}, {22.50, -4.35, 0.37, 0.0}}
        };
        QTest::newRow("payout_30_360") << c;
    }

    {
        const double annual2024{ZinsesZins_act_act(1.5, 1000.0, initialDate, QDate(2024, 12, 31), false)};
        const double interim2025{ZinsesZins_act_act(1.5, 1000.0, QDate(2024, 12, 31), deposit2025, false)};
        const double annual2025{ZinsesZins_act_act(1.5, 1500.0, deposit2025, QDate(2025, 12, 31), false)};
        const double opening2026{ZinsesZins_act_act(1.5, 1500.0, QDate(2025, 12, 31), QDate(2026, 12, 31), false)};
        const double payoutSlice2026{-ZinsesZins_act_act(1.5, 500.0, payout2026, QDate(2026, 12, 31), false)};
        const double depositSlice2026{ZinsesZins_act_act(1.5, 100.0, deposit2026, QDate(2026, 12, 31), false)};
        const double annual2026{r2(opening2026 + payoutSlice2026 + depositSlice2026)};

        referenceCase c;
        c.zinsusance = qsl("act/act");
        c.model = interestModel::payout;
        c.rate = 1.5;
        c.conclusionDate = conclusionDate;
        c.initialPaymentDate = initialDate;
        c.initialPaymentAmount = 1000.0;
        c.steps = {
            {referenceBookingStep::kind::deposit, deposit2025, 500.0, true},
            {referenceBookingStep::kind::annualSettlement, {}, 0.0, false, contract::undecided, 2025},
            {referenceBookingStep::kind::payout, payout2026, 500.0, false, contract::deferred},
            {referenceBookingStep::kind::deposit, deposit2026, 100.0},
            {referenceBookingStep::kind::annualSettlement, {}, 0.0, false, contract::undecided, 2026}
        };
        c.expectedBookings = {
            {bookingType::deposit, initialDate, 1000.0},
            {bookingType::payout, QDate(2024, 12, 31), -annual2024},
            {bookingType::annualInterestDeposit, QDate(2024, 12, 31), annual2024},
            {bookingType::payout, deposit2025, -interim2025},
            {bookingType::reInvestInterest, deposit2025, interim2025},
            {bookingType::deposit, deposit2025, 500.0},
            {bookingType::payout, QDate(2025, 12, 31), -annual2025},
            {bookingType::annualInterestDeposit, QDate(2025, 12, 31), annual2025},
            {bookingType::deferredMidYearInterest, payout2026, 0.0},
            {bookingType::payout, payout2026, -500.0},
            {bookingType::deposit, deposit2026, 100.0},
            {bookingType::payout, QDate(2026, 12, 31), -annual2026},
            {bookingType::annualInterestDeposit, QDate(2026, 12, 31), annual2026}
        };
        c.expectedBreakdowns = {
            {QDate(2024, 12, 31), contract::immediate, {contract::interestSlice::kind::annualInterest}, {annual2024}},
            {QDate(2025, 12, 31), contract::immediate, {contract::interestSlice::kind::interimInterest, contract::interestSlice::kind::annualInterest}, {interim2025, annual2025}},
            {QDate(2026, 12, 31), contract::deferred, {contract::interestSlice::kind::openingBalance, contract::interestSlice::kind::payout, contract::interestSlice::kind::deposit, contract::interestSlice::kind::payout}, {opening2026, payoutSlice2026, depositSlice2026, 0.0}}
        };
        QTest::newRow("payout_act_act") << c;
    }

    {
        const double annual2024{ZinsesZins_30_360(1.5, 1000.0, initialDate, QDate(2024, 12, 31), true)};
        const double interim2025{ZinsesZins_30_360(1.5, 1000.0, QDate(2024, 12, 31), deposit2025, true)};
        const double valueAfterDeposit2025{r2(1000.0 + interim2025 + 500.0)};
        const double annual2025{ZinsesZins_30_360(1.5, valueAfterDeposit2025, deposit2025, QDate(2025, 12, 31), true)};
        const double valueAtStart2026{r2(valueAfterDeposit2025 + annual2025)};
        const double opening2026{ZinsesZins_30_360(1.5, valueAtStart2026, QDate(2025, 12, 31), QDate(2026, 12, 31), true)};
        const double payoutSlice2026{-ZinsesZins_30_360(1.5, 500.0, payout2026, QDate(2026, 12, 31), true)};
        const double depositSlice2026{ZinsesZins_30_360(1.5, 100.0, deposit2026, QDate(2026, 12, 31), true)};

        referenceCase c;
        c.zinsusance = qsl("30/360");
        c.model = interestModel::reinvest;
        c.rate = 1.5;
        c.conclusionDate = conclusionDate;
        c.initialPaymentDate = initialDate;
        c.initialPaymentAmount = 1000.0;
        c.steps = {
            {referenceBookingStep::kind::deposit, deposit2025, 500.0},
            {referenceBookingStep::kind::annualSettlement, {}, 0.0, false, contract::undecided, 2025},
            {referenceBookingStep::kind::payout, payout2026, 500.0, false, contract::deferred},
            {referenceBookingStep::kind::deposit, deposit2026, 100.0},
            {referenceBookingStep::kind::annualSettlement, {}, 0.0, false, contract::undecided, 2026}
        };
        c.expectedBookings = {
            {bookingType::deposit, initialDate, 1000.0},
            {bookingType::annualInterestDeposit, QDate(2024, 12, 31), annual2024},
            {bookingType::reInvestInterest, deposit2025, interim2025},
            {bookingType::deposit, deposit2025, 500.0},
            {bookingType::annualInterestDeposit, QDate(2025, 12, 31), annual2025},
            {bookingType::deferredMidYearInterest, payout2026, 0.0},
            {bookingType::payout, payout2026, -500.0},
            {bookingType::deposit, deposit2026, 100.0},
            {bookingType::annualInterestDeposit, QDate(2026, 12, 31), r2(opening2026 + payoutSlice2026 + depositSlice2026)}
        };
        c.expectedBreakdowns = {
            {QDate(2024, 12, 31), contract::immediate, {contract::interestSlice::kind::annualInterest}, {annual2024}},
            {QDate(2025, 12, 31), contract::immediate, {contract::interestSlice::kind::interimInterest, contract::interestSlice::kind::annualInterest}, {interim2025, annual2025}},
            {QDate(2026, 12, 31), contract::deferred, {contract::interestSlice::kind::openingBalance, contract::interestSlice::kind::payout, contract::interestSlice::kind::deposit}, {opening2026, payoutSlice2026, depositSlice2026}}
        };
        QTest::newRow("reinvest_30_360") << c;
    }
}

void test_contract::test_manual_referenceCases()
{
    QFETCH(referenceCase, c);
    runReferenceCase(c);
}

void test_contract::test_deferredAnnualSettlement_usesYearSlices()
{
    creditor cred(saveRandomCreditor());
    contract cont(saveRandomContract(cred.id()));
    cont.setInterestRate(2.0);
    cont.setInterestModel(interestModel::reinvest);
    cont.updateInterestActive(true);
    cont.updateConclusionDate(QDate(2019, 12, 31));

    const QDate initialDate(2020, 1, 1);
    const QDate decisionDate(2020, 6, 1);
    const QDate depositDate(2020, 7, 1);

    QVERIFY(cont.bookInitialPayment(initialDate, 1000.));
    QVERIFY(bookDeferredInBetweenInterest(cont.id(), decisionDate));
    QVERIFY(cont.deposit(depositDate, 1000.));

    const double expectedInterestRaw =
            interestForPeriod(cont.actualInterestRate(), 1000., initialDate, QDate(2020, 12, 31))
            + interestForPeriod(cont.actualInterestRate(), 1000., depositDate, QDate(2020, 12, 31));
    const double expectedInterest = euroFromCt(ctFromEuro(expectedInterestRaw));

    QCOMPARE(cont.annualSettlement(2020), 2020);
    QCOMPARE(cont.latestBooking().type, bookingType::annualInterestDeposit);
    QCOMPARE(cont.latestBooking().date, QDate(2020, 12, 31));
    QCOMPARE(cont.latestBooking().amount, expectedInterest);
    QCOMPARE(cont.value(), 2000. + expectedInterest);

    const QVariantMap vm = cont.toVariantMap(QDate(2020, 1, 1), QDate(2020, 12, 31));
    QCOMPARE(vm.value(qsl("dJahresZinsen")).toDouble(), expectedInterest);
    QCOMPARE(vm.value(qsl("dSonstigeZinsen")).toDouble(), 0.);
}

void test_contract::test_deferredAnnualSettlement_failsOnUnexpectedInterimInterest()
{
    creditor cred(saveRandomCreditor());
    contract cont(saveRandomContract(cred.id()));
    cont.setInterestRate(2.0);
    cont.setInterestModel(interestModel::reinvest);
    cont.updateInterestActive(true);
    cont.updateConclusionDate(QDate(2019, 12, 31));

    QVERIFY(cont.bookInitialPayment(QDate(2020, 1, 1), 1000.));
    QVERIFY(bookDeferredInBetweenInterest(cont.id(), QDate(2020, 6, 1)));
    QVERIFY(writeBookingToDB(bookingType::reInvestInterest, cont.id(), QDate(2020, 7, 1), 10.));

    QCOMPARE(cont.annualSettlement(2020), 0);
}

void test_contract::test_deferredMidYearInterestSkipsInBetweenInterestBooking()
{
    creditor cred(saveRandomCreditor());
    contract cont(saveRandomContract(cred.id()));
    cont.setInterestRate(1.0);
    cont.setInterestModel(interestModel::reinvest);
    cont.updateInterestActive(true);

    const QDate initialDate(2020, 1, 15);
    const QDate decisionDate(2020, 6, 1);
    const QDate depositDate(2020, 7, 1);

    cont.updateConclusionDate(initialDate.addDays(-1));
    QVERIFY(cont.bookInitialPayment(initialDate, 1000.));
    QVERIFY(bookDeferredInBetweenInterest(cont.id(), decisionDate));

    QCOMPARE(cont.yearlyMidYearInterestMode(2020), contract::deferred);
    QVERIFY(cont.deposit(depositDate, 1000.));

    const QVector<booking> bookings = getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("Datum ASC"));
    QCOMPARE(bookings.size(), 3);
    QCOMPARE(bookings[0], booking(cont.id(), bookingType::deposit, initialDate, 1000.));
    QCOMPARE(bookings[1], booking(cont.id(), bookingType::deferredMidYearInterest, decisionDate, 0.));
    QCOMPARE(bookings[2], booking(cont.id(), bookingType::deposit, depositDate, 1000.));
    QCOMPARE(cont.value(), 2000.);
}

void test_contract::test_deferredMidYearInterestDoesNotSkipActivationBoundaryBooking()
{
    creditor cred(saveRandomCreditor());
    contract cont(saveRandomContract(cred.id()));
    cont.setInterestRate(1.0);
    cont.setInterestModel(interestModel::reinvest);
    cont.updateInterestActive(false);

    const QDate initialDate(2020, 1, 15);
    const QDate decisionDate(2020, 6, 1);
    const QDate activationDate(2020, 7, 1);

    cont.updateConclusionDate(initialDate.addDays(-1));
    QVERIFY(cont.bookInitialPayment(initialDate, 1000.));
    QVERIFY(bookDeferredInBetweenInterest(cont.id(), decisionDate));
    QVERIFY(cont.bookActivateInterest(activationDate));

    const QVector<booking> bookings = getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"));
    QCOMPARE(bookings.size(), 4);
    QCOMPARE(bookings[0], booking(cont.id(), bookingType::deposit, initialDate, 1000.));
    QCOMPARE(bookings[1], booking(cont.id(), bookingType::deferredMidYearInterest, decisionDate, 0.));
    QCOMPARE(bookings[2], booking(cont.id(), bookingType::reInvestInterest, activationDate, 0.));
    QCOMPARE(bookings[3], booking(cont.id(), bookingType::setInterestActive, activationDate, 0.));
}

void test_contract::test_firstDeferredBookingOfNewYear_keepsPriorAnnualSettlement()
{
    creditor cred(saveRandomCreditor());
    contract cont(saveRandomContract(cred.id()));
    cont.setInterestRate(1.0);
    cont.setInterestModel(interestModel::reinvest);
    cont.updateInterestActive(true);

    const QDate initialDate(2025, 1, 1);
    const QDate deposit2025(2025, 6, 1);
    const QDate deposit2026(2026, 4, 13);

    cont.updateConclusionDate(initialDate.addDays(-1));
    QVERIFY(cont.bookInitialPayment(initialDate, 1000.));
    QVERIFY(cont.deposit(deposit2025, 100.));

    QVector<booking> bookings = getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"));
    QCOMPARE(bookings.size(), 3);
    QVERIFY(cont.deposit(deposit2026, 50., false, contract::deferred));

    QCOMPARE(cont.yearlyMidYearInterestMode(2026), contract::deferred);
    bookings = getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"));

    QCOMPARE(bookings.size(), 6);
    QCOMPARE(bookings[0], booking(cont.id(), bookingType::deposit, initialDate, 1000.));
    QCOMPARE(bookings[1], booking(cont.id(), bookingType::reInvestInterest, deposit2025, 4.17));
    QCOMPARE(bookings[2], booking(cont.id(), bookingType::deposit, deposit2025, 100.));
    QCOMPARE(bookings[3].type, bookingType::annualInterestDeposit);
    QCOMPARE(bookings[3].date, QDate(2025, 12, 31));
    QVERIFY(bookings[3].amount > 0.0);
    QCOMPARE(bookings[4], booking(cont.id(), bookingType::deferredMidYearInterest, deposit2026, 0.));
    QCOMPARE(bookings[5], booking(cont.id(), bookingType::deposit, deposit2026, 50.));
}

void test_contract::test_finalize_deferredMidYearInterest()
{
    creditor cred(saveRandomCreditor());
    contract cont(saveRandomContract(cred.id()));
    cont.setInterestRate(2.0);
    cont.setInterestModel(interestModel::reinvest);
    cont.updateInterestActive(true);
    cont.updateConclusionDate(QDate(2019, 12, 31));

    const QDate initialDate(2020, 1, 1);
    const QDate decisionDate(2020, 6, 1);
    const QDate depositDate(2020, 7, 1);
    const QDate finalDate(2020, 9, 30);

    QVERIFY(cont.bookInitialPayment(initialDate, 1000.));
    QVERIFY(bookDeferredInBetweenInterest(cont.id(), decisionDate));
    QVERIFY(cont.deposit(depositDate, 1000.));

    const double expectedFinalInterestRaw =
            interestForPeriod(cont.actualInterestRate(), 1000., initialDate, finalDate)
            + interestForPeriod(cont.actualInterestRate(), 1000., depositDate, finalDate);
    const double expectedFinalInterest = euroFromCt(ctFromEuro(expectedFinalInterestRaw));

    const contractId_t contractId = cont.id();
    double finInterest = 0.;
    double finPayout = 0.;
    QVERIFY(cont.finalize(false, finalDate, finInterest, finPayout));
    QCOMPARE(finInterest, expectedFinalInterest);
    QCOMPARE(finPayout, 2000. + expectedFinalInterest);
    QCOMPARE(getNbrOfExBookings(contractId), 5);

    contract ex(contractId, true);
    QCOMPARE(ex.payedInterestAtTermination(), expectedFinalInterest);
}

void test_contract::test_finalize()
{
    creditor creditor(saveRandomCreditor());
    contract cont(saveRandomContract(creditor.id()));
    cont.updateInterestActive(true);
    QDate aDate = QDate(2020, 5, 1);
    cont.updateConclusionDate(aDate.addDays(-4));

    cont.bookInitialPayment(aDate, 1000.);
    cont.deposit(aDate.addMonths(1), 1000.);
    QCOMPARE(rowCount("Vertraege"), 1);
    QCOMPARE(rowCount("Buchungen"), 3);
    contractId_t contractId = cont.id();
    double fi =0., fp =0.;
    QVERIFY(cont.finalize(false, aDate.addMonths(2), fi, fp));
    // finalize should reset the cont object
    QCOMPARE(cont.id(), Invalid_contract_id);
    QCOMPARE(rowCount("Vertraege"), 0);
    QCOMPARE(getNbrOfBookings(contractId), 0);
    QCOMPARE(rowCount("exVertraege"), 1);
    QCOMPARE( getNbrOfExBookings(contractId), 5);
    QCOMPARE(executeSingleValueSql(
             contract::getTableDef_deletedContracts()["LaufzeitEnde"],
             "id=" +i2s(contractId.v)), QDate(aDate.addMonths(2)));
    contract ex(contractId, true);
    QCOMPARE(ex.value (), 0.);
}

void test_contract::test_readExContract()
{
    creditor c(saveRandomCreditor ());
    contract cont(saveRandomContract(c.id ()));
}
