
#include <QtTest>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>

#include "../DKV2/tabledatainserter.h"
#include "../DKV2/dbtable.h"
#include "../DKV2/creditor.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/helperfile.h"
#include "../DKV2/helper.h"

#include "testhelper.h"
#include "test_db.h"
// add necessary includes here

void test_db::initTestCase()
{
    init_DKDBStruct();
}

void test_db::init()
{   LOG_CALL;
    initTestDb();
}

void test_db::cleanup()
{   LOG_CALL;
    cleanupTestDb();
}

void test_db::test_init_and_cleanup()
{
}

void test_db::test_createSimpleTable()
{
    dbstructure s;
    dbtable t("t");
    dbfield f("f");
    t.append(f);
    s.appendTable(t);
    QVERIFY2(s.createDb(), "Database was not created");

    QVERIFY2(dbHasTable("t"), "Table was not created");
    QVERIFY2(dbTableHasField("t", "f"), "Field was not created");
}

void test_db::test_createSimpleTable2()
{
    dbstructure s = dbstructure()
                        .appendTable(dbtable("Ad").append(dbfield("vname")).append(dbfield("nname")))
                        .appendTable(dbtable("cities").append(dbfield("plz")));
    QVERIFY2(s.createDb(), "Database was not created");

    QVERIFY2(dbHasTable("Ad"), "Table Ad not found");
    QVERIFY2(dbTableHasField("Ad", "vname"), "Field not found");
    QVERIFY2(dbTableHasField("Ad", "nname"), "Field not found");
    QVERIFY2(dbHasTable("cities"), "Table cities not found");
    QVERIFY2(dbTableHasField("cities", "plz"), "Field not found");
}

void test_db::test_SimpleTableAddData()
{
    dbstructure s = dbstructure()
                        .appendTable(dbtable("Ad").append(dbfield("vname")).append(dbfield("nname")))
                        .appendTable(dbtable("cities").append(dbfield("plz")));

    QVERIFY2(s.createDb(), "Database was not created");
    QVERIFY2(QFile::exists(testDbFilename), "No database file found");

    TableDataInserter tdi(s["Ad"]);
    tdi.setValue("vname", QVariant("Holger"));
    tdi.setValue("nname", "Mairon");
    QVERIFY( -1 < tdi.InsertData());
    QVERIFY(tableRecordCount("Ad") == 1);
}

void test_db::test_createSimpleTable_wRefInt()
{
    LOG_CALL;
    dbstructure s;
    dbtable parent("p");
    dbfield id("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT");
    parent.append(id);
    s.appendTable(parent);
    dbtable child("c");
    dbfield childId("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT");
    dbfield parentId("parent", QVariant::Int, "");
    child.append(childId);
    child.append(parentId);
    dbForeignKey fk(childId, parent["id"], "ON DELETE CASCADE", "");
    child.append(fk);
    s.appendTable(child);
    QVERIFY2(s.createDb(), "Database was not created");

    QVERIFY2(dbHasTable("p"), "table p was not created");
    QVERIFY2(dbTableHasField("p", "id"), "table p has no field id");
    QVERIFY2(dbHasTable("c"), "table p was not created");
    QVERIFY2(dbTableHasField("c", "id"), "table c has no field id");
    QVERIFY2(dbTableHasField("c", "parent"), "table c has no field parent");
}

void test_db::test_createSimpleTable_wRefInt2()
{
    LOG_CALL;
    dbstructure s = dbstructure()
                    .appendTable(dbtable("p")
                    .append(dbfield("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT")));
    dbtable c("c");
    c.append(dbfield("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT"));
    c.append(dbfield("pid", QVariant::Int));
    c.append(dbForeignKey(c["pid"], s["p"]["id"], "ON DELETE CASCADE", ""));
    s.appendTable( c);

    QVERIFY2(s.createDb(), "Database was not created");

    QVERIFY2(dbHasTable("p"), "table p was not created");
    QVERIFY2(dbTableHasField("p", "id"), "table p has no field id");
    QVERIFY2(dbHasTable("c"), "table p was not created");
    QVERIFY2(dbTableHasField("c", "id"), "table c has no field id");
    QVERIFY2(dbTableHasField("c", "pid"), "table c has no field pid");}

void test_db::test_addRecords_wDep()
{
    LOG_CALL;
    dbstructure s = dbstructure()
                        .appendTable(
                            dbtable("p")
                                .append(dbfield("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT"))
                                .append(dbfield("name")));

    dbtable c("c");
    c.append(dbfield("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT"));
    c.append(dbfield("pid", QVariant::Int));
    c.append(dbForeignKey(c["pid"], s["p"]["id"], "ON DELETE CASCADE", ""));
    s.appendTable( c);

    QVERIFY2(s.createDb(), "Database was not created");

    TableDataInserter tdi(s["p"]);
    tdi.setValue("name", "Holger");
    QVERIFY( 0 <= tdi.InsertData());
    QVERIFY(tableRecordCount("p") == 1);

    qDebug() << "add depending data sets" << Qt::endl;
    TableDataInserter tdiChild1(s["c"]);
    tdiChild1.setValue("pid", QVariant(1)); // should work
    QVERIFY( 0 <= tdiChild1.InsertData());

    qDebug() << "add INVALID depending data sets" << Qt::endl;
    TableDataInserter tdiChild2(s["c"]);
    tdiChild2.setValue("pid", 2); // should fail - no matching parent in table p
    QVERIFY( -1 == tdiChild2.InsertData());
}

void test_db::test_deleteRecord_wDep()
{
    LOG_CALL;
    dbstructure s = dbstructure()
                        .appendTable(
                            dbtable("p")
                                .append(dbfield("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT"))
                                .append(dbfield("name")));
    dbtable c("c");
    c.append(dbfield("id", QVariant::Int, "PRIMARY KEY AUTOINCREMENT"));
    c.append(dbfield("pid", QVariant::Int));
    c.append(dbForeignKey(c["pid"], s["p"]["id"], "ON DELETE CASCADE", ""));
    s.appendTable( c);

    QVERIFY2(s.createDb(), "Database was not created");
    QVERIFY2(QFile::exists(testDbFilename), "No database file found");

    TableDataInserter tdi(s["p"]);
    tdi.setValue("name", "Holger");
    QVERIFY( 0<= tdi.InsertData());
    QVERIFY(tableRecordCount("p") == 1);

    qDebug() << "add depending data sets" << Qt::endl;
    TableDataInserter tdiChild1(s["c"]);
    tdiChild1.setValue("pid", QVariant(1)); // should work
    QVERIFY( 0<= tdiChild1.InsertData());
    TableDataInserter tdiChild2(s["c"]);
    tdiChild2.setValue("pid", QVariant(1)); // second child to matching parent in table p
    QVERIFY( 0<= tdiChild2.InsertData());
    QVERIFY(tableRecordCount("p") == 1);
    QVERIFY(tableRecordCount("c") == 2);

    qDebug() << "removing connected datasets" << Qt::endl;
    QSqlQuery deleteQ;
    deleteQ.exec("DELETE FROM p WHERE id = 1");
    QVERIFY(tableRecordCount("p") == 0);
    QVERIFY(tableRecordCount("c") == 0);
}

void test_db::newDbIsValid()
{
    dbgTimer t;
    QVERIFY(fill_DkDbDefaultContent());
}

