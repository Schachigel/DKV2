#include <QCoreApplication>
#include <QFile>
#include <QSqlDatabase>
#include <QString>
#include <QtTest>

#include "../dkv2/dbstructure.h"
#include "../dkv2/dbtable.h"
#include "../dkv2/dkdbhelper.h"
#include "../dkv2/filehelper.h"
#include "../dkv2/helper.h"

#include "tst_db.h"
// add necessary includes here

int tst_db::tableRecordCount(QString tname)
{
    QSqlQuery q(QSqlDatabase::database(testCon));
    if (q.exec("SELECT COUNT(*) FROM " + tname)) {
        q.next();
        qDebug() << "#Datensätze: " << q.record().value(0);
        return q.record().value(0).toInt();
    } else {
        qCritical() << "selecting data failed " << q.lastError() << "\n" << q.lastQuery() << endl;
        return -1;
    }
}

void tst_db::init()
{ //LOG_ENTRY_and_EXIT;
    if (QFile::exists(filename))
        QFile::remove(filename);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", testCon);
    db.setDatabaseName(filename);
    QVERIFY(db.open());
    QSqlQuery enableRefInt(db);
    QVERIFY2(enableRefInt.exec("PRAGMA foreign_keys = ON"),
             enableRefInt.lastError().text().toLocal8Bit().data());
}

void tst_db::cleanup()
{ //LOG_ENTRY_and_EXIT;
    QSqlDatabase::database().removeDatabase(testCon);
    QSqlDatabase::database().close();
    if (QFile::exists(filename))
        QFile::remove(filename);
}

void tst_db::test_createSimpleTable()
{
    LOG_ENTRY_and_EXIT;
    dbstructure s;
    dbtable t("t");
    dbfield f("f");
    t.append(f);
    s.appendTable(t);
    QVERIFY2(s.createDb(QSqlDatabase::database(testCon)), "Database was not created");
    QVERIFY2(QFile::exists(filename), "No database file found");
}

void tst_db::test_createSimpleTable2()
{
    dbstructure s = dbstructure()
                        .appendTable(dbtable("Ad").append(dbfield("vname")).append(dbfield("nname")))
                        .appendTable(dbtable("cities").append(dbfield("plz")));
    QVERIFY2(s.createDb(QSqlDatabase::database(testCon)), "Database was not created");
    QVERIFY2(QFile::exists(filename), "No database file found");
}

void tst_db::test_SimpleTableAddData()
{
    dbstructure s = dbstructure()
                        .appendTable(dbtable("Ad").append(dbfield("vname")).append(dbfield("nname")))
                        .appendTable(dbtable("cities").append(dbfield("plz")));

    QVERIFY2(s.createDb(QSqlDatabase::database(testCon)), "Database was not created");
    QVERIFY2(QFile::exists(filename), "No database file found");

    TableDataInserter tdi(s["Ad"]);
    tdi.setValue("vname", QVariant("Holger"));
    tdi.setValue("nname", "Mairon");
    QVERIFY(tdi.InsertData(QSqlDatabase::database(testCon)));
    QVERIFY(tableRecordCount("Ad") == 1);
}

void tst_db::test_createSimpleTable_wRefInt()
{
    LOG_ENTRY_and_EXIT;
    dbstructure s;
    dbtable parent("p");
    dbfield id("i", QVariant::Int, "PRIMARY KEY AUTOINCREMENT");
    parent.append(id);
    s.appendTable(parent);
    dbtable child("c");
    dbfield childId("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT");
    dbfield parentId("parent", QVariant::Int, "", parent["id"],
                     dbfield::refIntOption::onDeleteCascade);
    child.append(childId);
    child.append(parentId);
    s.appendTable(child);
    QVERIFY2(s.createDb(QSqlDatabase::database(testCon)), "Database was not created");
    QVERIFY2(QFile::exists(filename), "No database file found");
}

void tst_db::test_createSimpleTable_wRefInt2()
{
    LOG_ENTRY_and_EXIT;
    dbstructure s = dbstructure()
                        .appendTable(dbtable("p").append(
                            dbfield("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT")));
                        s.appendTable(
                            dbtable("c")
                                .append(dbfield("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT"))
                                .append(dbfield("pid",
                                                QVariant::Int,
                                                "",
                                                s["p"]["id"],
                                                dbfield::refIntOption::onDeleteCascade)));
    QVERIFY2(s.createDb(QSqlDatabase::database(testCon)), "Database was not created");
    QVERIFY2(QFile::exists(filename), "No database file found");
}

void tst_db::test_addRecords_wDep()
{
    LOG_ENTRY_and_EXIT;
    dbstructure s = dbstructure()
                        .appendTable(
                            dbtable("p")
                                .append(dbfield("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT"))
                                .append(dbfield("name")));
                        s.appendTable(
                            dbtable("c")
                                .append(dbfield("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT"))
                                .append(dbfield("pid",
                                                QVariant::Int,
                                                "",
                                                s["p"]["id"],
                                                dbfield::refIntOption::onDeleteCascade)));
    QVERIFY2(s.createDb(QSqlDatabase::database(testCon)), "Database was not created");
    QVERIFY2(QFile::exists(filename), "No database file found");

    TableDataInserter tdi(s["p"]);
    tdi.setValue("name", "Holger");
    QVERIFY(tdi.InsertData(QSqlDatabase::database(testCon)));
    QVERIFY(tableRecordCount("p") == 1);

    qDebug() << "add depending data sets" << endl;
    TableDataInserter tdiChild1(s["c"]);
    tdiChild1.setValue("pid", QVariant(1)); // should work
    QVERIFY(tdiChild1.InsertData(QSqlDatabase::database(testCon)));

    qDebug() << "add invalid depending data sets" << endl;
    TableDataInserter tdiChild2(s["c"]);
    tdiChild2.setValue("pid", 2); // should fail - no matching parent in table p
    QVERIFY(!tdiChild2.InsertData(QSqlDatabase::database(testCon)));
}

void tst_db::test_deleteRecord_wDep()
{
    LOG_ENTRY_and_EXIT;
    dbstructure s = dbstructure()
                        .appendTable(
                            dbtable("p")
                                .append(dbfield("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT"))
                                .append(dbfield("name")));
                        s.appendTable(
                            dbtable("c")
                                .append(dbfield("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT"))
                                .append(dbfield("pid",
                                                QVariant::Int,
                                                "",
                                                s["p"]["id"],
                                                dbfield::refIntOption::onDeleteCascade)));
    QVERIFY2(s.createDb(QSqlDatabase::database(testCon)), "Database was not created");
    QVERIFY2(QFile::exists(filename), "No database file found");

    TableDataInserter tdi(s["p"]);
    tdi.setValue("name", "Holger");
    QVERIFY(tdi.InsertData(QSqlDatabase::database(testCon)));
    QVERIFY(tableRecordCount("p") == 1);

    qDebug() << "add depending data sets" << endl;
    TableDataInserter tdiChild1(s["c"]);
    tdiChild1.setValue("pid", QVariant(1)); // should work
    QVERIFY(tdiChild1.InsertData(QSqlDatabase::database(testCon)));
    TableDataInserter tdiChild2(s["c"]);
    tdiChild2.setValue("pid", QVariant(1)); // second child to matching parent in table p
    QVERIFY(tdiChild2.InsertData(QSqlDatabase::database(testCon)));
    QVERIFY(tableRecordCount("p") == 1);
    QVERIFY(tableRecordCount("c") == 2);

    qDebug() << "removing connected datasets" << endl;
    QSqlQuery deleteQ(QSqlDatabase::database(testCon));
    deleteQ.exec("DELETE FROM p WHERE id = 1");
    QVERIFY(tableRecordCount("p") == 0);
    QVERIFY(tableRecordCount("c") == 0);
}

QTEST_MAIN(tst_db)
