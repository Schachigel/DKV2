
//#include <QFile>
#include <QSqlDatabase>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <QString>
#include <QtTest>

#include "test_dkdbhelper.h"

void test_dkdbhelper::init()
{
    if (QFile::exists(filename))
        QFile::remove(filename);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", testCon);
    db.setDatabaseName(filename);
    QVERIFY(db.open());
    QSqlQuery enableRefInt(db);
    QVERIFY2(enableRefInt.exec("PRAGMA foreign_keys = ON"),
             enableRefInt.lastError().text().toLocal8Bit().data());
}
void test_dkdbhelper::cleanup()
{
    QSqlDatabase::database().removeDatabase(testCon);
    QSqlDatabase::database().close();
    if (QFile::exists(filename))
        QFile::remove(filename);
}

void test_dkdbhelper::test_empty()
{
    QVERIFY(false);
}

void test_dkdbhelper::test_empty2()
{
    QVERIFY(true);
}
