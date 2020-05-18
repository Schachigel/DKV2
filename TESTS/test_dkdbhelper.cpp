#include <QtSql>
#include <QtTest>

#include "../DKV2/helper.h"
#include "../DKV2/sqlhelper.h"
#include "../DKV2/dkdbhelper.h"

#include "testhelper.h"
#include "test_dkdbhelper.h"

void test_dkdbhelper::initTestCase()
{   LOG_CALL;
    init_DKDBStruct();
}

void test_dkdbhelper::init()
{   LOG_CALL;
    initTestDb();
    create_DK_databaseContent();
}

void test_dkdbhelper::cleanup()
{   LOG_CALL;
    cleanupTestDb();
}

void test_dkdbhelper::test_querySingleValueInvalidQuery()
{   LOG_CALL;
    QString sql ("SELECT NOTEXISTINGFIELD FROM NOTEXISTINGTABLE WHERE NOTEXISTINGFIELD='0'");
    QVariant result;
    result = ExecuteSingleValueSql(sql);
    QVERIFY2(QVariant::Invalid == result.type(),
             "Invalid single value sql has poditiv result");
}

void test_dkdbhelper::test_querySingleValue()
{   LOG_CALL;
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QVariant::Int))
            .append(dbfield("f")));
    s.createDb();
    TableDataInserter tdi(s["t"]);
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo");
    tdi.InsertData();
    QVariant hallo = ExecuteSingleValueSql("SELECT [f] FROM [t] WHERE id=1");
    QVERIFY2(hallo.toString() == "Hallo", "ExecuteSingleValueSql failed");
}

void test_dkdbhelper::test_querySingleValue_multipleResults()
{   LOG_CALL;
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QVariant::Int))
            .append(dbfield("f")));
    s.createDb();
    TableDataInserter tdi(s["t"]);
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo");
    tdi.InsertData();
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo1");
    tdi.InsertData();
    QVariant hallo = ExecuteSingleValueSql("SELECT [f] FROM [t] WHERE id=1");
    QVERIFY2(hallo.type() == QVariant::Invalid , "ExecuteSingleValueSql failed");
}

void test_dkdbhelper::test_berechneZusammenfassung()
{
//    dbstructure s = dbstructure()
//        .appendTable(dbtable("Vertraege")
//            .append(dbfield("Betrag", QVariant::Double))
//            .append(dbfield("Wert", QVariant::Double))
//            .append(dbfield("thesaurierend", QVariant::Bool))
//            .append(dbfield("aktiv", QVariant::Bool)));
//    s.createDb(QSqlDatabase::database());

//    TableDataInserter tdi(dkdbstructur["Vertraege"]);
//    tdi.setValue("Betrag", 100.);
//    tdi.setValue("Wert", 101.);
//    tdi.setValue("aktiv", true);
//    tdi.setValue("thesaurierend", true);
//    tdi.InsertData(QSqlDatabase::database());
//    tdi.InsertData(QSqlDatabase::database());
//    tdi.setValue("Betrag", 200.);
//    tdi.setValue("Wert", 201.);
//    tdi.setValue("aktiv", false);
//    tdi.setValue("thesaurierend", true);
//    tdi.InsertData(QSqlDatabase::database());
//    tdi.InsertData(QSqlDatabase::database());
//    DbSummary dbs;
//    calculateSummary(dbs);
//    QCOMPARE(dbs.BetragAktive, 200);
//    QVERIFY2(dbs.BetragAktive == 100., "Betrag der Aktiven Verträge ist falsch");
//    QVERIFY(dbs.AnzahlAktive == 2);
//    QVERIFY(dbs.WertAktive == 202.);
//    QVERIFY(dbs.BetragPassive == 400.);

}

void test_dkdbhelper::test_ensureTable_existingTable()
{   LOG_CALL;
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QVariant::Int))
            .append(dbfield("f")));
    s.createDb();

    QVERIFY2(ensureTable(s["t"]), "ensure table for existing table failed");
}

void test_dkdbhelper::test_ensureTable_notExistingTable()
{   LOG_CALL;
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QVariant::Int))
            .append(dbfield("f")));
    s.createDb();

    dbtable notExistingTable("notExistingTable");
    notExistingTable.append(dbfield("id"));

    QVERIFY2(ensureTable(notExistingTable), "ensure table for not existing table failed");
    QVERIFY2(tableExists("notExistingTable"), "ensure Table of not existing table did not create the table");
}
