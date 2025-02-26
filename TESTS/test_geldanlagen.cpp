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

const int investment1_interest {100};
const int investment2_interest  {90};
const int investment3_interest  {50};

const double investedAmount1 =1000.;
const double investedAmount2 =2000.;
const double investedAmount3 =3000.;

double gesamtbetrag1 =0.;
double gesamtbetrag2 =0.;
double gesamtbetrag3 =0.;

void test_geldanlagen::test_kontinuierlicheAnlagen()
{
// prep: to have exactly one investment (1%, floating)
{
        QCOMPARE(1, saveNewFloatingInvestment (investment1_interest, qsl("investment 1")));
    QVector<QSqlRecord> resultset;
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(1, resultset.size());
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag1)));
}

//prep: another investment created, lower interest 0.9%
{
    QVector<QSqlRecord> resultset;
    QCOMPARE(2, saveNewFloatingInvestment (investment2_interest, qsl("investment 2")));
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag1)));
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
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag1)));
    QCOMPARE(resultset[1].value (fnAbGeschlossen), qsl("Nein"));
}

// perp: 1 Vertrag mit (offener) Geldanlage ohne Geldeingang

{
    QVector<QSqlRecord> resultset;
    contract cont01;
    cont01.setCreditorId (saveRandomCreditor ().id ());
    cont01.setConclusionDate (QDate::currentDate ().addMonths (-7));
    cont01.setPlannedInvest (investedAmount1);
    cont01.setInterestModel (interestModel::reinvest);
    cont01.setInterestRate (dbInterest2Interest(investment1_interest ));
    cont01.setInvestmentId (2);
    QCOMPARE( 1, cont01.saveNewContract ());
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (0/1)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    gesamtbetrag1 +=investedAmount1;
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag1)));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag2)));
}

// prep: add a second contract
{
    QVector<QSqlRecord> resultset;
    contract cont02;
    cont02.setCreditorId (saveRandomCreditor ().id ());
    cont02.setConclusionDate (QDate::currentDate ().addMonths (-1).addDays (5));
    cont02.setPlannedInvest (2000.);
    cont02.setInterestModel (interestModel::reinvest);
    cont02.setInterestRate (dbInterest2Interest(investment1_interest));
    cont02.setInvestmentId (2);
    QCOMPARE( 2, cont02.saveNewContract ());
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (0/2)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    gesamtbetrag1 =investedAmount1 +investedAmount2;
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag1, 'f', 2)));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag2, 'f', 2)));
}

// prep: activate one of the contracts
{
    QVector<QSqlRecord> resultset;
    contract cont01(1);
    cont01.bookInitialPayment (cont01.conclusionDate ().addMonths(1), investedAmount1);
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (1/1)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag1, 'f', 2)));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag2, 'f', 2)));
}

// prep: activate the other of the contracts
{
    QVector<QSqlRecord> resultset;
    contract cont02(2);
    cont02.bookInitialPayment (cont02.conclusionDate ().addDays (3), investedAmount2);
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (2/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag1, 'f', 2)));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag2, 'f', 2)));
}

// prep: finish the first contract
{
    QVector<QSqlRecord> resultset;
    contract cont01(1);
    double interest =0., payout =0.;
    QVERIFY(cont01.finalize (false, cont01.conclusionDate ().addMonths (7), interest, payout));
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(2, resultset.size());
    // Anlage m niedrigerem Zins kommt zuerst, Zinsen werden im Gesamtbetrag berücksichtigt
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 1);
    gesamtbetrag1 +=(6./12.*dbInterest2Interest(investment1_interest)*euroFromCt (investedAmount1));
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ")
                .arg(d2s(gesamtbetrag1, 'f', 2)));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag2, 'f', 2)));
}
// prep: finish the second contract
{
    QVector<QSqlRecord> resultset;
    contract cont02(2);
    double interest =0., payout =0.;
    QVERIFY(cont02.finalize (false, cont02.conclusionDate ().addMonths (3).addDays (3), interest, payout));
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    dbgDumpDB ();
    QCOMPARE(2, resultset.size());
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 2);
    gesamtbetrag1 +=3./12. *dbInterest2Interest(investment1_interest) *euroFromCt(investedAmount2);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ")
                .arg(QString::number(gesamtbetrag1, 'f', 2)));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag2, 'f', 2)));
}
// perp: add a third Investment and a contract with "fixed" interest model
{
    QDate contractConclusion =QDate::currentDate ().addMonths (-9).addDays (5);
    QVector<QSqlRecord> resultset;
    QCOMPARE(3, saveNewFloatingInvestment (investment3_interest, qsl("investment 3")));
    contract cont03;
    cont03.setCreditorId (saveRandomCreditor ().id ());
    cont03.setConclusionDate (QDate::currentDate ().addMonths (-9));
    cont03.setPlannedInvest (investedAmount3);
    gesamtbetrag3 +=investedAmount3;
    cont03.setInterestModel (interestModel::fixed);
    cont03.setInterestRate (dbInterest2Interest(investment3_interest ));
    cont03.setInvestmentId (3);
    cont03.saveNewContract ();
    cont03.bookInitialPayment (contractConclusion, cont03.plannedInvest ());
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(3, resultset.size());
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(investedAmount3)));

// prep: add money to the contract to provoke interest. It should not show up in the investment overview
    resultset.clear ();
    double deposit =500.;
    QVERIFY(cont03.deposit (contractConclusion.addMonths(3), deposit));
    gesamtbetrag3 +=deposit;
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag3)));
}
// prep: a contract older than one year should not show up
{
    QDate oldContractConcluseionDate = QDate::currentDate ().addYears(-1).addMonths(-1);
    contract oldContract01;
    oldContract01.setCreditorId (saveRandomCreditor ().id ());
    oldContract01.setConclusionDate (oldContractConcluseionDate);
    oldContract01.setPlannedInvest (investedAmount3);
    oldContract01.setInterestModel (interestModel::payout);
    oldContract01.setInterestRate (dbInterest2Interest(investment3_interest ));
    oldContract01.setInvestmentId (3);
    QCOMPARE( 4, oldContract01.saveNewContract ());
    // ObjectUnderTest: Investment values should not chage
    QVector<QSqlRecord> resultset;
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // tests
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag3)));
// prep: activate old contract more than a year back
    oldContract01.bookInitialPayment (oldContractConcluseionDate.addDays(4), investedAmount3);
    // ObjectUnderTest
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // tests: this should not change statistics
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag3)));
// prep: make a deposit less than a year back
    oldContract01.deposit (oldContractConcluseionDate.addMonths (2), investedAmount3, true); // 3000.
    resultset.clear ();
    // tests: this should not change statistics
    QVERIFY2(executeSql(sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (2/0)")); // one more contract to count
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetrag3 +investedAmount3)));


}


} // EO void test_geldanlagen::test_kontinuierlicheAnlagen()


#endif // TEST_GELDANLAGEN
