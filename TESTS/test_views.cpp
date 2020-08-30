#include <QtTest>

#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"

#include "test_views.h"

void test_views::initTestCase()
{
    init_DKDBStruct();
}
void test_views::cleanupTestCase()
{

}
void test_views::init()
{
    initTestDb();
    QVERIFY(create_DK_TablesAndContent());
}
void test_views::cleanup()
{
    cleanupTestDb();
}

void test_views::test_wertPassiveVertraege_oneContract()
{
    creditor c(saveRandomCreditor());
    contract cont;
    cont.initRandom(c.id());
    double v =1000.;
    cont.setPlannedInvest(v);
    cont.setReinvesting(true);
    cont.saveNewContract();
    DbSummary dbs;


    // passive contract
    QCOMPARE(1, numberOfInactiveContracts());
    QCOMPARE(v, valueOfInactiveContracts());
    QCOMPARE(0., weightedAverageInterestRate());
    QCOMPARE(0., avgInterestRate());
    QCOMPARE(0, numberOfActiveContracts());
    QCOMPARE(0., valueOfActiveContracts());
    QCOMPARE(0, numberOfActiveAccumulatingContracts());
    QCOMPARE(0., valueOfActiveAccumulatingContracts());
    QCOMPARE(0, numberOfActivePayoutContracts());
    QCOMPARE(0., valueOfActivePayoutContracts());

    cont.activate(QDate::currentDate(), cont.plannedInvest());
    double i =cont.interestRate();
    QCOMPARE(0,  numberOfInactiveContracts());
    QCOMPARE(0., valueOfInactiveContracts());
    QCOMPARE(i,  weightedAverageInterestRate());
    QCOMPARE(i,  avgInterestRate());
    QCOMPARE(1,  numberOfActiveContracts());
    QCOMPARE(v,  valueOfActiveContracts());
    QCOMPARE(cont.reinvesting()?1:0, numberOfActiveAccumulatingContracts());
    QCOMPARE(cont.reinvesting()?v:0., valueOfActiveAccumulatingContracts());
    QCOMPARE(cont.reinvesting()?0:1, numberOfActivePayoutContracts());
    QCOMPARE(cont.reinvesting()?0.:v, valueOfActivePayoutContracts());
}
