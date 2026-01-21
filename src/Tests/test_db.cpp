#include "test_db.h"

#include "../DKV2/helper_core.h"
#include "../DKV2/helpersql.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/dbtable.h"
#include "../DKV2/tabledatainserter.h"

#include "testhelper.h"

// add necessary includes here

void test_db::initTestCase()
{
    init_DKDBStruct();
}

void test_db::init()
{   LOG_CALL;
    initTestDkDb_InMemory();
}

void test_db::cleanup()
{   LOG_CALL;
    cleanupTestDb_InMemory();
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

    TableDataInserter tdi(s["Ad"]);
    tdi.setValue("vname", QVariant("Holger"));
    tdi.setValue("nname", "Mairon");
    QVERIFY( isValidRowId(tdi.InsertRecord()));
    QVERIFY(rowCount("Ad") == 1);
}

void test_db::test_createSimpleTable_wRefInt()
{
    LOG_CALL;
    dbstructure s;
    dbtable parent("p");
    dbfield id("id", QMetaType::Int);
    id.setAutoInc (true);
    parent.append(id);
    s.appendTable(parent);
    dbtable child("c");
    dbfield childId("id", QMetaType::Int);
    childId.setAutoInc (true);
    dbfield parentId("parent", QMetaType::Int);
    child.append(childId);
    child.append(parentId);
    dbForeignKey fk(childId, parent["id"], ODOU_Action::CASCADE);
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
                    .append(dbfield("id", QMetaType::Int).setAutoInc ()));
    dbtable c("c");
    c.append(dbfield("id", QMetaType::Int).setAutoInc ());
    c.append(dbfield("pid", QMetaType::Int));
    c.append(dbForeignKey(c["pid"], s["p"]["id"], ODOU_Action::CASCADE));
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
                                .append(dbfield("id", QMetaType::Int).setAutoInc ())
                                .append(dbfield("name")));

    dbtable c("c");
    c.append(dbfield("id", QMetaType::Int).setAutoInc ());
    c.append(dbfield("pid", QMetaType::Int));
    c.append(dbForeignKey(c["pid"], s["p"]["id"], ODOU_Action::CASCADE));
    s.appendTable( c);

    QVERIFY2(s.createDb(), "Database was not created");

    TableDataInserter tdi(s["p"]);
    tdi.setValue("name", "Holger");
    QVERIFY( isValidRowId(tdi.InsertRecord()));
    QVERIFY(rowCount("p") == 1);

    qInfo() << "add depending data sets\n";
    TableDataInserter tdiChild1(s["c"]);
    tdiChild1.setValue("pid", QVariant(1)); // should work
    QVERIFY( isValidRowId (tdiChild1.InsertRecord()));

    qInfo() << "add INVALID depending data sets\n";
    TableDataInserter tdiChild2(s["c"]);
    tdiChild2.setValue("pid", 2); // should fail - no matching parent in table p
    QVERIFY( not isValidRowId( tdiChild2.InsertRecord()));
}

void test_db::test_deleteRecord_wDep()
{
    LOG_CALL;
    dbstructure s = dbstructure()
                        .appendTable(
                            dbtable("p")
                                .append(dbfield("id", QMetaType::Int).setAutoInc ())
                                .append(dbfield("name")));
    dbtable c("c");
    c.append(dbfield("id", QMetaType::Int).setAutoInc ());
    c.append(dbfield("pid", QMetaType::Int));
    c.append(dbForeignKey(c["pid"], s["p"]["id"], ODOU_Action::CASCADE));
    s.appendTable( c);

    QVERIFY2(s.createDb(), "Database was not created");

    TableDataInserter tdi(s["p"]);
    tdi.setValue("name", "Holger");
    QVERIFY( isValidRowId(tdi.InsertRecord()));
    QVERIFY(rowCount("p") == 1);

    qInfo() << "add depending data sets\n";
    TableDataInserter tdiChild1(s["c"]);
    tdiChild1.setValue("pid", QVariant(1)); // should work
    QVERIFY( isValidRowId( tdiChild1.InsertRecord()));
    TableDataInserter tdiChild2(s["c"]);
    tdiChild2.setValue("pid", QVariant(1)); // second child to matching parent in table p
    QVERIFY( isValidRowId( tdiChild2.InsertRecord()));
    QVERIFY(rowCount("p") == 1);
    QVERIFY(rowCount("c") == 2);

    qInfo() << "removing connected datasets\n";
    QSqlQuery deleteQ;
    deleteQ.exec("DELETE FROM p WHERE id = 1");
    QVERIFY(rowCount("p") == 0);
    QVERIFY(rowCount("c") == 0);
}

void test_db::newDbIsValid()
{
    QVERIFY(fill_DkDbDefaultContent());
}

