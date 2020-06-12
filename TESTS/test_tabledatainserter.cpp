#include <QTest>

#include "../DKV2/helpersql.h"
#include "../DKV2/dbstructure.h"
#include "../DKV2/dbfield.h"
#include "../DKV2/dbtable.h"
#include "../DKV2/tabledatainserter.h"

#include "testhelper.h"
#include "test_tabledatainserter.h"

test_tableDataInserter::test_tableDataInserter(QObject *parent) :QObject(parent)
{}

void test_tableDataInserter::initTestCase()
{   LOG_CALL;
    initTestDb();
}

void test_tableDataInserter::cleanupTestCase()
{   LOG_CALL;
    cleanupTestDb();

}

void test_tableDataInserter::test_insert_and_retreive()
{
    dbtable t("tname");
    t.append(dbfield("id",      QVariant::LongLong).setPrimaryKey().setNotNull().setAutoInc());
    t.append(dbfield("type_integer", QVariant::Int).setNotNull());
    t.append(dbfield("type_string",  QVariant::String).setNotNull());
    t.append(dbfield("type_bool",    QVariant::Bool).setNotNull());
    t.append(dbfield("type_date",    QVariant::Date).setNotNull());
    QVERIFY(t.create());
    TableDataInserter tdi(t);

    QVariant type_integer(42);
    QVariant type_string("Hallo Welt");
    QVariant type_bool(true);
    QVariant type_date(QDate(1995, 5, 6));

    tdi.setValue("type_integer", type_integer);
    tdi.setValue("type_string", type_string);
    tdi.setValue("type_bool", type_bool);
    tdi.setValue("type_date", type_date);
    QVERIFY(tdi.InsertData());
    QSqlRecord r = executeSingleRecordSql( t.Fields());
    qDebug() << r;
    QVERIFY( r.value("id").type() == QVariant::LongLong
             || r.value("id").type() == QVariant::Int);
    QCOMPARE( r.value("id"), 1);

    QVariant v = r.value("type_integer");
    qDebug() << v << " (" << v.type() << ")";
    QCOMPARE( v.type(), QVariant::Int);
    QCOMPARE( r.value("type_integer"), type_integer);
    QCOMPARE( r.value("type_bool").type(), QVariant::Bool);
    QCOMPARE( r.value("type_bool"), type_bool);
    QCOMPARE( r.value("type_string").type(), QVariant::String);
    QCOMPARE( r.value("type_string"), type_string);
    QCOMPARE( r.value("type_date").type(), QVariant::Date);
    QCOMPARE( r.value("type_date"), type_date);
}
