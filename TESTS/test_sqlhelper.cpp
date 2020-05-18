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

    bool b = false;
    QSqlQuery query;
    b = query.exec("CREATE TABLE testSqlVal "
               "(id integer primary key, "
               "valBool INTEGER, "
               "valDouble REAL, "
               "valString STRING, "
               "valDate STRING)");
    QVERIFY( b );
    b = query.exec("INSERT INTO testSqlVal VALUES "
               "(1, "
               "1, "
               "1.1, "
               "'teststring', "
               "'2019-01-01 00:00:00.000')");
    QVERIFY( b );
    sqlValQuery  = QSqlQuery();
    b = sqlValQuery.exec("SELECT * FROM testSqlVal");
    QVERIFY( b );
    b = sqlValQuery.first();
    QVERIFY( b );
    int id = sqlVal<int>(sqlValQuery, "id");
    QVERIFY( (id==1) );
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

void test_sqlhelper::test_sqlValBool()
{
    bool valBool = sqlVal<bool>(sqlValQuery, "valBool");
    QCOMPARE( valBool, true );
};

void test_sqlhelper::test_sqlValDouble()
{
    double valDouble = sqlVal<double>(sqlValQuery, "valDouble");
    QCOMPARE( valDouble, 1.1 );
};

void test_sqlhelper::test_sqlValString()
{
    QString valString = sqlVal<QString>(sqlValQuery, "valString");
    QCOMPARE( valString, QString("teststring") );

};

void test_sqlhelper::test_sqlValDate()
{
    QDate valDate = sqlVal<QDate>(sqlValQuery, "valDate");
    QCOMPARE( valDate, QDate(2019, 1, 1) );
};

