#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QtTest>

#include "../DKV2/sqlhelper.h"
#include "../DKV2/helper.h"
#include "testhelper.h"

#include "test_sqlhelper.h"

test_sqlhelper::test_sqlhelper(QObject *parent) : QObject(parent)
{
}

void test_sqlhelper::initTestCase()
{
    LOG_CALL;
    initTestDb();

    QSqlQuery query;
    QVERIFY( query.exec("CREATE TABLE testSqlVal "
               "(id integer primary key, "
               "valBool INTEGER, "
               "valDouble REAL, "
               "valString STRING, "
               "valDate STRING)"));
    QVERIFY( query.exec("INSERT INTO testSqlVal VALUES "
               "(1, "
               "1, "
               "1.1, "
               "'teststring', "
               "'2019-01-01 00:00:00.000')"));
}

void test_sqlhelper::cleanupTestCase()
{
    LOG_CALL;
    cleanupTestDb();
}

void test_sqlhelper::test_getFields()
{
    QVector<QString> fields = getFieldsFromTablename("testSqlVal");
    QVERIFY2(fields.count() == 5, "failed to getFields from test database");
    QVERIFY2(fields.indexOf("id") != -1, "failed to getField 'id' from test database");
    fields.clear();
    fields = getFieldsFromTablename("nonExistingTable");
    QVERIFY2( fields.count() == 0, "failed to getFields from empty table");
}

void test_sqlhelper::test_tableExists()
{
    QVERIFY2(tableExists("testSqlVal"), "test of tableExists faild on existing table");
    QVERIFY2(!tableExists("notExistingTable"), "test of tableExists faild on NOT existing table");
}

void test_sqlhelper::test_sqlValInt()
{
    QSqlQuery sqlValQuery("SELECT * FROM testSqlVal");
    QVERIFY( sqlValQuery.first());
    QVERIFY( 1 == sqlVal<int>(sqlValQuery, "id"));
}

void test_sqlhelper::test_sqlValBool()
{
    QSqlQuery sqlValQuery("SELECT * FROM testSqlVal");
    QVERIFY( sqlValQuery.first());
    QVERIFY(sqlVal<bool>(sqlValQuery, "valBool"));
};

void test_sqlhelper::test_sqlValDouble()
{
    QSqlQuery sqlValQuery("SELECT * FROM testSqlVal");
    QVERIFY( sqlValQuery.first());
    QCOMPARE( sqlVal<double>(sqlValQuery, "valDouble"), 1.1 );
};

void test_sqlhelper::test_sqlValString()
{
    QSqlQuery sqlValQuery("SELECT * FROM testSqlVal");
    QVERIFY( sqlValQuery.first());
    QCOMPARE( sqlVal<QString>(sqlValQuery, "valString"), QString("teststring") );
};

void test_sqlhelper::test_sqlValDate()
{
    QSqlQuery sqlValQuery("SELECT * FROM testSqlVal");
    QVERIFY( sqlValQuery.first());
    QCOMPARE( sqlVal<QDate>(sqlValQuery, "valDate"), QDate(2019, 1, 1) );
};

