#ifndef TEST_GELDANLAGEN
#define TEST_GELDANLAGEN

#include "../DKV2/creditor.h"
#include "../DKV2/investment.h"
#include "../DKV2/dkdbviews.h"
#include "../DKV2/helpersql.h"
#include "testhelper.h"
#include "test_geldanlagen.h"

test_geldanlagen::test_geldanlagen()
{
}

void test_geldanlagen::init()
{
    initTestDkDb_InMemory ();
}

void test_geldanlagen::cleanup()
{
    cleanupTestDb_InMemory();
}

void test_geldanlagen::test_ohneAnlagen()
{
    QVector<QSqlRecord> resultset;
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    QCOMPARE(0, resultset.size());
}

void test_geldanlagen::test_kontinuierlicheAnlagen()
{
    // prep: to have exactly one investment (1%, floating)
    QCOMPARE(1, saveNewFloatingInvestment (100, qsl("investment 1")));
    QVector<QSqlRecord> resultset;
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(1, resultset.size());
    QCOMPARE(resultset[0].value (qsl("Anz. laufende V. (mit/ ohne Geldeingang)")), qsl("0 (0/0)"));
    QCOMPARE(resultset[0].value (qsl("Anz. beendete Verträge")).toInt(), 0);
    QCOMPARE(resultset[0].value (qsl("Gesamtbetrag")), qsl(" 0.00 € "));

    //prep: another investment created, lower interest
    resultset.clear ();
    QCOMPARE(2, saveNewFloatingInvestment (90, qsl("investment 2")));
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    QCOMPARE(resultset[1].value (qsl("Anz. laufende V. (mit/ ohne Geldeingang)")), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (qsl("Anz. beendete Verträge")).toInt(), 0);
    QCOMPARE(resultset[1].value (qsl("Gesamtbetrag")), qsl(" 0.00 € "));

    // prep
    resultset.clear();
    closeInvestment(1);
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    QCOMPARE(resultset[0].value (qsl("Anz. laufende V. (mit/ ohne Geldeingang)")), qsl("0 (0/0)"));
    QCOMPARE(resultset[0].value (qsl("Anz. beendete Verträge")).toInt(), 0);
    QCOMPARE(resultset[0].value (qsl("Gesamtbetrag")), qsl(" 0.00 € "));
    QCOMPARE(resultset[0].value (qsl("(Ab-)Geschlossen")), qsl("Ja"));
    QCOMPARE(resultset[1].value (qsl("Anz. laufende V. (mit/ ohne Geldeingang)")), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (qsl("Anz. beendete Verträge")).toInt(), 0);
    QCOMPARE(resultset[1].value (qsl("Gesamtbetrag")), qsl(" 0.00 € "));
    QCOMPARE(resultset[1].value (qsl("(Ab-)Geschlossen")), qsl("Nein"));

    // perp
    resultset.clear ();
    contract _contract;
    _contract.setCreditorId (saveRandomCreditor ().id ());
    _contract.setConclusionDate (QDate::currentDate ().addMonths (1));
    _contract.setPlannedInvest (1000.);
    _contract.setInterestModel (interestModel::reinvest);
    _contract.setInterestRate (0.9);
    _contract.setInvestmentId (2);
    QCOMPARE( 1, _contract.saveNewContract ());
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(2, resultset.size());
    QCOMPARE(resultset[1].value (qsl("Anz. laufende V. (mit/ ohne Geldeingang)")), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (qsl("Anz. beendete Verträge")).toInt(), 0);
    QCOMPARE(resultset[1].value (qsl("Gesamtbetrag")), qsl(" 0.00 € "));
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (qsl("Anz. laufende V. (mit/ ohne Geldeingang)")), qsl("1 (0/1)"));
    QCOMPARE(resultset[0].value (qsl("Anz. beendete Verträge")).toInt(), 0);
    QCOMPARE(resultset[0].value (qsl("Gesamtbetrag")), qsl(" 1000.00 € "));





    dbgDumpDB ();

}




#endif // TEST_GELDANLAGEN
