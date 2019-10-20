
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QtTest>

#include "../DKV2/helper.h"
#include "../DKV2/dkdbhelper.h"

#include "test_dkdbhelper.h"

void test_dkdbhelper::init()
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
void test_dkdbhelper::cleanup()
{LOG_ENTRY_and_EXIT;
    QSqlDatabase::database().removeDatabase(testCon);
    QSqlDatabase::database().close();
    if (QFile::exists(filename))
        QFile::remove(filename);
}

void test_dkdbhelper::test_querySingleValueInvalidQuery()
{
    QString sql ("SELECT NOTEXISTINGFIELD FROM NOTEXISTINGTABLE WHERE NOTEXISTINGFIELD='0'");
    QVERIFY2(QVariant::Invalid == ExecuteSingleValueSql(sql).type(),
             "Invalid single value sql has poditiv result");
}

void test_dkdbhelper::test_querySingleValue()
{
    dbstructure s = dbstructure()
        .appendTable(dbtable("t")
            .append(dbfield("id", QVariant::Int))
            .append(dbfield("f")));
    s.createDb(QSqlDatabase::database(testCon));
    TableDataInserter tdi(s["t"]);
    tdi.setValue("id", 1);
    tdi.setValue("f", "Hallo");
    tdi.InsertData(QSqlDatabase::database(testCon));
    QVariant hallo = ExecuteSingleValueSql("SELECT [f] FROM [t] WHERE id=1", testCon);
    QVERIFY2(hallo.toString() == "Hallo", "ExecuteSingleValueSql failed");
}


