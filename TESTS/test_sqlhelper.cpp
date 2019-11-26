#include "test_sqlhelper.h"

#include "../dkv2/sqlhelper.h"
#include "../dkv2/helper.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QtTest>

test_sqlhelper::test_sqlhelper(QObject *parent) : QObject(parent)
{

}

void test_sqlhelper::initTestCase()
{
    LOG_ENTRY_and_EXIT;
    QDir().mkdir(QString("..\\data"));
    if (QFile::exists(filename))
        QFile::remove(filename);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", testCon);
    db.setDatabaseName(filename);
    QVERIFY(db.open());
    QSqlQuery enableRefInt(db);
    QVERIFY2(enableRefInt.exec("PRAGMA foreign_keys = ON"),
             enableRefInt.lastError().text().toLocal8Bit().data());
    QVERIFY2( (QFile::exists(filename) == true), "create database failed." );
    bool b = false;
    QSqlQuery query(db);
    b = query.exec("CREATE TABLE testSqlVal "
               "(id integer primary key, "
               "valBool INTEGER, "
               "valDouble REAL, "
               "valString STRING, "
               "valDate STRING)");
    QVERIFY( b ); // QCOMPARE( query.lastError().text(), QString(""));
    b = query.exec("INSERT INTO testSqlVal VALUES "
               "(1, "
               "1, "
               "1.1, "
               "'teststring', "
               "'2019-01-01 00:00:00.000')");
    QVERIFY( b ); // QCOMPARE( query.lastError().text(), QString(""));
    sqlValQuery = QSqlQuery(db);
    b = sqlValQuery.exec("SELECT * FROM testSqlVal");
    QVERIFY( b ); // QCOMPARE( sqlValQuery.lastError().text(), QString(""));
    b = sqlValQuery.first();
    QVERIFY( b ); // QCOMPARE( sqlValQuery.lastError().text(), QString(""));
    int id = sqlVal<int>(sqlValQuery, "id");
    QVERIFY( (id==1) );
}

void test_sqlhelper::cleanupTestCase()
{
    LOG_ENTRY_and_EXIT;
    QSqlDatabase::database().removeDatabase(testCon);
    QSqlDatabase::database().close();
    if (QFile::exists(filename))
        QFile::remove(filename);
    QDir().rmdir("..\\data");
    QVERIFY2( (QFile::exists(filename) == false), "destroy database failed." );
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

