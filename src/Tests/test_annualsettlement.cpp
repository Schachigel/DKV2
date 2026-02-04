#include "test_annualsettlement.h"
#include "annualSettlement.h"

#include "creditor.h"
#include "contract.h"
#include "testhelper.h"

#include <QtTest/QTest>

void test_annualSettlement::init()
{
    initTestDkDb_InMemory();
}

void test_annualSettlement::cleanup()
{
    cleanupTestDb_InMemory();
}

void test_annualSettlement::test_noContract_noAS()
{
    QCOMPARE(QDate(), dateOfnextSettlement());
}

void test_annualSettlement::test_oneContract_Mid_Year()
{
    // prep one contract
    saveRandomCreditor();
    contract c;
    c.setCreditorId(1);
    c.setInterestRate(10.f);
    c.setInterestActive(true);
    QDate cDate (2000, 6, 15);
    c.setConclusionDate(cDate.addDays(15));
    c.saveNewContract();
    // TEST: no AS
    QDate yearEnd2000(2000,12,31);
    QCOMPARE(QDate(), dateOfnextSettlement());
    QCOMPARE(executeCompleteAS(yearEnd2000.year()), 0);
    // init payment
    c.bookInitialPayment(cDate.addDays(15), 1000);
    // TEST first AS
    QCOMPARE(yearEnd2000, dateOfnextSettlement());
    QCOMPARE(executeCompleteAS(yearEnd2000.year()), 1);
    // TEST second AS
    QDate yearEnd2001 =yearEnd2000.addYears(1);
    QCOMPARE(yearEnd2001, dateOfnextSettlement());
    QCOMPARE(executeCompleteAS(yearEnd2001.year()), 1);
}

void test_annualSettlement::test_contract_intrest_activation()
{
    // prep one contract
    saveRandomCreditor();
    contract c;
    c.setCreditorId(1);
    c.setInterestRate(10.f);
    c.setInterestActive(false);
    c.setLabel(qsl("Vertrag 001"));
    QDate cDate (2000, 6, 15);
    c.setConclusionDate(cDate.addDays(15));
    c.saveNewContract();

    QDate yearEnd2000(2000,12,31);
    // TEST: zActive false -> no AS
    QCOMPARE(QDate(), dateOfnextSettlement());
    QCOMPARE(executeCompleteAS(yearEnd2000.year()), 0);
    // interest activation fails w/o init payment
    QVERIFY(not c.bookActivateInterest(cDate.addMonths(1)));
    // TEST: with initial payment
    c.bookInitialPayment(cDate.addDays(15), 1000);
    // interest activation after initial payment
    QVERIFY(c.bookActivateInterest(cDate.addMonths(1)));
    QCOMPARE(yearEnd2000, dateOfnextSettlement());
    QCOMPARE(executeCompleteAS(yearEnd2000.year()), 1);
    qInfo().noquote().nospace() << "\n" << formulate_AS_as_CSV(yearEnd2000.year());
dbgDumpDB(); // todo: remove
}

void test_annualSettlement::test_contract_intrest_activation_yearEnd()
{
    // prep one contract
    saveRandomCreditor();
    contract c;
    c.setCreditorId(1);
    c.setInterestRate(10.f);
    c.setInterestActive(false);
    QDate cDate (2000, 11, 15);
    c.setConclusionDate(cDate);
    c.saveNewContract();
    // init payment
    c.bookInitialPayment(cDate.addMonths(1), 1000);
    // act. interest
    c.bookActivateInterest(QDate(2000, 12, 31));
    // AS should work in 2001
    QDate yearEnd2001(2001, 12, 31);
    QCOMPARE(yearEnd2001, dateOfnextSettlement());
    QCOMPARE(executeCompleteAS(yearEnd2001.addYears(-1).year()), 0);
    QCOMPARE(executeCompleteAS(yearEnd2001.year()), 1);
}

void test_annualSettlement::test_multipleContracts()
{
    // prep creditors and contracts
    saveRandomCreditor();
    saveRandomCreditor();
    struct test_contract {
        tableindex_t creditor;
        double amount;
        double interestRate;
        interestModel iModel;
        bool interestActive;
        QDate conclusionDate;
    };

    QVector<test_contract>  contractData {
        {1, 1000., 10., interestModel::reinvest, true,  QDate(2000, 1,  1)},
        {2, 2000.,  1., interestModel::payout,   false, QDate(2000, 2,  1)},
        {1, 3000.,  5., interestModel::fixed,    true,  QDate(2001, 1, 22)},
        {2, 1500., 10., interestModel::zero,     false, QDate(2001, 4,  5)},
        {1, 2500.,  1., interestModel::reinvest, true,  QDate(2002, 3, 15)},
        {2, 3500.,  5., interestModel::payout,   false, QDate(2002, 6, 22)},
        {1, 3000.,  5., interestModel::fixed,    true,  QDate(2003, 3,  5)},
        {2, 1000., 10., interestModel::zero,     false, QDate(2003, 8,  7)},
        };

    int i {0};
    for (const auto& dataset : contractData) {
        contract c1;
        c1.setCreditorId(dataset.creditor);
        c1.setInterestRate(dataset.interestRate);
        c1.setInterestActive(dataset.interestActive);
        c1.setInterestModel(dataset.iModel);
        c1.setConclusionDate(dataset.conclusionDate);
        c1.setLabel(qsl("Vertrag #%1").arg(QString::number(i++)));
        c1.saveNewContract();
    }


}
