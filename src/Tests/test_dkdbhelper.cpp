#include "test_dkdbhelper.h"

#include "../DKV2/helper_core.h"
#include "../DKV2/helpersql.h"
#include "../DKV2/dbstructure.h"
#include "../DKV2/tabledatainserter.h"
#include "../DKV2/appconfig.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"

#include "testhelper.h"

#include <QtTest/QTest>

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



