
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QtTest>

#include "../DKV2/dbstructure.h"
#include "../DKV2/dbtable.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/filehelper.h"
#include "../DKV2/helper.h"

#include "testhelper.h"
#include "tst_db.h"
// add necessary includes here

void tst_db::init()
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

void tst_db::cleanup()
{LOG_ENTRY_and_EXIT;
    QSqlDatabase::database().removeDatabase(testCon);
    QSqlDatabase::database().close();
    if (QFile::exists(filename))
        QFile::remove(filename);
}

void tst_db::test_createSimpleTable()
{

    dbstructure s;
    dbtable t("t");
    dbfield f("f");
    t.append(f);
    s.appendTable(t);
    QVERIFY2(s.createDb(QSqlDatabase::database(testCon)), "Database was not created");

    QVERIFY2(dbHasTable("t"), "Table was not created");
    QVERIFY2(dbTableHasField("t", "f"), "Field was not created");
}

void tst_db::test_createSimpleTable2()
{
    dbstructure s = dbstructure()
                        .appendTable(dbtable("Ad").append(dbfield("vname")).append(dbfield("nname")))
                        .appendTable(dbtable("cities").append(dbfield("plz")));
    QVERIFY2(s.createDb(QSqlDatabase::database(testCon)), "Database was not created");

    QVERIFY2(dbHasTable("Ad"), "Table Ad not found");
    QVERIFY2(dbTableHasField("Ad", "vname"), "Field not found");
    QVERIFY2(dbTableHasField("Ad", "nname"), "Field not found");
    QVERIFY2(dbHasTable("cities"), "Table cities not found");
    QVERIFY2(dbTableHasField("cities", "plz"), "Field not found");
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
    dbfield id("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT");
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

    QVERIFY2(dbHasTable("p"), "table p was not created");
    QVERIFY2(dbTableHasField("p", "id"), "table p has no field id");
    QVERIFY2(dbHasTable("c"), "table p was not created");
    QVERIFY2(dbTableHasField("c", "id"), "table c has no field id");
    QVERIFY2(dbTableHasField("c", "parent"), "table c has no field parent");
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

    QVERIFY2(dbHasTable("p"), "table p was not created");
    QVERIFY2(dbTableHasField("p", "id"), "table p has no field id");
    QVERIFY2(dbHasTable("c"), "table p was not created");
    QVERIFY2(dbTableHasField("c", "id"), "table c has no field id");
    QVERIFY2(dbTableHasField("c", "pid"), "table c has no field pid");}

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

void tst_db::dbfieldCopyConst()
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

    TableDataInserter tdi(s["p"]);
    tdi.setValue("name", "Holger");
    QVERIFY(tdi.InsertData(QSqlDatabase::database(testCon)));
    QVERIFY(tableRecordCount("p") == 1);
    dbfield cp(s["c"]["pid"]);
    QVERIFY(!cp.getReferenzeInfo().tablename.isEmpty());
    QVERIFY(!cp.getReferenzeInfo().name.isEmpty());
}

