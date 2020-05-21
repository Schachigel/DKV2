#include <QVariant>
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
{   LOG_CALL;
    initTestDb();
    QVERIFY( QSqlQuery().exec("CREATE TABLE testSqlVal "
               "(id integer primary key, "
               "valBool INTEGER, "
               "valDouble REAL, "
               "valString STRING, "
               "valDate STRING)"));
    QVERIFY( QSqlQuery().exec("INSERT INTO testSqlVal VALUES "
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

void test_sqlhelper::test_tableExists()
{
    QVERIFY2(tableExists("testSqlVal"), "test of tableExists faild on existing table");
    QVERIFY2(!tableExists("notExistingTable"), "test of tableExists faild on NOT existing table");
}

void test_sqlhelper::test_rowCount()
{   LOG_CALL;
    QSqlQuery q;
    QVERIFY( q.exec("CREATE TABLE testRowCount01 "
               "(id integer PRIMARY KEY AUTOINCREMENT, s STRING)"));
    dbgTimer testrowcount ("trc");
    int maxRows = 10;
    for( int i=1; i<=maxRows; i++)
        QVERIFY( q.exec(QString() + "INSERT INTO testRowCount01 (s) VALUES ('Hallo Welt')"));
    for( int i=1; i<=maxRows; i += 2)
        QVERIFY( q.exec(QString() + "DELETE FROM testRowCount01 WHERE (id=" + QString::number(i) +")"));
    QCOMPARE(rowCount("testRowCount01"), maxRows/2);
    QVERIFY (q.exec("DROP TABLE testRowCount01"));
}

void test_sqlhelper::test_ensureTable_existingTableOK()
{
    QString tname{"ensureTable01"};
    QVERIFY( QSqlQuery().exec("CREATE TABLE " + tname
               + " (id integer PRIMARY KEY AUTOINCREMENT, s STRING, i INTEGER)"));
    dbtable t(tname);
    t.append(dbfield("id", QVariant::LongLong));
    t.append(dbfield("s"));
    t.append(dbfield("i", QVariant::Int));
    QVERIFY(ensureTable(t));
    QVERIFY(QSqlQuery().exec("DROP TABLE " + tname));
}
void test_sqlhelper::test_ensureTable_existingTable_tableSizeMismatch()
{
    QString tname{"ensureTable02"};
    QSqlQuery query;
    QVERIFY( query.exec("CREATE TABLE " + tname +
               " (id integer PRIMARY KEY AUTOINCREMENT, s STRING, i INTEGER, j INTEGER)"));
    dbtable t(tname);
    t.append(dbfield("id", QVariant::LongLong));
    t.append(dbfield("s"));
    t.append(dbfield("i", QVariant::Int));
    QVERIFY( ! ensureTable(t));
    QVERIFY(query.exec("DROP TABLE " + tname));
}
void test_sqlhelper::test_ensureTable_existingtable_fieldTypeMismatch()
{
    QString tname{"ensureTable03"};
    QSqlQuery query;
    QVERIFY( query.exec("CREATE TABLE " + tname
               + " (id integer PRIMARY KEY AUTOINCREMENT, s STRING, i INTEGER)"));
    dbtable t(tname);
    t.append(dbfield("id", QVariant::String));
    t.append(dbfield("s"));
    t.append(dbfield("i", QVariant::Int));
    QVERIFY( ! ensureTable(t));
    QVERIFY(query.exec("DROP TABLE " + tname));
}
void test_sqlhelper::test_ensureTable_nonexistingTable()
{
    dbtable t("tableThatDoesNotExist");
    t.append(dbfield("id"));
    QVERIFY(ensureTable(t));
    QSqlQuery("DROP TABLE tableThatDoesNotExist");
}

void test_sqlhelper::test_selectQueryFromFields_noWhere()
{
    // preparation
    QString tname{"simpletable"};
    dbtable simple(tname);
    simple.append(dbfield("id", QVariant::LongLong, "PRIMARY KEY AUTOINCREMENT"));
    simple.append(dbfield("col"));
    ensureTable(simple);
    QSqlQuery q;
    q.exec("INSERT INTO " + tname + " VALUES( NULL, 'Hallo ')");
    q.exec("INSERT INTO " + tname + " VALUES( NULL, 'Welt!')");
    // test
    QString sql = selectQueryFromFields(simple.Fields());
    QSqlQuery probe;
    QVERIFY( probe.exec(sql));
    probe.first();
    QCOMPARE( probe.record().value("col").toString(), "Hallo ");
    probe.next();
    QCOMPARE( probe.record().value("col").toString(), "Welt!");
}

void test_sqlhelper::test_selectQueryFromFields_withWhere()
{
    // preparation
    QString tname{"simpletable"};
    dbtable simple(tname);
    simple.append(dbfield("id", QVariant::LongLong, "PRIMARY KEY AUTOINCREMENT"));
    simple.append(dbfield("col"));
    ensureTable(simple);
    QSqlQuery q;
    q.exec("INSERT INTO " + tname + " VALUES( NULL, 'Hallo ')");
    q.exec("INSERT INTO " + tname + " VALUES( NULL, 'Welt!')");
    // test
    QString sql = selectQueryFromFields(simple.Fields(), "id=2");
    QSqlQuery probe;
    QVERIFY( probe.exec(sql));
    probe.first();
    QCOMPARE( probe.record().value("col").toString(), "Welt!");
}

void test_sqlhelper::test_selectQueryFromFields_wReference()
{
    // preparation
    QString tname{"t1"};
    dbtable referenced(tname);
    referenced.append(dbfield("id", QVariant::LongLong, "PRIMARY KEY AUTOINCREMENT"));
    referenced.append(dbfield("col"));
    ensureTable(referenced);
    QSqlQuery inserter;
    inserter.exec("INSERT INTO " + tname + " VALUES( NULL, 'Hugo')");
    inserter.exec("INSERT INTO " + tname + " VALUES( NULL, 'Franz')");

    QString tname2{"t2"};
    dbtable referencing(tname2);
    referencing.append(dbfield("refId", QVariant::LongLong, "", referenced["id"], dbfield::refIntOption::onDeleteCascade));
    referencing.append(dbfield("other"));
    ensureTable(referencing);
    inserter.exec("INSERT INTO " + tname2 + " VALUES( 1, 'Hut')");
    qDebug() << inserter.lastError() << endl << inserter.lastQuery();
    inserter.exec("INSERT INTO " + tname2 + " VALUES( 1, 'Schuh')");
    inserter.exec("INSERT INTO " + tname2 + " VALUES( 2, 'Hemd')");
    // test
    QVector<dbfield> selected{referenced.Fields()};
    selected.append(referencing["refId"]);
    selected.append(referencing["other"]);
    QString sql = selectQueryFromFields(selected);
    QSqlQuery probe;
    QVERIFY(probe.exec(sql));
    probe.first();
    qDebug() << probe.record();
    QCOMPARE( probe.record().value("t1.col").toString(), "Hugo");
    QCOMPARE( probe.record().value("t2.other").toString(), "Hut");
    probe.next();
    QCOMPARE( probe.record().value("t1.col").toString(), "Hugo");
    QCOMPARE( probe.record().value("t2.other").toString(), "Schuh");
    probe.next();
    QCOMPARE( probe.record().value("t1.col").toString(), "Franz");
    QCOMPARE( probe.record().value("t2.other").toString(), "Hemd");
}

void test_sqlhelper::test_selectQueryFromFields_wRefwWhere()
{
    // preparation
    QString tname{"t1"};
    dbtable referenced(tname);
    referenced.append(dbfield("id", QVariant::LongLong, "PRIMARY KEY AUTOINCREMENT"));
    referenced.append(dbfield("col"));
    ensureTable(referenced);
    QSqlQuery inserter;
    inserter.exec("INSERT INTO " + tname + " VALUES( NULL, 'Hugo')");
    inserter.exec("INSERT INTO " + tname + " VALUES( NULL, 'Franz')");

    QString tname2{"t2"};
    dbtable referencing(tname2);
    referencing.append(dbfield("refId", QVariant::LongLong, "", referenced["id"], dbfield::refIntOption::onDeleteCascade));
    referencing.append(dbfield("other"));
    ensureTable(referencing);
    inserter.exec("INSERT INTO " + tname2 + " VALUES( 1, 'Hut')");
    qDebug() << inserter.lastError() << endl << inserter.lastQuery();
    inserter.exec("INSERT INTO " + tname2 + " VALUES( 1, 'Schuh')");
    inserter.exec("INSERT INTO " + tname2 + " VALUES( 2, 'Hemd')");
    // test
    QVector<dbfield> selected{referenced.Fields()};
    selected.append(referencing["refId"]);
    selected.append(referencing["other"]);
    QString sql = selectQueryFromFields(selected, "t1.id=2");
    QSqlQuery probe;
    QVERIFY(probe.exec(sql));
    probe.first();
    QCOMPARE( probe.record().value("t1.col").toString(), "Franz");
    QCOMPARE( probe.record().value("t2.other").toString(), "Hemd");
}

void test_sqlhelper::test_eSingleValueSql_OK()
{
    QString tablename("testTable");
    dbtable t(tablename);
    t.append(dbfield("id",      QVariant::LongLong, "PRIMARY KEY AUTOINCREMENT"));
    t.append(dbfield("aDate",   QVariant::Date));
    t.append(dbfield("anInt",   QVariant::Int));
    t.append(dbfield("aString", QVariant::String));
    t.append(dbfield("aBool",   QVariant::Bool));
    QVERIFY(ensureTable(t));

    QString string{"Hallo Welt!"};
    QDate date{1965, 5, 6};
    int i {42};
    bool b = true;
    QString createSql{ QString("INSERT INTO " +tablename+  " VALUES( "
                 + "NULL, "
                 + "'" + date.toString(Qt::ISODate) + "', "
                 + QString::number(i) + ", "
                 +  "'" + string + "', "
                 +  (b ? "1" : "0")
                 + ")")};

    QVERIFY(QSqlQuery().exec(createSql));

    QCOMPARE(executeSingleValueSql("id", "testtable", "id=1").toLongLong(), 1);
    QCOMPARE(executeSingleValueSql("aDate", "testtable", "id=1").toDate(), date);
    QCOMPARE(executeSingleValueSql("anInt", "testtable", "id=1").toInt(), i);
    QCOMPARE(executeSingleValueSql("aString", "testtable", "id=1").toString(), string);
    QCOMPARE(executeSingleValueSql("aBool", "testtable", "id=1").toBool(), b);

    QVERIFY(QSqlQuery().exec("DROP TABLE " +tablename));
}
