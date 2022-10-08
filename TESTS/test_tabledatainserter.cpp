#include <QTest>

#include "../DKV2/helpersql.h"
#include "../DKV2/dbstructure.h"
#include "../DKV2/dbfield.h"
#include "../DKV2/dbtable.h"
#include "../DKV2/tabledatainserter.h"

#include "testhelper.h"
#include "test_tabledatainserter.h"

void test_tableDataInserter::initTestCase()
{
    init_DKDBStruct();
}

void test_tableDataInserter::init()
{   LOG_CALL;
    initTestDb();
}

void test_tableDataInserter::cleanup()
{   LOG_CALL;
    cleanupTestDb();

}

namespace {
const QString tname    {qsl("tname")};
const QString fnId     {qsl("id")};
const QString fnInt    {qsl("type_integer")};
const QString fnString {qsl("type_string")};
const QString fnBool   {qsl("type_bool")};
const QString fnDate   {qsl("type_date")};

const QVariant val_int    (42);
const QVariant val_string ("Hallo Welt");
const QVariant val_bool   (true);
const QVariant val_date   (QDate(1995, 5, 6));

dbtable defineTestTable()
{
    dbtable t(tname);
    t.append(dbfield(fnId,     QVariant::LongLong).setPrimaryKey().setNotNull().setAutoInc());
    t.append(dbfield(fnInt,    QVariant::Int).setNotNull());
    t.append(dbfield(fnString, QVariant::String).setNotNull());
    t.append(dbfield(fnBool,   QVariant::Bool).setNotNull());
    t.append(dbfield(fnDate,   QVariant::Date).setNotNull());
    return t;
}
}

void test_tableDataInserter::test_insert_and_retreive()
{

    dbtable t =defineTestTable();
    QVERIFY(t.create());
    {/// CUT ->
        TableDataInserter tdi(t);
        tdi.setValue(fnInt, val_int);
        tdi.setValue(fnString, val_string);
        tdi.setValue(fnBool, val_bool);
        tdi.setValue(fnDate, val_date);
        QVERIFY(-1 not_eq tdi.InsertRecord());
    }/// <- CUT
    QSqlRecord r = executeSingleRecordSql( t.Fields());
    qDebug() << r;
    QVERIFY( r.value("id").type() == QVariant::LongLong
             or r.value("id").type() == QVariant::Int);
    QCOMPARE( r.value("id"), 1);
    QVariant v = r.value(fnInt);
    qDebug() << v << " (" << v.type() << ")";
    QCOMPARE( v.type(), QVariant::Int);
    QCOMPARE( v, val_int);
    QCOMPARE( r.value(fnBool).type(), QVariant::Bool);
    QCOMPARE( r.value(fnBool), val_bool);
    QCOMPARE( r.value(fnString).type(), QVariant::String);
    QCOMPARE( r.value(fnString), val_string);
    QCOMPARE( r.value(fnDate).type(), QVariant::Date);
    QCOMPARE( r.value(fnDate), val_date);

    // add more data
    {
        TableDataInserter tdi_more(t);
        tdi_more.setValue (fnInt, 24);
        tdi_more.setValue (fnString, "hi!");
        tdi_more.setValue (fnBool, false);
        tdi_more.setValue (fnDate, QDate(1965, 5, 7));
        QVERIFY( -1 not_eq tdi_more.InsertRecord ());
    }
    {   /// CUT ->
        TableDataInserter tdi_update(t);
        QVERIFY( not tdi_update.updateValue(qsl("notexisting"), QVariant(4), 2));
        QVERIFY( tdi_update.updateValue(fnString, QVariant("ho!"), 2));
        /// <- CUT
        QSqlRecord updatedRec = executeSingleRecordSql( t.Fields(), "id=2");
        QCOMPARE( updatedRec.value(fnInt), 24);
        QCOMPARE( updatedRec.value(fnString), QVariant("ho!"));
    }

    {
        TableDataInserter tdi_update(t);
        tdi_update.setValue (fnInt, 24);
        tdi_update.setValue (fnString, "hi!");
        tdi_update.setValue (fnBool, false);
        tdi_update.setValue (fnDate, QDate(1965, 5, 7));
        QCOMPARE(3, tdi_update.InsertRecord ());
        /// CUT ->
        tdi_update.setValue("id", 3);
        tdi_update.setValue(fnInt, 13);
        tdi_update.setValue(fnBool, true);
        tdi_update.setValue(fnDate, QDate(1982, 5, 6));
        QCOMPARE(3, tdi_update.UpdateRecord ());

        QSqlRecord rec_updated =executeSingleRecordSql(t.Fields (), "id=3");
        QCOMPARE( rec_updated.value(fnInt), 13);
        QCOMPARE( rec_updated.value(fnString), "hi!");
        QCOMPARE( rec_updated.value(fnBool), true);
        QCOMPARE( rec_updated.value(fnDate), QDate(1982, 5, 6));
        /// <- CUT
    }
}
