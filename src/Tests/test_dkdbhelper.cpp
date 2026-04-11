#include "test_dkdbhelper.h"

#include "../DKV2/helper_core.h"
#include "../DKV2/helpersql.h"
#include "../DKV2/dbstructure.h"
#include "../DKV2/tabledatainserter.h"
#include "../DKV2/appconfig.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"
#include "../DKV2/booking.h"

#include "testhelper.h"

#include <QtTest/QTest>

namespace {
int countTriggersForTable(const QString& tableName, const QSqlDatabase& db =QSqlDatabase::database())
{
    return executeSingleValueSql(qsl("SELECT COUNT(*) FROM sqlite_master WHERE type = ? AND tbl_name = ?"),
                                 QVector<QVariant>{qsl("trigger"), tableName},
                                 db).toInt();
}
}

void test_dkdbhelper::init()
{   LOG_CALL;
    initTestDkDb_InMemory();
}

void test_dkdbhelper::cleanup()
{   LOG_CALL;
    cleanupTestDb_InMemory();
}

void test_dkdbhelper::test_querySingleValueInvalidQuery()
{   LOG_CALL;
    QString sql ("SELECT NOTEXISTINGFIELD FROM NOTEXISTINGTABLE WHERE NOTEXISTINGFIELD='0'");
    QVariant result;
    result = executeSingleValueSql(sql);
    QVERIFY2( not result.isValid(),
             "Invalid single value sql has poditiv result");
}

void test_dkdbhelper::test_querySingleValue()
{   LOG_CALL;
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QMetaType::Int))
            .append(dbfield("f")));
    s.createDb();
    TableDataInserter tdi(s["t"]);
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo");
    tdi.InsertRecord();
    QVariant hallo = executeSingleValueSql("SELECT [f] FROM [t] WHERE id=1");
    QVERIFY2(hallo.toString() == "Hallo", "ExecuteSingleValueSql failed");
}

void test_dkdbhelper::test_querySingleValue_multipleResults()
{   LOG_CALL;
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QMetaType::Int))
            .append(dbfield("f")));
    s.createDb();
    TableDataInserter tdi(s["t"]);
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo");
    tdi.InsertRecord();

    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo1");
    tdi.InsertRecord();

    QVariant hallo = executeSingleValueSql("SELECT [f] FROM [t] WHERE id=1");
    QVERIFY2(not hallo.isValid(), "ExecuteSingleValueSql failed");
}

void test_dkdbhelper::test_nextContractLabelIndex_advancesOnSave()
{
    QVERIFY(fill_DkDbDefaultContent(QSqlDatabase::database(), false));
    dbConfig::writeValue(GMBH_INITIALS, qsl("TST"));
    dbConfig::writeValue(STARTINDEX, 5000);
    dbConfig::writeValue(NEXT_CONTRACT_LABEL_INDEX, 5000);

    const QString label1 = proposeContractLabel();
    QVERIFY(label1.endsWith(qsl("-005000")));

    creditor cred = saveRandomCreditor();
    contract c;
    c.initRandom(cred.id());
    c.setLabel(label1);
    QVERIFY(c.saveNewContract().v >= Minimal_contract_id.v);

    QCOMPARE(nextContractLabelIndex(), 5001);

    const QString label2 = proposeContractLabel();
    QVERIFY(label2.endsWith(qsl("-005001")));
}

void test_dkdbhelper::test_nextContractLabelIndex_legacyGuessFromLabels()
{
    QVERIFY(fill_DkDbDefaultContent(QSqlDatabase::database(), false));
    dbConfig::writeValue(GMBH_INITIALS, qsl("TST"));
    dbConfig::writeValue(STARTINDEX, 2000);

    creditor cred = saveRandomCreditor();
    contract c;
    c.initRandom(cred.id());
    c.setLabel(qsl("DK-TST-2026-002050"));
    QVERIFY(c.saveNewContract().v >= Minimal_contract_id.v);

    // Simulate a legacy DB: key does not exist yet.
    QVERIFY(executeSql_wNoRecords(qsl("DELETE FROM Meta WHERE Name = ?"),
                                  QVector<QVariant>{dbConfig::paramName(NEXT_CONTRACT_LABEL_INDEX)}));

    QCOMPARE(nextContractLabelIndex(), 2051);
}

void test_dkdbhelper::test_nextContractLabelIndex_legacyGuessHonorsHigherStartIndex()
{
    QVERIFY(fill_DkDbDefaultContent(QSqlDatabase::database(), false));
    dbConfig::writeValue(GMBH_INITIALS, qsl("TST"));
    dbConfig::writeValue(STARTINDEX, 3000);

    creditor cred = saveRandomCreditor();
    contract c;
    c.initRandom(cred.id());
    c.setLabel(qsl("DK-TST-2026-001500"));
    QVERIFY(c.saveNewContract().v >= Minimal_contract_id.v);

    // Simulate a legacy DB: key does not exist yet.
    QVERIFY(executeSql_wNoRecords(qsl("DELETE FROM Meta WHERE Name = ?"),
                                  QVector<QVariant>{dbConfig::paramName(NEXT_CONTRACT_LABEL_INDEX)}));

    QCOMPARE(nextContractLabelIndex(), 3000);
}

void test_dkdbhelper::test_fillDkDbDefaultContent_createsZeitstempelTriggers()
{
    QVERIFY(fill_DkDbDefaultContent(QSqlDatabase::database(), false));

    QCOMPARE(countTriggersForTable(creditor::tablename), 2);
    QCOMPARE(countTriggersForTable(contract::tnContracts), 2);
    QCOMPARE(countTriggersForTable(contract::tnExContracts), 2);
    QCOMPARE(countTriggersForTable(booking::tn_Buchungen), 2);
    QCOMPARE(countTriggersForTable(booking::tn_ExBuchungen), 2);

    QVERIFY(executeSql_wNoRecords(
        qsl("INSERT INTO Kreditoren (id, Vorname, Nachname, Strasse, Plz, Stadt, Zeitstempel) "
            "VALUES (1, 'Ada', 'Lovelace', 'Memory Lane 1', '68167', 'Mannheim', NULL)")));

    const QString zeitstempel =executeSingleValueSql(qsl("Zeitstempel"),
                                                     creditor::tablename,
                                                     qsl("id = 1")).toString();
    QVERIFY(not zeitstempel.isEmpty());
}

void test_dkdbhelper::test_postDbUpgradeActions_backfillsZeitstempelHistorically()
{
    TestTempDir tmp(this);
    QVERIFY(tmp.isValid());
    const QString filename =QDir(tmp.path()).filePath(qsl("upgrade.sqlite"));

    QVERIFY(createNewDatabaseFileWDefaultContent(filename, zs_30360, dkdbstructur, false));

    {
        autoDb db(filename, qsl("seed-old-db"));
        QVERIFY(db.db.isOpen());

        QVERIFY(executeSql_wNoRecords(
            qsl("INSERT INTO Kreditoren "
                "(id, Vorname, Nachname, Strasse, Plz, Stadt, Zeitstempel) "
                "VALUES (1, 'Ada', 'Lovelace', 'Memory Lane 1', '68167', 'Mannheim', NULL)"), db));

        QVERIFY(executeSql_wNoRecords(
            qsl("INSERT INTO Vertraege "
                "(id, KreditorId, Kennung, ZSatz, Betrag, thesaurierend, Vertragsdatum, Kfrist, AnlagenId, LaufzeitEnde, zActive, KueDatum, Zeitstempel) "
                "VALUES (1, 1, 'DK-TST-2026-000001', 150, 10000, 1, '2024-01-15', 6, NULL, '9999-12-31', TRUE, '9999-12-31', NULL)"), db));

        QVERIFY(executeSql_wNoRecords(
            qsl("INSERT INTO Buchungen "
                "(id, %1, %2, %3, %4, %5, Zeitstempel) "
                "VALUES (1, 1, '2024-02-20', 1, 10000, '1900-01-01', NULL)")
                .arg(booking::fn_bVertragsId,
                     booking::fn_bDatum,
                     booking::fn_bBuchungsArt,
                     booking::fn_bBetrag,
                     booking::fn_bModifiziert), db));

        QVERIFY(executeSql_wNoRecords(
            qsl("INSERT INTO exVertraege "
                "(id, KreditorId, Kennung, Anmerkung, ZSatz, Betrag, thesaurierend, Vertragsdatum, Kfrist, AnlagenId, LaufzeitEnde, zActive, KueDatum, Zeitstempel) "
                "VALUES (2, 1, 'DK-TST-2023-000002', '', 150, 10000, 1, '2023-05-10', 6, NULL, '2023-12-31', TRUE, '9999-12-31', NULL)"), db));

        QVERIFY(executeSql_wNoRecords(
            qsl("INSERT INTO %1 "
                "(id, %2, %3, %4, %5, %6, Zeitstempel) "
                "VALUES (2, 2, '2023-12-31', 8, 123, '1900-01-01', NULL)")
                .arg(booking::tn_ExBuchungen,
                     booking::fn_bVertragsId,
                     booking::fn_bDatum,
                     booking::fn_bBuchungsArt,
                     booking::fn_bBetrag,
                     booking::fn_bModifiziert), db));
    }

    QVERIFY(postDB_UpgradeActions(16, filename));

    {
        autoDb db(filename, qsl("verify-upgraded-db"));
        QVERIFY(db.db.isOpen());

        QCOMPARE(countTriggersForTable(creditor::tablename, db), 2);
        QCOMPARE(countTriggersForTable(contract::tnContracts, db), 2);
        QCOMPARE(countTriggersForTable(contract::tnExContracts, db), 2);
        QCOMPARE(countTriggersForTable(booking::tn_Buchungen, db), 2);
        QCOMPARE(countTriggersForTable(booking::tn_ExBuchungen, db), 2);

        QCOMPARE(executeSingleValueSql(qsl("Zeitstempel"), contract::tnContracts, qsl("id = 1"), db).toString(),
                 qsl("2024-01-15 00:00:00"));
        QCOMPARE(executeSingleValueSql(qsl("Zeitstempel"), booking::tn_Buchungen, qsl("id = 1"), db).toString(),
                 qsl("2024-02-20 00:00:00"));
        QCOMPARE(executeSingleValueSql(qsl("Zeitstempel"), contract::tnExContracts, qsl("id = 2"), db).toString(),
                 qsl("2023-05-10 00:00:00"));
        QCOMPARE(executeSingleValueSql(qsl("Zeitstempel"), booking::tn_ExBuchungen, qsl("id = 2"), db).toString(),
                 qsl("2023-12-31 00:00:00"));

        const QString creditorZeitstempel =executeSingleValueSql(qsl("Zeitstempel"),
                                                                 creditor::tablename,
                                                                 qsl("id = 1"),
                                                                 db).toString();
        QVERIFY(not creditorZeitstempel.isEmpty());

        QCOMPARE(executeSingleValueSql(qsl("Wert"), qsl("Meta"), qsl("Name = 'migration.17.Zeitstempel'"), db).toString(),
                 qsl("historical-backfill"));

        const QString oldContractTimestamp =executeSingleValueSql(qsl("Zeitstempel"), contract::tnContracts, qsl("id = 1"), db).toString();
        QVERIFY(executeSql_wNoRecords(qsl("UPDATE Vertraege SET Anmerkung = 'changed by test' WHERE id = 1"), db));
        const QString newContractTimestamp =executeSingleValueSql(qsl("Zeitstempel"), contract::tnContracts, qsl("id = 1"), db).toString();
        QVERIFY(oldContractTimestamp not_eq newContractTimestamp);
    }
}



