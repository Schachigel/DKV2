#include "../DKV2/helper.h"
#include "../DKV2/dbtable.h"
#include "test_properties.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/dkdbhelper.h"


test_properties::test_properties(QObject *parent) : QObject(parent)
{
}

void test_properties::initTestCase()
{
    init_DKDBStruct();
    init_additionalTables();
}
void test_properties::init()
{LOG_CALL;
    if (QFile::exists(filename))
        QVERIFY(QFile::remove(filename));
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", testCon);
    db.setDatabaseName(filename);
    QVERIFY(db.open());
    QSqlQuery enableRefInt(db);
    QVERIFY2(enableRefInt.exec("PRAGMA foreign_keys = ON"),
             enableRefInt.lastError().text().toLocal8Bit().data());

    dbtable table(dkdbstructur["Meta"]);
    table.create(db);
}

void test_properties::cleanup()
{LOG_CALL;
    QSqlDatabase::database().removeDatabase(testCon);
    QSqlDatabase::database().close();
    if (QFile::exists(filename))
        QVERIFY(QFile::remove(filename));
}
void test_properties::test_setProperty_getProperty()
{
    setMetaInfo("Hallo", "Welt", testCon);
    QCOMPARE(getMetaInfo("Hallo", testCon), "Welt");
}
void test_properties::test_initProperty_getProperty()
{
    initMetaInfo("Hallo", "Welt", testCon);
    QCOMPARE(getMetaInfo("Hallo", testCon), "Welt");
}
void test_properties::test_set_init_getProperty()
{
    setMetaInfo("Hallo", "Weltall", testCon);
    initMetaInfo("Hallo", "Welt", testCon);
    QCOMPARE(getMetaInfo("Hallo", testCon), "Weltall");
}
void test_properties::test_get_uninit()
{
    QCOMPARE(getMetaInfo("Hallo"), "");
}
void test_properties::test_set_get_num()
{
    setNumMetaInfo("Pi", 3.1415, testCon);
    QCOMPARE(getNumMetaInfo("Pi", testCon)*10000, 31415);
}
void test_properties::test_init_get_num()
{
    initNumMetaInfo("twoPi", 6.243, testCon);
    QCOMPARE(getNumMetaInfo("twoPi", testCon), 6.243);
}
void test_properties::test_set_init_get_num()
{
    setNumMetaInfo("three", 3., testCon);
    initNumMetaInfo("three", 4, testCon);
    QCOMPARE( getNumMetaInfo("three", testCon), 3);
}
void test_properties::test_get_uninit_num()
{
    QCOMPARE(getNumMetaInfo("null",testCon), 0.);
}

