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
    init_additionalTables();
}

void test_dkdbhelper::init()
{   LOG_CALL;
    if (QFile::exists(testDbFilename))
        QVERIFY(QFile::remove(testDbFilename));
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", testCon);
    db.setDatabaseName(testDbFilename);
    QVERIFY(db.open());
    QSqlQuery enableRefInt(db);
    QVERIFY2(enableRefInt.exec("PRAGMA foreign_keys = ON"),
             enableRefInt.lastError().text().toLocal8Bit().data());
    create_DK_database(testDb());
}

void test_dkdbhelper::cleanup()
{   LOG_CALL;
    QSqlDatabase::database().removeDatabase(testCon);
    QSqlDatabase::database().close();
    if (QFile::exists(testDbFilename))
        QVERIFY(QFile::remove(testDbFilename));
}

void test_dkdbhelper::test_querySingleValueInvalidQuery()
{   LOG_CALL;
    QString sql ("SELECT NOTEXISTINGFIELD FROM NOTEXISTINGTABLE WHERE NOTEXISTINGFIELD='0'");
    QVERIFY2(QVariant::Invalid == ExecuteSingleValueSql(sql).type(),
             "Invalid single value sql has poditiv result");
}

void test_dkdbhelper::test_querySingleValue()
{   LOG_CALL;
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QVariant::Int))
            .append(dbfield("f")));
    s.createDb(testDb());
    TableDataInserter tdi(s["t"]);
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo");
    tdi.InsertData(testDb());
    QVariant hallo = ExecuteSingleValueSql("SELECT [f] FROM [t] WHERE id=1", testDb());
    QVERIFY2(hallo.toString() == "Hallo", "ExecuteSingleValueSql failed");
}

void test_dkdbhelper::test_querySingleValue_multipleResults()
{   LOG_CALL;
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QVariant::Int))
            .append(dbfield("f")));
    s.createDb(testDb());
    TableDataInserter tdi(s["t"]);
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo");
    tdi.InsertData(testDb());
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo1");
    tdi.InsertData(testDb());
    QVariant hallo = ExecuteSingleValueSql("SELECT [f] FROM [t] WHERE id=1", testDb());
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
//    s.createDb(QSqlDatabase::database(testDb()));

//    TableDataInserter tdi(dkdbstructur["Vertraege"]);
//    tdi.setValue("Betrag", 100.);
//    tdi.setValue("Wert", 101.);
//    tdi.setValue("aktiv", true);
//    tdi.setValue("thesaurierend", true);
//    tdi.InsertData(QSqlDatabase::database(testDb()));
//    tdi.InsertData(QSqlDatabase::database(testDb()));
//    tdi.setValue("Betrag", 200.);
//    tdi.setValue("Wert", 201.);
//    tdi.setValue("aktiv", false);
//    tdi.setValue("thesaurierend", true);
//    tdi.InsertData(QSqlDatabase::database(testDb()));
//    tdi.InsertData(QSqlDatabase::database(testDb()));
//    DbSummary dbs;
//    calculateSummary(dbs, testDb());
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
    s.createDb(testDb());

    QVERIFY2(ensureTable(s["t"], testDb()), "ensure table for existing table failed");
}

void test_dkdbhelper::test_ensureTable_notExistingTable()
{   LOG_CALL;
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QVariant::Int))
            .append(dbfield("f")));
    s.createDb(testDb());

    dbtable notExistingTable("s");
    notExistingTable.append(dbfield("id"));

    QVERIFY2( ensureTable(notExistingTable, testDb()), "ensure table for not existing table failed");
    QVERIFY2(tableExists("s", testDb()), "ensure Table of not existing table did not create the table");
}
