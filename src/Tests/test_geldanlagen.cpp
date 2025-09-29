#ifndef TEST_GELDANLAGEN
#define TEST_GELDANLAGEN

#include "../DKV2/creditor.h"
#include "../DKV2/investment.h"
#include "../DKV2/dkdbviews.h"
#include "../DKV2/helpersql.h"
#include "qnamespace.h"
#include "qtestcase.h"
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

const QString fnAnzahlLaufende {qsl("Anz_laufendeV_(mit_ohne_Geldeingang)")};
const QString fnAnzahlBeendete {qsl("Anz_beendeteVerträge")};
const QString fnGesamtbetrag   {qsl("Gesamtbetrag")};
const QString fnAbGeschlossen  {qsl("(Ab-)Geschlossen")};
const QString invOffen =qsl("Offen");
const QString invGeschlossen =qsl("f. neue Vertr. geschlossen");

QString modifyInvestmentsSqlForAlternate_NOW_date(QDate newNowDate)
{
    return QString(sqlInvestmentsView)
        .replace (qsl("now"), newNowDate.toString (Qt::ISODate));
}

void test_geldanlagen::test_kontinuierlicheAnlagen()
{
    QFETCH( QDate, start);
    QString alt_sqlInvestmentsView =modifyInvestmentsSqlForAlternate_NOW_date (start);
    const int investment_floating1_interest {100};
    const int investment2_floating_interest  {90};
    const int investment3_floating_interest  {50};

    const int investment1_interval_interest {30};

    const double investedAmount1 =1000.;
    const double investedAmount2 =2000.;
    const double investedAmount3 =3000.;

    double gesamtbetragA1 =0.;
    double gesamtbetragA2 =0.;
    double gesamtbetragA3 =0.;
    double gesamtbetragA4 =0.;

    const qlonglong inv1_floating =1;
    const qlonglong inv2_floating =2;
    const qlonglong inv3_floating =3;
    const qlonglong inv4_framed   =4;
    int contractcount =0;
// prep: to have exactly one investment (1%, floating)
{
    QCOMPARE(inv1_floating, saveNewFloatingInvestment (investment_floating1_interest, qsl("investment 1")));
    QVector<QSqlRecord> resultset;
    // ObjectUnderTest
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(resultset.size(), 1);
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA1)));
}

//prep: another investment created, lower interest 0.9%
{
    QVector<QSqlRecord> resultset;
    QCOMPARE(inv2_floating, saveNewFloatingInvestment (investment2_floating_interest, qsl("investment 2")));
    // ObjectUnderTest
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(resultset.size(), 2);
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA1)));
}

// prep: one of the investments is closed and listed as such
{
    QVector<QSqlRecord> resultset;
    closeInvestment(1);
    // ObjectUnderTest
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(resultset.size(), 2);
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" 0.00 € "));
    QCOMPARE(resultset[0].value (fnAbGeschlossen), invOffen);
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA1)));
    QCOMPARE(resultset[1].value (fnAbGeschlossen), invGeschlossen);
}

// perp: 1 Vertrag mit (offener) Geldanlage ohne Geldeingang

{
    QVector<QSqlRecord> resultset;
    contract cont01;
    cont01.setCreditorId (saveRandomCreditor ().id ());
    cont01.setConclusionDate (start.addMonths (-7));
    cont01.setPlannedInvest (investedAmount1);
    cont01.setInterestModel (interestModel::reinvest);
    cont01.setInterestRate (dbInterest2Interest(investment_floating1_interest ));
    cont01.setInvestmentId (2);
    QCOMPARE( ++contractcount, cont01.saveNewContract ());
    // ObjectUnderTest
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(resultset.size(), 2);
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (0/1)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    gesamtbetragA1 +=investedAmount1;
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA1)));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA2)));
}

// prep: add a second contract
{
    QVector<QSqlRecord> resultset;
    contract cont02;
    cont02.setCreditorId (saveRandomCreditor ().id ());
    cont02.setConclusionDate (start.addMonths (-4).addDays (-5));
    cont02.setPlannedInvest (2000.);
    cont02.setInterestModel (interestModel::reinvest);
    cont02.setInterestRate (dbInterest2Interest(investment_floating1_interest));
    cont02.setInvestmentId (2);
    QCOMPARE( ++contractcount, cont02.saveNewContract ());
    // ObjectUnderTest
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(resultset.size(), 2);
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (0/2)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    gesamtbetragA1 =investedAmount1 +investedAmount2;
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA1, 'f', 2)));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA2, 'f', 2)));
}

// prep: activate one of the contracts
{
    QVector<QSqlRecord> resultset;
    contract cont01(1);
    cont01.bookInitialPayment (cont01.conclusionDate ().addMonths(1), investedAmount1);
    // ObjectUnderTest
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(resultset.size(), 2);
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (1/1)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA1, 'f', 2)));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA2, 'f', 2)));
}

// prep: activate the other of the contracts
{
    QVector<QSqlRecord> resultset;
    contract cont02(2);
    cont02.bookInitialPayment (cont02.conclusionDate ().addDays (5), investedAmount2);
    // ObjectUnderTest
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(resultset.size(), 2);
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (2/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA1, 'f', 2)));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA2, 'f', 2)));
}

// prep: finish the first contract
{
    QVector<QSqlRecord> resultset;
    contract cont01(1);
    double interest =0., payout =0.;
    QDate finDate =cont01.conclusionDate ().addMonths (7);
    gesamtbetragA1 -=cont01.value (finDate);
    QVERIFY(cont01.finalize (false, finDate, interest, payout));
    gesamtbetragA1 +=payout; // includes all interest payments
    // ObjectUnderTest
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(resultset.size(), 2);
    // Anlage m niedrigerem Zins kommt zuerst, Zinsen werden im Gesamtbetrag berücksichtigt
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 1);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ")
                .arg(d2s(gesamtbetragA1, 'f', 2)));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA2, 'f', 2)));
}
// // prep: finish the second contract
{
    QVector<QSqlRecord> resultset;
    contract cont02(2);
    double interest =0., payout =0.;
    QDate finDate =cont02.conclusionDate ().addMonths (3);
    gesamtbetragA1 -=cont02.value (finDate);
    QVERIFY(cont02.finalize (false, finDate, interest, payout));
    gesamtbetragA1 +=payout;
    // ObjectUnderTest
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // test
    QCOMPARE(resultset.size(), 2);
    // Anlage m niedrigerem Zins kommt zuerst
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 2);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ")
                .arg(QString::number(gesamtbetragA1, 'f', 2)));
    // Anlage m höherem Zins kommt danach - unverändert
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("0 (0/0)"));
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA2, 'f', 2)));
}
// perp: add a third Investment and a contract with "fixed" interest model
{
    QDate contractConclusion =start.addMonths (-9).addDays (5);
    QVector<QSqlRecord> resultset;
    QCOMPARE(inv3_floating, saveNewFloatingInvestment (investment3_floating_interest, qsl("investment 3")));
    contract cont03;
    cont03.setCreditorId (saveRandomCreditor ().id ());
    cont03.setConclusionDate (start.addMonths (-9));
    cont03.setPlannedInvest (investedAmount3);
    gesamtbetragA3 +=investedAmount3;
    cont03.setInterestModel (interestModel::fixed);
    cont03.setInterestRate (dbInterest2Interest(investment3_floating_interest ));
    cont03.setInvestmentId (3);
    QCOMPARE( ++contractcount,    cont03.saveNewContract ());
    cont03.bookInitialPayment (contractConclusion, cont03.plannedInvest ());
    // ObjectUnderTest
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
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
    gesamtbetragA3 +=deposit;
    // ObjectUnderTest
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA3)));
}
// prep: a contract older than one year should not show up
{
    QDate oldContractConcluseionDate = start.addYears(-1).addMonths(-1);
    contract oldContract01;
    oldContract01.setCreditorId (saveRandomCreditor ().id ());
    oldContract01.setConclusionDate (oldContractConcluseionDate);
    oldContract01.setPlannedInvest (investedAmount3);
    oldContract01.setInterestModel (interestModel::payout);
    oldContract01.setInterestRate (dbInterest2Interest(investment3_floating_interest ));
    oldContract01.setInvestmentId (3);
    QCOMPARE( ++contractcount, oldContract01.saveNewContract ());
    QVector<QSqlRecord> resultset;
    // ObjectUnderTest: Investment values should not chage
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // tests
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA3)));
// prep: activate old contract more than a year back
    oldContract01.bookInitialPayment (oldContractConcluseionDate.addDays(4), investedAmount3);
    // ObjectUnderTest
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // tests: this should not change statistics
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)"));
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA3)));
// prep: make a deposit less than one year back
    oldContract01.deposit (oldContractConcluseionDate.addMonths (2), investedAmount3, true); // 3000.
    resultset.clear ();
    // tests: this should not change contract count, but Amount in the Investment
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (2/0)")); // one more contract to count
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA3 +investedAmount3)));
// prep: future bookings do not count in
    oldContract01.deposit(start.addDays (5), investedAmount3); // FUTURE date
    resultset.clear();
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (2/0)")); // one more contract to count
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA3 +investedAmount3)));
// prep: future new contract should not change count or Amount
    contract futureContract;
    futureContract.setCreditorId (saveRandomCreditor ().id());
    futureContract.setConclusionDate (start.addDays (1));
    futureContract.setPlannedInvest (investedAmount1);
    futureContract.setInterestModel (interestModel::payout);
    futureContract.setInterestRate (dbInterest2Interest(investment3_floating_interest ));
    futureContract.setInvestmentId (inv3_floating);
    QCOMPARE( ++contractcount, futureContract.saveNewContract ());

    resultset.clear();
    // ObjectUnderTest
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // investment 3 w. lowest interest is unchanged
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (2/0)")); // one more contract to count
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA3 +investedAmount3)));

 // prep: add a contract so that there are multiple continous investments w active contracts
    contract contractInOtherContInv;
    contractInOtherContInv.setCreditorId (1);
    contractInOtherContInv.setConclusionDate (start.addMonths (-9));
    contractInOtherContInv.setPlannedInvest (investedAmount2);
    contractInOtherContInv.setInterestModel (interestModel::payout);
    contractInOtherContInv.setInterestRate (investment2_floating_interest);
    contractInOtherContInv.setInvestmentId (inv2_floating);
    QCOMPARE (++contractcount, contractInOtherContInv.saveNewContract ());
    gesamtbetragA1 +=investedAmount2;
    // ObjectUnderTest
    resultset.clear();
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // investment 2 adds one more inactive contract
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("1 (0/1)")); // one more contract to count
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 2);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA1)));
// prep: activate contract
    contractInOtherContInv.bookInitialPayment (contractInOtherContInv.conclusionDate ().addDays (14), contractInOtherContInv.plannedInvest ());
    // ObjectUnderTest
    resultset.clear();
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // investment 2 adds one more inactive contract
    QCOMPARE(resultset[1].value (fnAnzahlLaufende), qsl("1 (1/0)")); // one more contract to count
    QCOMPARE(resultset[1].value (fnAnzahlBeendete).toInt(), 2);
    QCOMPARE(resultset[1].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA1)));
}
//////////////////////////////////////////////////////////
// prep: create time interval investments & contracts
//////////////////////////////////////////////////////////
{
    QDate startDate01 =start.addMonths (-10);
    QDate endDate02   =startDate01.addYears (1);
    QCOMPARE (inv4_framed, saveNewTimeframedInvestment (investment1_interval_interest, startDate01, endDate02, "inerval01"));
    QVector<QSqlRecord> resultset;
    // object under test
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // new investment (4) is listed, but all values zero
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("0 (0/0)")); // one more contract to count
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(0.)));

    contract contract01_framedI;
    contract01_framedI.setCreditorId (saveRandomCreditor ().id());
    contract01_framedI.setConclusionDate (startDate01);
    contract01_framedI.setPlannedInvest (investedAmount1);
    contract01_framedI.setInterestModel (interestModel::fixed);
    contract01_framedI.setInvestmentId (inv4_framed);
    contract01_framedI.setInterestRate (investment1_interval_interest);
    QCOMPARE( ++contractcount, contract01_framedI.saveNewContract ());
    gesamtbetragA4 +=investedAmount1;

    resultset.clear ();
    // ObjectUnderTest: one new inactive contract
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // new investment (4) is listed, but all values zero
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (0/1)")); // one more contract to count
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA4)));

    // activate contract
    contract01_framedI.bookInitialPayment (startDate01.addMonths (1), contract01_framedI.plannedInvest ());
    // ObjectUnderTest: one new active contract
    resultset.clear ();
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // new investment (4) has now one active contract
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)")); // one more contract to count
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA4)));

// prep: second contract same investment
    contract contract02_framedI;
    contract02_framedI.setCreditorId (2);
    contract02_framedI.setConclusionDate (startDate01.addDays (14));
    contract02_framedI.setPlannedInvest (investedAmount1);
    contract02_framedI.setInterestModel (interestModel::reinvest);
    contract02_framedI.setInvestmentId (inv4_framed);
    contract02_framedI.setInterestRate (investment1_interval_interest);
    gesamtbetragA4 +=investedAmount1;

    QCOMPARE( ++contractcount, contract02_framedI.saveNewContract ());
    // ObjectUnderTest: second contract
    resultset.clear ();
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // tests
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (1/1)")); // one more contract to count
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA4)));
    QCOMPARE(resultset[0].value (fnAbGeschlossen), invOffen);
 // prep: activate second contract
    contract02_framedI.bookInitialPayment (startDate01.addDays (28), contract02_framedI.plannedInvest ());
    // ObjectUnderTest: second contract
    resultset.clear ();
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // tests
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (2/0)")); // one more contract to count
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA4)));
    QCOMPARE(resultset[0].value (fnAbGeschlossen), invOffen);
// prep: add. payment to contract01
    double einzahlung = 1000.;
    contract01_framedI.deposit (startDate01.addMonths (7), einzahlung, false); // fixed interest
    gesamtbetragA4 += einzahlung;
    // ObjectUnderTest: second contract
    resultset.clear ();
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // tests
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("2 (2/0)")); // one more contract to count
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 0);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA4)));
// prep: finalize contracts
    double contract01_framedI_final_value_of_fixed_contract =contract01_framedI.interestBearingValue ();
    double returnedInterest =0., contract01_framedI_payout =0.;
    QVERIFY (contract01_framedI.finalize (false, startDate01.addMonths (11), returnedInterest, contract01_framedI_payout));
    // ObjectUnderTest
    resultset.clear ();
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // tests
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)")); // one more contract to count
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 1);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA4)));


    // add payment to second contract after the investment end date
    contract02_framedI.deposit (startDate01.addMonths (12), 1000.);
    gesamtbetragA4 = contract02_framedI.value () +contract01_framedI_final_value_of_fixed_contract;

    // ObjectUnderTest
    resultset.clear ();
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // tests: this should not change the values
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("1 (1/0)")); // one more contract to count
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 1);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA4)));


// second contract in this investment is finalized
    returnedInterest =0.; contract01_framedI_payout =0.;
    contract02_framedI.finalize (false, startDate01.addMonths (13), returnedInterest, contract01_framedI_payout);
    // ObjectUnderTest
    resultset.clear ();
    QVERIFY2(executeSql(alt_sqlInvestmentsView, resultset), "Geldanlagen Ansicht SQL konnte nicht ausgeführt werden!");
    // tests
    QCOMPARE(resultset[0].value (fnAnzahlLaufende), qsl("0 (0/0)")); // one more contract to count
    QCOMPARE(resultset[0].value (fnAnzahlBeendete).toInt(), 2);
    QCOMPARE(resultset[0].value (fnGesamtbetrag), qsl(" %1 € ").arg(d2s(gesamtbetragA4)));
}


} // EO void test_geldanlagen::test_kontinuierlicheAnlagen()

void test_geldanlagen::test_kontinuierlicheAnlagen_data()
{
    QTest::addColumn<QDate> ("start");
    // test different times, so that yearly interest payments will occure at different parts of the process
    for (int shiftMonth =0; shiftMonth < 15; shiftMonth+=2) {
        QDate start =QDate::currentDate ().addMonths (shiftMonth);
        QTest::newRow(start.toString ().toLocal8Bit ().data ()) << start;
    }
}

#endif // TEST_GELDANLAGEN
