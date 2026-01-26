#include "test_annualsettlement.h"
#include "annualSettlement.h"

#include "creditor.h"
#include "contract.h"
#include "testhelper.h"

#include <QtTest/QTest>

contract newContract()
{
    return saveRandomContract(saveRandomCreditor().id());
}

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

void test_annualSettlement::test_oneContract_InitMidYear_onContract()
{
    contract c{newContract()};
    QDate in1990(1990,6,15);
    c.bookInitialPayment(in1990, 1000);
    QDate yearEnd1990(1990,12,31);
    // TEST
    QCOMPARE(c.annualSettlement(1989), 0);
    QCOMPARE(yearEnd1990, dateOfnextSettlement());
    QCOMPARE(c.annualSettlement(1990), 1990);
    // second annual settlement
    QCOMPARE(c.annualSettlement(1991), 1991);
}

void test_annualSettlement::test_oneContract_InitOnYearEnd_onContract()
{
    contract c{newContract()};
    QDate yearEnd1990(1990,12,31);
    c.bookInitialPayment(yearEnd1990, 1000);
    // TEST
    QDate yearEnd1991(1991,12,31);
    QCOMPARE(yearEnd1991, dateOfnextSettlement());
    QCOMPARE(c.annualSettlement(1990), 0);
    QCOMPARE(c.annualSettlement(1991), 1991);
    QCOMPARE(c.annualSettlement(1992), 1992);
}

void test_annualSettlement::test_oneContract_InitMidYear_globally()
{
    contract c{newContract()};
    QDate in1990(1990,6,15);
    c.bookInitialPayment(in1990, 1000);
    QDate yearEnd1990(1990,12,31);
    // TEST
    QVERIFY(not executeAnnualSettlement(1989));
    QCOMPARE(yearEnd1990, dateOfnextSettlement());
    QVERIFY(executeAnnualSettlement(1990));
    // second annual settlement
    QVERIFY(executeAnnualSettlement(1991));
}

void test_annualSettlement::test_oneContract_InitOnYearEnd_globally()
{
    contract c{newContract()};
    QDate almostYearEnd(1990,12,30);
    c.bookInitialPayment(almostYearEnd, 1000);
    // TEST
    QDate yearEnd1991(1991,12,31);
    //QCOMPARE(yearEnd1991, dateOfnextSettlement());
    QCOMPARE(QDate(1990,12,31), dateOfnextSettlement());
    QVERIFY(executeAnnualSettlement(1989));
    QVERIFY(executeAnnualSettlement(1990));
    QVERIFY(executeAnnualSettlement(1991));
}
