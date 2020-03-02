#include <QtSql>
#include <QtTest>

#include "../DKV2/helper.h"
#include "../DKV2/sqlhelper.h"
#include "../DKV2/dkdbhelper.h"

#include "test_dkdbhelper.h"

void test_dkdbhelper::init()
{LOG_ENTRY_and_EXIT;
    if (QFile::exists(filename))
        QFile::remove(filename);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", testCon);
    db.setDatabaseName(filename);
    QVERIFY(db.open());
    QSqlQuery enableRefInt(db);
    QVERIFY2(enableRefInt.exec("PRAGMA foreign_keys = ON"),
             enableRefInt.lastError().text().toLocal8Bit().data());
}
void test_dkdbhelper::cleanup()
{LOG_ENTRY_and_EXIT;
    QSqlDatabase::database().removeDatabase(testCon);
    QSqlDatabase::database().close();
    if (QFile::exists(filename))
        QFile::remove(filename);
}

void test_dkdbhelper::test_querySingleValueInvalidQuery()
{
    QString sql ("SELECT NOTEXISTINGFIELD FROM NOTEXISTINGTABLE WHERE NOTEXISTINGFIELD='0'");
    QVERIFY2(QVariant::Invalid == ExecuteSingleValueSql(sql).type(),
             "Invalid single value sql has poditiv result");
}

void test_dkdbhelper::test_querySingleValue()
{
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QVariant::Int))
            .append(dbfield("f")));
    s.createDb(QSqlDatabase::database(testCon));
    TableDataInserter tdi(s["t"]);
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo");
    tdi.InsertData(QSqlDatabase::database(testCon));
    QVariant hallo = ExecuteSingleValueSql("SELECT [f] FROM [t] WHERE id=1", testCon);
    QVERIFY2(hallo.toString() == "Hallo", "ExecuteSingleValueSql failed");
}

void test_dkdbhelper::test_querySingleValue_multipleResults()
{
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QVariant::Int))
            .append(dbfield("f")));
    s.createDb(QSqlDatabase::database(testCon));
    TableDataInserter tdi(s["t"]);
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo");
    tdi.InsertData(QSqlDatabase::database(testCon));
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo1");
    tdi.InsertData(QSqlDatabase::database(testCon));
    QVariant hallo = ExecuteSingleValueSql("SELECT [f] FROM [t] WHERE id=1", testCon);
    QVERIFY2(hallo.type() == QVariant::Invalid , "ExecuteSingleValueSql failed");
}

void test_dkdbhelper::test_berechneZusammenfassung()
{
    dbstructure s = dbstructure()
        .appendTable(dbtable("Vertraege")
            .append(dbfield("Betrag", QVariant::Double))
            .append(dbfield("Wert", QVariant::Double))
            .append(dbfield("thesaurierend", QVariant::Bool))
            .append(dbfield("aktiv", QVariant::Bool)));
    s.createDb(QSqlDatabase::database(testCon));

    TableDataInserter tdi(s["Vertraege"]);
    tdi.setValue("Betrag", 100.);
    tdi.setValue("Wert", 101.);
    tdi.setValue("aktiv", true);
    tdi.setValue("thesaurierend", true);
    tdi.InsertData(QSqlDatabase::database(testCon));
    tdi.InsertData(QSqlDatabase::database(testCon));
    tdi.setValue("Betrag", 200.);
    tdi.setValue("Wert", 201.);
    tdi.setValue("aktiv", false);
    tdi.setValue("thesaurierend", true);
    tdi.InsertData(QSqlDatabase::database(testCon));
    tdi.InsertData(QSqlDatabase::database(testCon));
    DbSummary dbs;
    calculateSummary(dbs, testCon);
    QVERIFY(dbs.BetragAktive == 200.);
    QVERIFY(dbs.AnzahlAktive == 2);
    QVERIFY(dbs.WertAktive == 202.);
    QVERIFY(dbs.BetragPassive == 400.);

}

void test_dkdbhelper::test_ensureTable_existingTable()
{
    QSqlDatabase db =QSqlDatabase::database(testCon);
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QVariant::Int))
            .append(dbfield("f")));
    s.createDb(db);

    QVERIFY2(ensureTable(s["t"], db), "ensure table for existing table failed");
}

void test_dkdbhelper::test_ensureTable_notExistingTable()
{
    QSqlDatabase db =QSqlDatabase::database(testCon);
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QVariant::Int))
            .append(dbfield("f")));
    s.createDb(db);

    dbtable notExistingTable("s");
    notExistingTable.append(dbfield("id"));

    QVERIFY2( ensureTable(notExistingTable, db), "ensure table for not existing table failed");
    QVERIFY2(tableExists("s", testCon), "ensure Table of not existing table did not create the table");
}
