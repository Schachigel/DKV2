#include "test_views.h"

#include "../DKV2/appconfig.h"
#include "../DKV2/booking.h"
#include "../DKV2/contract.h"
#include "../DKV2/creditor.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/dkdbviews.h"
#include "../DKV2/helperfin.h"
#include "../DKV2/helpersql.h"
#include "../DKV2/investment.h"
#include "testhelper.h"

#include <QtTest/QTest>
#include <optional>

namespace {
creditorId_t insertMinimalCreditor()
{
    TableDataInserter creditorTdi(creditor::getTableDef());
    creditorTdi.setValue(creditor::fnVorname, qsl("Ada"));
    creditorTdi.setValue(creditor::fnNachname, qsl("Lovelace"));
    creditorTdi.setValue(creditor::fnStrasse, qsl("Memory Lane 1"));
    creditorTdi.setValue(creditor::fnPlz, qsl("68167"));
    creditorTdi.setValue(creditor::fnStadt, qsl("Mannheim"));
    return {creditorTdi.InsertRecord()};
}

// Scenarios drive the real contract lifecycle API (saveNewContract/bookInitialPayment/
// deposit/payout/annualSettlement/finalize) instead of injecting raw rows, so a scenario
// can only represent a state the application could actually produce. Expected interest
// amounts are computed via the same reference formulas (ZinsesZins_act_act/_30_360) used
// by test_contract.cpp, independently of whatever the business functions end up booking.
struct piAction
{
    enum class kind { openContract, deposit, payout, annualSettlement, terminate };
    kind type{kind::openContract};
    int contractIndex{0};
    QDate date;
    double amount{0.0};              // openContract: initial payment; deposit/payout: amount
    double interestRate{0.0};        // openContract only
    interestModel model{interestModel::reinvest}; // openContract only
    int year{0};                     // annualSettlement only
};

struct piExpectedOverviewRow
{
    QDate date;
    int bookingCount{0};
    double bookedAtDate{0.0};
    int contractCount{0};
    double totalInclInterest{0.0};
    double totalWithoutInterest{0.0};
};

struct piScenario
{
    QString name;
    QString zinsusance{qsl("act/act")};
    QVector<piAction> actions;
    QVector<piExpectedOverviewRow> expectedRows;
};

void runPerpetualInvestmentActions(const piScenario& scenario)
{
    dbConfig::writeValue(ZINSUSANCE, scenario.zinsusance);

    const tableindex_t investmentId{saveNewInvestment(200,
                                                      QDate(2024, 1, 1),
                                                      EndOfTheFuckingWorld,
                                                      qsl("Offene Testanlage DDT"))};
    QVERIFY(isValidRowId(investmentId));

    const creditorId_t creditorId{insertMinimalCreditor()};
    QVERIFY(isValidRowId(creditorId.v));

    QVector<contract> contracts;
    for (const piAction& action : scenario.actions) {
        switch (action.type) {
        case piAction::kind::openContract: {
            contract c;
            c.initContractDefaults(creditorId);
            c.setInterestRate(action.interestRate);
            c.setInterestModel(action.model);
            c.setConclusionDate(action.date);
            c.setInvestmentId(investmentId);
            c.saveNewContract();
            QVERIFY(isValidRowId(c.id().v));
            QVERIFY(c.bookInitialPayment(action.date, action.amount));
            contracts.push_back(c);
            break;
        }
        case piAction::kind::deposit:
            QVERIFY(contracts[action.contractIndex].deposit(action.date, action.amount));
            break;
        case piAction::kind::payout:
            QVERIFY(contracts[action.contractIndex].payout(action.date, action.amount));
            break;
        case piAction::kind::annualSettlement:
            QCOMPARE(contracts[action.contractIndex].annualSettlement(action.year), action.year);
            break;
        case piAction::kind::terminate: {
            double finInterest{0.};
            double finPayout{0.};
            QVERIFY(contracts[action.contractIndex].finalize(false, action.date, finInterest, finPayout));
            break;
        }
        }
    }
}

void comparePerpetualInvestmentOverview(const QVector<QStringList>& data, const QVector<piExpectedOverviewRow>& expectedRows)
{
    QCOMPARE(data.size(), expectedRows.size());
    for (qsizetype i =0; i < expectedRows.size(); ++i) {
        const auto& expected{expectedRows[i]};
        QCOMPARE(data[i].size(), 7);
        QCOMPARE(data[i][1], expected.date.toString(qsl("dd.MM.yyyy")));
        QCOMPARE(data[i][2], i2s(expected.bookingCount));
        QCOMPARE(data[i][3], s_d2euro(expected.bookedAtDate));
        QCOMPARE(data[i][4], i2s(expected.contractCount));
        QCOMPARE(data[i][5], s_d2euro(expected.totalInclInterest));
        QCOMPARE(data[i][6], s_d2euro(expected.totalWithoutInterest));
    }
}
}

Q_DECLARE_METATYPE(piScenario)

void test_views::initTestCase()
{
    createTestDkDbTemplate();
}
void test_views::cleanupTestCase()
{
    cleanupTestDkDbTemplate();
}
void test_views::init()
{
    initTestDkDbFromTemplate();
}
void test_views::cleanup()
{
    cleanupTestDkDb();
}

void test_views::test_investmentOverview_includesDeletedContractsAndBookings()
{
    const tableindex_t investmentId = saveNewInvestment(250, QDate(2026, 1, 1), QDate(2026, 12, 31), qsl("Testanlage"));
    QVERIFY(isValidRowId(investmentId));

    const creditorId_t creditorId{insertMinimalCreditor()};
    QVERIFY(isValidRowId(creditorId.v));

    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO Vertraege "
            "(id, KreditorId, Kennung, ZSatz, Betrag, thesaurierend, Vertragsdatum, Kfrist, AnlagenId, LaufzeitEnde, zActive, KueDatum) "
            "VALUES (1, %1, 'DK-TST-2026-000001', 250, 10000, 0, '2026-01-15', 6, %2, '9999-12-31', TRUE, '9999-12-31')")
            .arg(i2s(creditorId.v), i2s(investmentId))));
    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO Buchungen "
            "(id, %1, %2, %3, %4, %5) "
            "VALUES (1, 1, '2026-01-15', 1, 10000, '1900-01-01')")
            .arg(booking::fn_bVertragsId,
                 booking::fn_bDatum,
                 booking::fn_bBuchungsArt,
                 booking::fn_bBetrag,
                 booking::fn_bModifiziert)));
    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO Buchungen "
            "(id, %1, %2, %3, %4, %5) "
            "VALUES (2, 1, '2026-12-31', 8, 1000, '1900-01-01')")
            .arg(booking::fn_bVertragsId,
                 booking::fn_bDatum,
                 booking::fn_bBuchungsArt,
                 booking::fn_bBetrag,
                 booking::fn_bModifiziert)));

    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO exVertraege "
            "(id, KreditorId, Kennung, Anmerkung, ZSatz, Betrag, thesaurierend, Vertragsdatum, Kfrist, AnlagenId, LaufzeitEnde, zActive, KueDatum) "
            "VALUES (2, %1, 'DK-TST-2026-000002', '', 250, 20000, 1, '2026-02-15', 6, %2, '2026-12-31', TRUE, '9999-12-31')")
            .arg(i2s(creditorId.v), i2s(investmentId))));
    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO exBuchungen "
            "(id, %1, %2, %3, %4, %5) "
            "VALUES (3, 2, '2026-02-15', 1, 20000, '1900-01-01')")
            .arg(booking::fn_bVertragsId,
                 booking::fn_bDatum,
                 booking::fn_bBuchungsArt,
                 booking::fn_bBetrag,
                 booking::fn_bModifiziert)));
    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO exBuchungen "
            "(id, %1, %2, %3, %4, %5) "
            "VALUES (4, 2, '2026-12-31', 8, 2000, '1900-01-01')")
            .arg(booking::fn_bVertragsId,
                 booking::fn_bDatum,
                 booking::fn_bBuchungsArt,
                 booking::fn_bBetrag,
                 booking::fn_bModifiziert)));

    QSqlRecord rec = executeSingleRecordSql(
        qsl("SELECT Anzahl, SummeVertraege, AnzahlAktive, SummeAktive, Einzahlungen, SummeInclZins "
            "FROM vInvestmentsOverview WHERE AnlagenId = %1").arg(i2s(investmentId)));
    QVERIFY(not rec.isEmpty());

    QCOMPARE(rec.value(0).toInt(), 2);
    QCOMPARE(rec.value(1).toDouble(), 300.0);
    QCOMPARE(rec.value(2).toInt(), 2);
    QCOMPARE(rec.value(3).toDouble(), 300.0);
    QCOMPARE(rec.value(4).toDouble(), 310.0);
    QCOMPARE(rec.value(5).toDouble(), 330.0);

    investment invest(investmentId);
    const investment::invStatisticData data = invest.getStatisticData(QDate(2026, 6, 1));
    QCOMPARE(data.anzahlVertraege, 2);
    QCOMPARE(data.summeVertraege, 300.0);
    QCOMPARE(data.EinAuszahlungen, 300.0);
    QCOMPARE(data.ZzglZins, 330.0);
}

void test_views::test_getStatisticData_anchorsOnFirstPaymentElseContractDate()
{
    // Window for newContractDate = 2026-06-01 (continuous investment): (2025-06-01, 2026-06-01]
    const tableindex_t investmentId = saveNewInvestment(250, QDate(1900, 1, 1), QDate(9999, 12, 31), qsl("Testanlage"));
    QVERIFY(isValidRowId(investmentId));

    const creditorId_t creditorId{insertMinimalCreditor()};
    QVERIFY(isValidRowId(creditorId.v));

    // Contract 1: Vertragsdatum before the window, but first payment lands inside it
    // -> must count via Ersteinzahlung, would be missed by a pure Vertragsdatum filter.
    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO Vertraege "
            "(id, KreditorId, Kennung, ZSatz, Betrag, thesaurierend, Vertragsdatum, Kfrist, AnlagenId, LaufzeitEnde, zActive, KueDatum) "
            "VALUES (1, %1, 'DK-TST-2026-000001', 250, 5000, 0, '2025-01-01', 6, %2, '9999-12-31', TRUE, '9999-12-31')")
            .arg(i2s(creditorId.v), i2s(investmentId))));
    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO Buchungen (id, %1, %2, %3, %4, %5) VALUES (1, 1, '2025-12-01', 1, 5000, '1900-01-01')")
            .arg(booking::fn_bVertragsId, booking::fn_bDatum, booking::fn_bBuchungsArt,
                 booking::fn_bBetrag, booking::fn_bModifiziert)));

    // Contract 2: Vertragsdatum inside the window, never paid
    // -> must count via the Vertragsdatum fallback (earliest possible booking date).
    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO Vertraege "
            "(id, KreditorId, Kennung, ZSatz, Betrag, thesaurierend, Vertragsdatum, Kfrist, AnlagenId, LaufzeitEnde, zActive, KueDatum) "
            "VALUES (2, %1, 'DK-TST-2026-000002', 250, 3000, 0, '2025-07-01', 6, %2, '9999-12-31', TRUE, '9999-12-31')")
            .arg(i2s(creditorId.v), i2s(investmentId))));

    // Contract 3: Vertragsdatum inside the window, but first payment lands after it
    // -> must NOT count; a pure Vertragsdatum filter would wrongly include it.
    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO Vertraege "
            "(id, KreditorId, Kennung, ZSatz, Betrag, thesaurierend, Vertragsdatum, Kfrist, AnlagenId, LaufzeitEnde, zActive, KueDatum) "
            "VALUES (3, %1, 'DK-TST-2026-000003', 250, 7000, 0, '2025-08-01', 6, %2, '9999-12-31', TRUE, '9999-12-31')")
            .arg(i2s(creditorId.v), i2s(investmentId))));
    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO Buchungen (id, %1, %2, %3, %4, %5) VALUES (2, 3, '2026-07-01', 1, 7000, '1900-01-01')")
            .arg(booking::fn_bVertragsId, booking::fn_bDatum, booking::fn_bBuchungsArt,
                 booking::fn_bBetrag, booking::fn_bModifiziert)));

    investment invest(investmentId);
    const investment::invStatisticData data = invest.getStatisticData(QDate(2026, 6, 1));
    QCOMPARE(data.anzahlVertraege, 2);
    QCOMPARE(data.summeVertraege, 80.0);
}

void test_views::test_investmentsOverview_fortlaufend_anchorsOnFirstPaymentElseContractDate()
{
    const QDate today = QDate::currentDate();
    const tableindex_t investmentId = saveNewInvestment(250, QDate(1900, 1, 1), QDate(9999, 12, 31), qsl("Testanlage"));
    QVERIFY(isValidRowId(investmentId));

    const creditorId_t creditorId{insertMinimalCreditor()};
    QVERIFY(isValidRowId(creditorId.v));

    // Contract A: Vertragsdatum 2 years old (outside window), but first payment recent
    // -> must count in Anzahl/AnzahlAktive via the Ersteinzahlung anchor.
    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO Vertraege "
            "(id, KreditorId, Kennung, ZSatz, Betrag, thesaurierend, Vertragsdatum, Kfrist, AnlagenId, LaufzeitEnde, zActive, KueDatum) "
            "VALUES (1, %1, 'DK-TST-2026-000001', 250, 5000, 0, '%2', 6, %3, '9999-12-31', TRUE, '9999-12-31')")
            .arg(i2s(creditorId.v), today.addYears(-2).toString(Qt::ISODate), i2s(investmentId))));
    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO Buchungen (id, %1, %2, %3, %4, %5) VALUES (1, 1, '%6', 1, 5000, '1900-01-01')")
            .arg(booking::fn_bVertragsId, booking::fn_bDatum, booking::fn_bBuchungsArt,
                 booking::fn_bBetrag, booking::fn_bModifiziert, today.addDays(-30).toString(Qt::ISODate))));

    // Contract B: Vertragsdatum recent, never paid
    // -> must count in Anzahl via the Vertragsdatum fallback, but NOT in AnzahlAktive.
    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO Vertraege "
            "(id, KreditorId, Kennung, ZSatz, Betrag, thesaurierend, Vertragsdatum, Kfrist, AnlagenId, LaufzeitEnde, zActive, KueDatum) "
            "VALUES (2, %1, 'DK-TST-2026-000002', 250, 3000, 0, '%2', 6, %3, '9999-12-31', TRUE, '9999-12-31')")
            .arg(i2s(creditorId.v), today.addDays(-10).toString(Qt::ISODate), i2s(investmentId))));

    // Contract C: both Vertragsdatum and first payment 2 years old
    // -> must NOT count anywhere; truly stale, outside the window by either anchor.
    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO Vertraege "
            "(id, KreditorId, Kennung, ZSatz, Betrag, thesaurierend, Vertragsdatum, Kfrist, AnlagenId, LaufzeitEnde, zActive, KueDatum) "
            "VALUES (3, %1, 'DK-TST-2026-000003', 250, 7000, 0, '%2', 6, %3, '9999-12-31', TRUE, '9999-12-31')")
            .arg(i2s(creditorId.v), today.addYears(-2).toString(Qt::ISODate), i2s(investmentId))));
    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO Buchungen (id, %1, %2, %3, %4, %5) VALUES (2, 3, '%6', 1, 7000, '1900-01-01')")
            .arg(booking::fn_bVertragsId, booking::fn_bDatum, booking::fn_bBuchungsArt,
                 booking::fn_bBetrag, booking::fn_bModifiziert, today.addYears(-2).addDays(5).toString(Qt::ISODate))));

    QSqlRecord rec = executeSingleRecordSql(
        qsl("SELECT Anzahl, SummeVertraege, AnzahlAktive, SummeAktive FROM vInvestmentsOverview WHERE AnlagenId = %1")
            .arg(i2s(investmentId)));
    QVERIFY(not rec.isEmpty());

    QCOMPARE(rec.value(0).toInt(), 2);        // Anzahl: A (via Ersteinzahlung) + B (via Vertragsdatum fallback)
    QCOMPARE(rec.value(1).toDouble(), 80.0);  // SummeVertraege: 50 + 30
    QCOMPARE(rec.value(2).toInt(), 1);        // AnzahlAktive: only A
    QCOMPARE(rec.value(3).toDouble(), 50.0);  // SummeAktive: only A
}

void test_views::test_interestByYearOverview_classifiesInterimInterestByContractMode()
{
    dbConfig::writeValue(ZINSUSANCE, qsl("30/360"));

    const creditorId_t creditorId{insertMinimalCreditor()};
    QVERIFY(isValidRowId(creditorId.v));
    creditor cred1{creditorId};

    const QDate conclusionDate{2024, 12, 1};
    const QDate initialDate{2024, 12, 30};
    const QDate commonDepositDate{2025, 3, 15};

    contract payoutContract;
    payoutContract.initContractDefaults(cred1.id());
    payoutContract.setInterestModel(interestModel::payout);
    payoutContract.setInterestRate(1.5);
    payoutContract.setPlannedInvest(1000.);
    payoutContract.updateConclusionDate(conclusionDate);
    payoutContract.saveNewContract();

    QVERIFY(payoutContract.bookInitialPayment(initialDate, 1000.));
    QVERIFY(payoutContract.deposit(commonDepositDate, 500., true));
    QCOMPARE(payoutContract.annualSettlement(2025), 2025);

    contract reinvestContract;
    reinvestContract.initContractDefaults(cred1.id());
    reinvestContract.setInterestModel(interestModel::reinvest);
    reinvestContract.setInterestRate(1.5);
    reinvestContract.setPlannedInvest(1000.);
    reinvestContract.updateConclusionDate(conclusionDate);
    reinvestContract.saveNewContract();

    QVERIFY(reinvestContract.bookInitialPayment(initialDate, 1000.));
    QVERIFY(reinvestContract.deposit(commonDepositDate, 500.));
    QCOMPARE(reinvestContract.annualSettlement(2025), 2025);

    QVector<QSqlRecord> records;
    QVERIFY(executeSql(sqlInterestByYearOverview, records));

    auto findYearRow = [&records](const QString& year, const QString& ba, const QString& thesa) -> std::optional<QSqlRecord>
    {
        for (const QSqlRecord& rec : records) {
            if (rec.value(qsl("Year")).toString() == year
                and rec.value(qsl("BA")).toString() == ba
                and rec.value(qsl("Thesa")).toString() == thesa)
                return rec;
        }
        return std::nullopt;
    };

    const auto interimPayoutRow = findYearRow(qsl("2025"), qsl("Unterjährige Zinsen"), qsl(" ausgezahlte Zinsen "));
    QVERIFY(interimPayoutRow.has_value());
    QCOMPARE(interimPayoutRow->value(qsl("Summe")).toDouble(), 3.13);

    const auto interimReinvestRow = findYearRow(qsl("2025"), qsl("Unterjährige Zinsen"), qsl(" angerechnete Zinsen "));
    QVERIFY(interimReinvestRow.has_value());
    QCOMPARE(interimReinvestRow->value(qsl("Summe")).toDouble(), 3.13);

    const auto interimTotalRow = findYearRow(qsl("2025"), qsl("Unterjährige Zinsen"), qsl(" gesamte Zinsen"));
    QVERIFY(interimTotalRow.has_value());
    QCOMPARE(interimTotalRow->value(qsl("Summe")).toDouble(), 6.26);

    const auto annualPayoutRow = findYearRow(qsl("2025"), qsl("Zins aus Jahresendabrechnungen"), qsl(" ausbezahlte Zinsen "));
    QVERIFY(annualPayoutRow.has_value());
    QCOMPARE(annualPayoutRow->value(qsl("Summe")).toDouble(), 17.81);

    const auto annualReinvestRow = findYearRow(qsl("2025"), qsl("Zins aus Jahresendabrechnungen"), qsl(" angerechnete Zinsen "));
    QVERIFY(annualReinvestRow.has_value());
    QCOMPARE(annualReinvestRow->value(qsl("Summe")).toDouble(), 17.85);

    const auto annualTotalRow = findYearRow(qsl("2025"), qsl("Zins aus Jahresendabrechnungen"), qsl(" gesamte Zinsen "));
    QVERIFY(annualTotalRow.has_value());
    QCOMPARE(annualTotalRow->value(qsl("Summe")).toDouble(), 35.66);
}

void test_views::test_shortInfo_overviews_keepDeferredMarkerNeutral()
{
    const creditorId_t creditorId{insertMinimalCreditor()};
    QVERIFY(isValidRowId(creditorId.v));
    creditor cred{creditorId};

    contract payoutContract;
    payoutContract.initContractDefaults(cred.id());
    payoutContract.setInterestModel(interestModel::payout);
    payoutContract.setInterestRate(1.0);
    payoutContract.setPlannedInvest(1000.);
    payoutContract.updateConclusionDate(QDate(2025, 1, 1));
    payoutContract.saveNewContract();
    QVERIFY(payoutContract.bookInitialPayment(QDate(2025, 1, 15), 1000.));

    contract deferredReinvestContract;
    deferredReinvestContract.initContractDefaults(cred.id());
    deferredReinvestContract.setInterestModel(interestModel::reinvest);
    deferredReinvestContract.setInterestRate(2.0);
    deferredReinvestContract.setPlannedInvest(2000.);
    deferredReinvestContract.updateConclusionDate(QDate(2025, 2, 1));
    deferredReinvestContract.saveNewContract();
    QVERIFY(deferredReinvestContract.bookInitialPayment(QDate(2025, 2, 15), 2000.));
    QVERIFY(bookDeferredInBetweenInterest(deferredReinvestContract.id(), QDate(2025, 6, 1)));

    contract inactiveFixedContract;
    inactiveFixedContract.initContractDefaults(cred.id());
    inactiveFixedContract.setInterestModel(interestModel::fixed);
    inactiveFixedContract.setInterestRate(3.0);
    inactiveFixedContract.setPlannedInvest(3000.);
    inactiveFixedContract.updateConclusionDate(QDate(2025, 3, 1));
    inactiveFixedContract.saveNewContract();

    const QVector<QStringList> activeInfo{overviewShortInfo(sqlOverviewActiveContracts)};
    QCOMPARE(activeInfo.size(), 7);
    const QSqlRecord activeRec{executeSingleRecordSql(sqlOverviewActiveContracts)};
    QVERIFY(not activeRec.isEmpty());
    QCOMPARE(activeRec.value(qsl("AnzahlKreditoren")).toInt(), 1);
    QCOMPARE(activeRec.value(qsl("AnzahlVertraege")).toInt(), 2);
    QCOMPARE(activeRec.value(qsl("GesamtVolumen")).toDouble(), 3000.0);
    QCOMPARE(activeRec.value(qsl("MittlererVertragswert")).toDouble(), 1500.0);
    QCOMPARE(activeRec.value(qsl("JahresZins")).toDouble(), 50.0);
    QCOMPARE(r2(activeRec.value(qsl("ZinsRate")).toDouble()), 1.67);
    QCOMPARE(activeRec.value(qsl("MittelZins")).toDouble(), 1.5);

    const QVector<QStringList> inactiveInfo{overviewShortInfo(sqlOverviewInActiveContracts)};
    QCOMPARE(inactiveInfo.size(), 7);
    const QSqlRecord inactiveRec{executeSingleRecordSql(sqlOverviewInActiveContracts)};
    QVERIFY(not inactiveRec.isEmpty());
    QCOMPARE(inactiveRec.value(qsl("AnzahlKreditoren")).toInt(), 1);
    QCOMPARE(inactiveRec.value(qsl("AnzahlVertraege")).toInt(), 1);
    QCOMPARE(inactiveRec.value(qsl("GesamtVolumen")).toDouble(), 3000.0);
    QCOMPARE(inactiveRec.value(qsl("MittlererVertragswert")).toDouble(), 3000.0);
    QCOMPARE(inactiveRec.value(qsl("JahresZins")).toDouble(), 90.0);
    QCOMPARE(inactiveRec.value(qsl("ZinsRate")).toDouble(), 3.0);
    QCOMPARE(inactiveRec.value(qsl("MittelZins")).toDouble(), 3.0);

    const QVector<QStringList> allInfo{overviewShortInfo(sqlOverviewAllContracts)};
    QCOMPARE(allInfo.size(), 7);
    const QSqlRecord allRec{executeSingleRecordSql(sqlOverviewAllContracts)};
    QVERIFY(not allRec.isEmpty());
    QCOMPARE(allRec.value(qsl("AnzahlKreditoren")).toInt(), 1);
    QCOMPARE(allRec.value(qsl("AnzahlVertraege")).toInt(), 3);
    QCOMPARE(allRec.value(qsl("GesamtVolumen")).toDouble(), 6000.0);
    QCOMPARE(allRec.value(qsl("MittlererVertragswert")).toDouble(), 2000.0);
    QCOMPARE(allRec.value(qsl("JahresZins")).toDouble(), 140.0);
    QCOMPARE(r2(allRec.value(qsl("ZinsRate")).toDouble()), 2.33);
    QCOMPARE(allRec.value(qsl("MittelZins")).toDouble(), 2.0);
}

void test_views::test_perpetualInvestmentBookings_executesForOpenInvestment()
{
    dbConfig::writeValue(ZINSUSANCE, qsl("act/act"));

    const tableindex_t investmentId{saveNewInvestment(250,
                                                      QDate(2026, 1, 1),
                                                      EndOfTheFuckingWorld,
                                                      qsl("Offene Testanlage"))};
    QVERIFY(isValidRowId(investmentId));

    const creditorId_t creditorId{insertMinimalCreditor()};
    QVERIFY(isValidRowId(creditorId.v));

    contract cont;
    cont.initContractDefaults(creditorId);
    cont.setInterestRate(2.5);
    cont.setInterestModel(interestModel::reinvest);
    cont.setConclusionDate(QDate(2026, 1, 1));
    cont.setInvestmentId(investmentId);
    cont.saveNewContract();
    QVERIFY(isValidRowId(cont.id().v));
    QVERIFY(cont.bookInitialPayment(QDate(2026, 1, 15), 10000.));
    QCOMPARE(cont.annualSettlement(2026), 2026);

    const double interest{ZinsesZins_act_act(2.5, 10000.0, QDate(2026, 1, 15), QDate(2026, 12, 31), true)};
    const QVector<booking> bookings{getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"))};
    QCOMPARE(bookings.size(), 2);
    QCOMPARE(bookings[1], booking(cont.id(), bookingType::annualInterestDeposit, QDate(2026, 12, 31), interest));

    const QVector<QStringList> data{perpetualInvestment_bookings()};
    QVERIFY(not data.isEmpty());
    QCOMPARE(data.size(), 2);
    QCOMPARE(data[0].size(), 7);
    QCOMPARE(data[1].size(), 7);

    QCOMPARE(data[0][1], qsl("15.01.2026"));
    QCOMPARE(data[0][2], qsl("1"));
    QCOMPARE(data[0][3], s_d2euro(10000.0));
    QCOMPARE(data[0][4], qsl("1"));
    QCOMPARE(data[0][5], s_d2euro(10000.0));
    QCOMPARE(data[0][6], s_d2euro(10000.0));

    QCOMPARE(data[1][1], qsl("31.12.2026"));
    QCOMPARE(data[1][2], qsl("1"));
    QCOMPARE(data[1][3], s_d2euro(interest));
    QCOMPARE(data[1][4], qsl("1"));
    QCOMPARE(data[1][5], s_d2euro(10000.0 + interest));
    QCOMPARE(data[1][6], s_d2euro(10000.0));
}

void test_views::test_perpetualInvestmentBookings_keepsFinalizedContractsPositiveForOneYear()
{
    dbConfig::writeValue(ZINSUSANCE, qsl("act/act"));

    const tableindex_t investmentId{saveNewInvestment(250,
                                                      QDate(2026, 1, 1),
                                                      EndOfTheFuckingWorld,
                                                      qsl("Offene Testanlage"))};
    QVERIFY(isValidRowId(investmentId));

    const creditorId_t creditorId{insertMinimalCreditor()};
    QVERIFY(isValidRowId(creditorId.v));

    // stays active for the whole year: a single deposit plus its year-end interest
    contract activeContract;
    activeContract.initContractDefaults(creditorId);
    activeContract.setInterestRate(2.5);
    activeContract.setInterestModel(interestModel::reinvest);
    activeContract.setConclusionDate(QDate(2026, 1, 1));
    activeContract.setInvestmentId(investmentId);
    activeContract.saveNewContract();
    QVERIFY(isValidRowId(activeContract.id().v));
    QVERIFY(activeContract.bookInitialPayment(QDate(2026, 1, 15), 10000.));
    QCOMPARE(activeContract.annualSettlement(2026), 2026);
    const double activeInterest{ZinsesZins_act_act(2.5, 10000.0, QDate(2026, 1, 15), QDate(2026, 12, 31), true)};

    // deposited, then terminated mid-year: must still count toward maxInvestNbr/maxInvestSum
    // until its own 1-year window (from first payment) runs out, per CLAUDE.md deviation #4
    contract finalizedContract;
    finalizedContract.initContractDefaults(creditorId);
    finalizedContract.setInterestRate(2.5);
    finalizedContract.setInterestModel(interestModel::reinvest);
    finalizedContract.setConclusionDate(QDate(2026, 2, 1));
    finalizedContract.setInvestmentId(investmentId);
    finalizedContract.saveNewContract();
    QVERIFY(isValidRowId(finalizedContract.id().v));
    QVERIFY(finalizedContract.bookInitialPayment(QDate(2026, 2, 15), 20000.));

    double finInterest{0.};
    double finPayout{0.};
    QVERIFY(finalizedContract.finalize(false, QDate(2026, 6, 1), finInterest, finPayout));
    const double expectedFinInterest{ZinsesZins_act_act(2.5, 20000.0, QDate(2026, 2, 15), QDate(2026, 6, 1), true)};
    QCOMPARE(finInterest, expectedFinInterest);
    QCOMPARE(finPayout, 20000.0 + expectedFinInterest);

    const QVector<QStringList> data{perpetualInvestment_bookings()};
    QVERIFY(not data.isEmpty());
    QCOMPARE(data.size(), 4);

    QCOMPARE(data[0][1], qsl("15.01.2026"));
    QCOMPARE(data[0][2], qsl("1"));
    QCOMPARE(data[0][3], s_d2euro(10000.0));
    QCOMPARE(data[0][4], qsl("1"));
    QCOMPARE(data[0][5], s_d2euro(10000.0));
    QCOMPARE(data[0][6], s_d2euro(10000.0));

    QCOMPARE(data[1][1], qsl("15.02.2026"));
    QCOMPARE(data[1][2], qsl("1"));
    QCOMPARE(data[1][3], s_d2euro(20000.0));
    QCOMPARE(data[1][4], qsl("2"));
    QCOMPARE(data[1][5], s_d2euro(30000.0));
    QCOMPARE(data[1][6], s_d2euro(30000.0));

    // finalize() books the final interest (Typ4) and the full payout (Typ2) on the same day;
    // together they net out to exactly -principal on the "booked at this date" column, while
    // the interest still adds to the rolling incl.-interest total.
    QCOMPARE(data[2][1], qsl("01.06.2026"));
    QCOMPARE(data[2][2], qsl("2"));
    QCOMPARE(data[2][3], s_d2euro(-20000.0));
    QCOMPARE(data[2][4], qsl("2"));
    QCOMPARE(data[2][5], s_d2euro(30000.0 + expectedFinInterest));
    QCOMPARE(data[2][6], s_d2euro(30000.0));

    // the terminated contract keeps counting (AnzahlVerträge stays 2) even though it has no
    // more bookings from here on - this is the behavior deviation #4 fixed.
    QCOMPARE(data[3][1], qsl("31.12.2026"));
    QCOMPARE(data[3][2], qsl("1"));
    QCOMPARE(data[3][3], s_d2euro(activeInterest));
    QCOMPARE(data[3][4], qsl("2"));
    QCOMPARE(data[3][5], s_d2euro(30000.0 + expectedFinInterest + activeInterest));
    QCOMPARE(data[3][6], s_d2euro(30000.0));
}

void test_views::test_perpetualInvestmentBookings_skipsZeroNetValueChangeDates()
{
    dbConfig::writeValue(ZINSUSANCE, qsl("act/act"));

    const tableindex_t investmentId{saveNewInvestment(250,
                                                      QDate(2026, 1, 1),
                                                      EndOfTheFuckingWorld,
                                                      qsl("Offene Testanlage"))};
    QVERIFY(isValidRowId(investmentId));

    const creditorId_t creditorId{insertMinimalCreditor()};
    QVERIFY(isValidRowId(creditorId.v));

    // interestModel::payout means thesaurierend==0: annualSettlement() books the interest
    // both as a payout (Typ2) and as a same-amount recognition record (Typ8) on the same
    // date. Since relevanteBuchungen only counts Typ4/Typ8 when thesaurierend != 0, neither
    // contributes to the rolling totals here, so this date's net value change is exactly
    // zero and the view must skip it as a separate row.
    contract cont;
    cont.initContractDefaults(creditorId);
    cont.setInterestRate(2.5);
    cont.setInterestModel(interestModel::payout);
    cont.setConclusionDate(QDate(2026, 1, 1));
    cont.setInvestmentId(investmentId);
    cont.saveNewContract();
    QVERIFY(isValidRowId(cont.id().v));
    QVERIFY(cont.bookInitialPayment(QDate(2026, 1, 15), 10000.));
    QCOMPARE(cont.annualSettlement(2026), 2026);

    const QVector<booking> bookings{getBookings(cont.id(), BeginingOfTime, EndOfTheFuckingWorld, qsl("id ASC"))};
    QCOMPARE(bookings.size(), 3);

    const QVector<QStringList> data{perpetualInvestment_bookings()};
    QVERIFY(not data.isEmpty());
    QCOMPARE(data.size(), 1);
    QCOMPARE(data[0].size(), 7);

    QCOMPARE(data[0][1], qsl("15.01.2026"));
    QCOMPARE(data[0][2], qsl("1"));
    QCOMPARE(data[0][3], s_d2euro(10000.0));
    QCOMPARE(data[0][4], qsl("1"));
    QCOMPARE(data[0][5], s_d2euro(10000.0));
    QCOMPARE(data[0][6], s_d2euro(10000.0));
}

void test_views::test_perpetualInvestmentBookings_referenceCases_data()
{
    QTest::addColumn<piScenario>("scenario");

    {
        piScenario basicReinvestYearEnd;
        basicReinvestYearEnd.name = qsl("basic_reinvest_year_end");
        basicReinvestYearEnd.actions = {
            {piAction::kind::openContract, 0, QDate(2026, 1, 15), 10000.0, 2.5, interestModel::reinvest, 0},
            {piAction::kind::annualSettlement, 0, {}, 0.0, 0.0, interestModel::reinvest, 2026}
        };
        const double interest{ZinsesZins_act_act(2.5, 10000.0, QDate(2026, 1, 15), QDate(2026, 12, 31), true)};
        basicReinvestYearEnd.expectedRows = {
            {QDate(2026, 1, 15), 1, 10000.0, 1, 10000.0, 10000.0},
            {QDate(2026, 12, 31), 1, interest, 1, 10000.0 + interest, 10000.0}
        };
        QTest::newRow("basic_reinvest_year_end") << basicReinvestYearEnd;
    }

    {
        // a contract that gets terminated the same day as its first (and only) payment is
        // still a real, reachable state via contract::finalize() - unlike the previous
        // version of this test, which faked a "deleted contract with a deposit and nothing
        // else" state directly via SQL, a state the application itself can never produce.
        piScenario terminatedShortlyAfterDeposit;
        terminatedShortlyAfterDeposit.name = qsl("terminated_contract_shortly_after_deposit");
        terminatedShortlyAfterDeposit.actions = {
            {piAction::kind::openContract, 0, QDate(2026, 2, 15), 20000.0, 2.0, interestModel::reinvest, 0},
            {piAction::kind::terminate, 0, QDate(2026, 2, 15), 0.0, 0.0, interestModel::reinvest, 0}
        };
        terminatedShortlyAfterDeposit.expectedRows = {
            // deposit + zero-day final interest (Typ4, 0.0) + full payout (Typ2) all land on
            // the same date; they net to 0 on "booked at this date" but the deposit still
            // counts toward the rolling totals and the contract count.
            {QDate(2026, 2, 15), 3, 0.0, 1, 20000.0, 20000.0}
        };
        QTest::newRow("terminated_contract_shortly_after_deposit") << terminatedShortlyAfterDeposit;
    }

    // Add future manually checked scenarios here. The test runner below is generic on purpose.
}

void test_views::test_perpetualInvestmentBookings_referenceCases()
{
    QFETCH(piScenario, scenario);
    runPerpetualInvestmentActions(scenario);
    const QVector<QStringList> data{perpetualInvestment_bookings()};
    QVERIFY2(not data.isEmpty(), scenario.name.toUtf8().constData());
    comparePerpetualInvestmentOverview(data, scenario.expectedRows);
}

// Todo?? insert views for SQLite User

//void test_views::test_stat_activateContract_reinvesting()
//{
//    creditor c(saveRandomCreditor());
//    contract cont;
//    cont.initRandom(c.id());
//    double v =1000.;
//    cont.setPlannedInvest(v);
//    double ir =3.3;
//    cont.setInterestRate(ir);
//    cont.setInterestModel(interestModel::reinvest);
//    cont.saveNewContract();

//    // passive contract
//    dbStats expected;
//    expected.addContract(v, ir, dbStats::payoutType::thesa, c.id());
//    QCOMPARE(expected, getStatistic());
//    // active contract
//    cont.activate(QDate::currentDate(), cont.plannedInvest());
//    // setup expected ...
//    expected.activateContract(v, cont.plannedInvest(), ir, dbStats::payoutType::thesa, c.id());
//    QCOMPARE(expected, getStatistic());
//}

//void test_views::test_stat_activateContract_wpayout()
//{
//    creditor c(saveRandomCreditor());
//    contract cont;
//    cont.initRandom(c.id());
//    double v =1000.;
//    cont.setPlannedInvest(v);
//    double ir =3.3;
//    cont.setInterestRate(ir);
//    cont.setInterestModel(interestModel::payout);
//    cont.saveNewContract();

//    // passive contract
//    dbStats expected;
//    // setup expected ...
//    expected.addContract(v, ir, dbStats::payoutType::pout, c.id());
//    QCOMPARE(expected, getStatistic());
//    // active contract
//    cont.activate(QDate::currentDate(), cont.plannedInvest());
//    // setup expected ...
//    expected.activateContract(v, cont.plannedInvest(), ir, dbStats::payoutType::pout, c.id());
//    QCOMPARE(expected, getStatistic());
//}


//void test_views::test_stat_create_activate_multipleContract()
//{
//    // contrat 1
//    creditor c(saveRandomCreditor());
//    contract first;
//    first.initRandom(c.id());
//    double v1 =1000.;
//    first.setPlannedInvest(v1);
//    double ir1 =3.3;
//    first.setInterestRate(ir1);
//    first.setInterestModel(interestModel::reinvest);
//    first.saveNewContract();

//    dbStats expected;
//    expected.addContract(v1, ir1, dbStats::payoutType::thesa, c.id());
//    QCOMPARE(expected, getStatistic());

//    // passive contract 2
//    contract second;
//    second.initRandom(c.id());
//    double v2 =2000.;
//    second.setPlannedInvest(v2);
//    //...todo: setup expected ...
//    double ir2 =1.66;
//    second.setInterestRate(ir2);
//    second.setInterestModel(interestModel::reinvest);
//    second.saveNewContract();

//    expected.addContract(v2, ir2, dbStats::payoutType::thesa, c.id());
//    QCOMPARE(expected, getStatistic());

//    // passive contract 3
//    contract third;
//    third.initRandom(c.id());
//    double v3 =1000.;
//    third.setPlannedInvest(v3);
//    //...todo: setup expected ...
//    double ir3 =3.3;
//    third.setInterestRate(ir3);
//    //...todo: setup expected ...
//    third.setInterestModel(interestModel::payout);
//    third.saveNewContract();

//    expected.addContract(v3, ir3, dbStats::payoutType::pout, c.id());
//    QCOMPARE(expected, getStatistic());

//    third.activate(third.conclusionDate().addYears(1), third.plannedInvest());
//    second.activate(second.conclusionDate().addYears(1), second.plannedInvest());

//    expected.activateContract( v3, v3, ir3, dbStats::payoutType::pout, c.id());
//    expected.activateContract(v2, v2, ir2, dbStats::payoutType::thesa, c.id());
//    QCOMPARE(expected, getStatistic());

//    first.activate(first.conclusionDate().addYears(1), first.plannedInvest());

//    expected.activateContract(v1, v1, ir1, dbStats::payoutType::thesa, c.id());
//    QCOMPARE(expected, getStatistic());
//}

//void test_views::test_stat_annualSettelemnt()
//{
//    dbStats expected;
//    QDate conclusionDate (2000, 1, 1),  activationDate(2000, 6, 30);
//    /* have contracts and test statistics over an annual settlement
//    *  having 5 contracts:
//    *  1 inactive, 2 activ & reinvesting, 2 active & not reinvesting
//    * */
//    creditor inactiveCred(saveRandomCreditor());
//    contract inactive; inactive.initRandom(inactiveCred.id());
//    inactive.setPlannedInvest(5000.);
//    inactive.saveNewContract();
//    qInfo().noquote() << inactive.toString(qsl("Inactive Contract:")) << qsl("\n");
//    expected =getStatistic();

//    creditor activeCred1(saveRandomCreditor());
//    contract activeReInv1; activeReInv1.initRandom(activeCred1.id());
//    activeReInv1.setInterestModel(interestModel::reinvest);
//    double v =1000, ir =0.5; // %
//    activeReInv1.setPlannedInvest(v);
//    activeReInv1.setInterestRate(ir);
//    activeReInv1.setConclusionDate(conclusionDate);
//    activeReInv1.saveNewContract();
//    expected.addContract(v, ir, dbStats::payoutType::thesa, activeCred1.id());
//    activeReInv1.activate(activationDate, activeReInv1.plannedInvest());
//    expected.activateContract(v, v, ir, dbStats::payoutType::thesa,activeCred1.id());

//    contract activeReInv2; activeReInv2.initRandom(activeCred1.id());
//    activeReInv2.setInterestModel(interestModel::reinvest);
//    activeReInv2.setPlannedInvest(2 *v);
//    activeReInv2.setInterestRate(2 *ir);
//    activeReInv2.setConclusionDate(conclusionDate);
//    activeReInv2.saveNewContract();
//    expected.addContract(2*v, 2*ir, dbStats::payoutType::thesa, activeCred1.id());
//    activeReInv2.activate(activationDate, activeReInv2.plannedInvest());
//    expected.activateContract(2*v, 2*v, 2*ir,dbStats::payoutType::thesa, activeCred1.id());

//    creditor activeCred2(saveRandomCreditor());
//    contract activeNonReIn1;activeNonReIn1.initRandom(activeCred2.id());
//    activeNonReIn1.setInterestModel(interestModel::payout);
//    activeNonReIn1.setPlannedInvest(3 *v);
//    activeNonReIn1.setInterestRate(3 *ir);
//    activeNonReIn1.setConclusionDate(conclusionDate);
//    activeNonReIn1.saveNewContract();
//    expected.addContract(3*v, 3*ir, dbStats::payoutType::pout, activeCred2.id());
//    activeNonReIn1.activate(activationDate, activeNonReIn1.plannedInvest());
//    expected.activateContract(3*v, 3*v, 3*ir, dbStats::payoutType::pout, activeCred2.id());

//    contract activeNonReIn2;activeNonReIn2.initRandom(activeCred2.id());
//    activeNonReIn2.setInterestModel(interestModel::payout);
//    activeNonReIn2.setPlannedInvest(4 *v);
//    activeNonReIn2.setInterestRate(4 *ir);
//    activeNonReIn2.setConclusionDate(conclusionDate);
//    activeNonReIn2.saveNewContract();
//    expected.addContract(4*v, 4*ir, dbStats::payoutType::pout, activeCred2.id());
//    activeNonReIn2.activate(activationDate, activeNonReIn2.plannedInvest());
//    expected.activateContract(4*v, 4*v, 4*ir, dbStats::payoutType::pout, activeCred2.id());


////    /* values:
////     * v=1000
////    inactive: random
////    active, reinvesting: 1000-0.5%, 2000-1.0%
////    active, not reinv. : 3000-1.5%, 4000-2.0%
////    sum: 10 *v = 10000
////    avg: 1.25%, w avg 1.5%
////    */

//    qInfo().noquote() << "pre settlement check. Expected Values: " << "\n" << expected.toString();
//    QCOMPARE(expected, getStatistic());

//    /*
//     * run annual settlement and check statistics
//     */
//    activeReInv1.annualSettlement(activationDate.year());
//    activeReInv2.annualSettlement(activationDate.year());
//    activeNonReIn1.annualSettlement(activationDate.year());
//    activeNonReIn2.annualSettlement(activationDate.year());
//    qInfo().noquote() << activeReInv1.toString("Re1") << "\n" << activeReInv2.toString("Re2") << "\n" <<
//        activeNonReIn1.toString("nRe1") << "\n" << activeNonReIn2.toString("nRe2");

//    // setup expected ...
//    expected.reinvest(v, ir, 180);
//    expected.reinvest(2* v, 2* ir, 180);
//    // no change with inactive contracts

//    QCOMPARE(expected, getStatistic());
//}

////void test_views::test_stat_extend_contract_sameYear()
////{
////    QDate activationDate(2020, 3, 31);
////    creditor c(saveRandomCreditor());
////    contract cont;
////    cont.initRandom(c.id());
////    double v =1000.;
////    cont.setPlannedInvest(v);
////    double ir =3.3;
////    cont.setInterestRate(ir);
////    cont.setInterestModel(interestModel::payout);
////    cont.setConclusionDate(activationDate.addMonths(1));
////    cont.saveNewContract();

////    // active contract
////    cont.activate(activationDate, cont.plannedInvest());
////    dbStats expected( dbStats::calculate);
////    cont.deposit(activationDate.addDays(36), v);
////    expected.changeContract(v, ir, 36, dbStats::payoutType::pout);

////    QCOMPARE(expected, getStatistic());

////}

////void test_views::test_stat_mny_contracts()
////{
////    {
////        dbgTimer timer("create many contracts");
////        saveRandomCreditors(44);
////        saveRandomContracts(50);    // contract date: 2 years back or less
////        activateRandomContracts(90);// activation date: > contract date
////        // 500 contracts < 1min
////    }
////    {
////        dbgTimer timer("statistics");
////        dbStats stats(true);
////        qInfo().noquote() << stats.toString();
////        // 500 contracts < 200ms
////    }

////}
