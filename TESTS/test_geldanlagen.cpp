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

const QString fnAnzahlLaufende {qsl("Anz. laufende V. (mit/ ohne Geldeingang)")};
const QString fnAnzahlBeendete {qsl("Anz. beendete Verträge")};
const QString fnGesamtbetrag   {qsl("Gesamtbetrag")};
const QString fnAbGeschlossen  {qsl("(Ab-)Geschlossen")};

void test_geldanlagen::test_kontinuierlicheAnlagen()
{
// prep: to have exactly one investment (1%, floating)
{
    QCOMPARE(1, saveNewFloatingInvestment (100, qsl("investment 1")));
    QVector<QSqlRecord> resultset;
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(1, resultset.size());
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" 0.00 € "));
}

//prep: another investment created, lower interest 0.9%
{
    QVector<QSqlRecord> resultset;
    QCOMPARE(2, saveNewFloatingInvestment (90, qsl("investment 2")));
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" 0.00 € "));
}

// prep: one of the investments is closed and listed as such
{
    QVector<QSqlRecord> resultset;
    closeInvestment(1);
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" 0.00 € "));
    QCOMPARE(resultset[0].value (fnAbGeschlossen), qsl("Ja"));
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" 0.00 € "));
    QCOMPARE(resultset[1].value (fnAbGeschlossen), qsl("Nein"));
}

// perp: 1 Vertrag mit (offener) Geldanlage ohne Geldeingang

{
    QVector<QSqlRecord> resultset;
    contract cont01;
    cont01.setCreditorId (saveRandomCreditor ().id ());
    cont01.setConclusionDate (QDate::currentDate ().addMonths (-1));
    cont01.setPlannedInvest (1000.);
    cont01.setInterestModel (interestModel::reinvest);
    cont01.setInterestRate (0.9);
    cont01.setInvestmentId (2);
    QCOMPARE( 1, cont01.saveNewContract ());
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (0/1)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" 1000.00 € "));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" 0.00 € "));
}

// prep: add a second contract
{
    QVector<QSqlRecord> resultset;
    contract cont02;
    cont02.setCreditorId (saveRandomCreditor ().id ());
    cont02.setConclusionDate (QDate::currentDate ().addMonths (-1).addDays (5));
    cont02.setPlannedInvest (2000.);
    cont02.setInterestModel (interestModel::reinvest);
    cont02.setInterestRate (0.9);
    cont02.setInvestmentId (2);
    QCOMPARE( 2, cont02.saveNewContract ());
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (0/2)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" 3000.00 € "));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" 0.00 € "));
}

// prep: activate one of the contracts
{
    QVector<QSqlRecord> resultset;
    contract cont01(1);
    cont01.bookInitialPayment (cont01.conclusionDate ().addDays(10), 1000.);
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (1/1)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" 3000.00 € "));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" 0.00 € "));
}

// prep: activate the other of the contracts
{
    QVector<QSqlRecord> resultset;
    contract cont02(2);
    cont02.bookInitialPayment (cont02.conclusionDate ().addDays (3), 2000.);
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (2/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" 3000.00 € "));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" 0.00 € "));
}

// prep: finish the first contract
{
    QVector<QSqlRecord> resultset;
    contract cont01(1);
    double interest =0., payout =0.;
    QVERIFY(cont01.finalize (false, cont01.conclusionDate ().addMonths (3), interest, payout));
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    dbgDumpDB ();
    // test
    QCOMPARE(2, resultset.size());
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 1);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" 3002.00 € "));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" 0.00 € "));
}
// prep: finish the second contract
{
    QVector<QSqlRecord> resultset;
    contract cont02(2);
    double interest =0., payout =0.;
    QVERIFY(cont02.finalize (false, cont02.conclusionDate ().addMonths (3), interest, payout));
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 2);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" 3006.35 € "));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" 0.00 € "));
}
// perp: add a third Investment and a contract with "fixed" interest model
{
    QDate contractConclusion =QDate::currentDate ().addMonths (-9).addDays (5);
    QVector<QSqlRecord> resultset;
    QCOMPARE(3, saveNewFloatingInvestment (50, qsl("investment 3")));
    contract cont03;
    cont03.setCreditorId (saveRandomCreditor ().id ());
    cont03.setConclusionDate (QDate::currentDate ().addMonths (-9));
    cont03.setPlannedInvest (3000.);
    cont03.setInterestModel (interestModel::fixed);
    cont03.setInterestRate (0.5);
    cont03.setInvestmentId (3);
    cont03.saveNewContract ();
    cont03.bookInitialPayment (contractConclusion, 3000.);
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(3, resultset.size());
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" 3000.00 € "));

// prep: add money to the contract to provoke interest. It should not show up in the investment overview
    resultset.clear ();
    QVERIFY(cont03.deposit (contractConclusion.addMonths(3), 500.));
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    dbgDumpDB ();
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" 3500.00 € "));

}


} // EO void test_geldanlagen::test_kontinuierlicheAnlagen()


#endif // TEST_GELDANLAGEN
