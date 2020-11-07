#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QtTest>

#include "../DKV2/helpersql.h"
#include "../DKV2/helper.h"
#include "testhelper.h"

#include "test_sqlhelper.h"

test_sqlhelper::test_sqlhelper(QObject *parent) : QObject(parent)
{}

void test_sqlhelper::initTestCase()
{   LOG_CALL;
    initTestDb();
}

void test_sqlhelper::cleanupTestCase()
{   LOG_CALL;
    cleanupTestDb();
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
    QSqlQuery("DROP TABLE testRowCount01");
}

void test_sqlhelper::test_tableExists()
{
    QVERIFY( QSqlQuery().exec("CREATE TABLE testSqlVal "
               "(id integer primary key, "
               "valBool INTEGER, valDouble REAL, "
               "valString STRING, valDate STRING)"));
    QVERIFY( QSqlQuery().exec("INSERT INTO testSqlVal VALUES "
               "(1, 1, 1.1, 'teststring', '2019-01-01 00:00:00.000')"));
    QVERIFY2(tableExists("testSqlVal"), "test of tableExists faild on existing table");
    QVERIFY2(!tableExists("notExistingTable"), "test of tableExists faild on NOT existing table");
    QSqlQuery("DROP TABLE testSqlVal");
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

void test_sqlhelper::test_eSingleValueSql_not_PreservsValue()
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
    // note: types are assumed. the actual variant type could be different
    QCOMPARE(executeSingleValueSql("id", "testtable", "id=1").toLongLong(), 1);
    QCOMPARE(executeSingleValueSql("aDate", "testtable", "id=1").toDate(), date);
    QCOMPARE(executeSingleValueSql("anInt", "testtable", "id=1").toInt(), i);
    QCOMPARE(executeSingleValueSql("aString", "testtable", "id=1").toString(), string);
    QCOMPARE(executeSingleValueSql("aBool", "testtable", "id=1").toBool(), b);

    QVERIFY(QSqlQuery().exec("DROP TABLE " +tablename));
}
void test_sqlhelper::test_eSingleValueSqlPreservsValue()
{
    QString tablename("testTable");
    dbtable t(tablename);
    t.append(dbfield("id",      QVariant::LongLong, "PRIMARY KEY AUTOINCREMENT"));
    t.append(dbfield("aDate",   QVariant::Date));
    t.append(dbfield("anInt",   QVariant::Int));
    t.append(dbfield("aString", QVariant::String));
    t.append(dbfield("aBool",   QVariant::Bool));
    QVERIFY(ensureTable(t));

    QVariant string{"Hallo Welt!"};
    QVariant date{QDate (1965, 5, 6)};
    QVariant i {42};
    QVariant b (true);
    QString createSql{ QString("INSERT INTO " +tablename+  " VALUES( NULL, "
                 + dbInsertableString(date) +", "
                 + dbInsertableString(i) +", "
                 + dbInsertableString(string) + ", "
                 + dbInsertableString(b) + ")")};

    QVERIFY(QSqlQuery().exec(createSql));
    // note: types could be enforced, they are known to
    QCOMPARE(executeSingleValueSql(t["id"], "id=1"), 1);
    QCOMPARE(executeSingleValueSql(t["aDate"], "id=1"), date);
    QCOMPARE(executeSingleValueSql(t["anInt"], "id=1"), i);
    QCOMPARE(executeSingleValueSql(t["aString"], "id=1"), string);
    QCOMPARE(executeSingleValueSql(t["aBool"], "id=1"), b);

    QVERIFY(QSqlQuery().exec("DROP TABLE " +tablename));
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
    QString sql = selectQueryFromFields(simple.Fields(), QVector<dbForeignKey>(), "id=2");
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
    referencing.append(dbfield("refId", QVariant::LongLong));
    referencing.append(dbForeignKey(referencing["refId"], referenced["id"]));
    referencing.append(dbfield("other"));
    ensureTable(referencing);
    inserter.exec("INSERT INTO " + tname2 + " VALUES( 1, 'Hut')");
    qDebug() << inserter.lastError() << Qt::endl << inserter.lastQuery();
    inserter.exec("INSERT INTO " + tname2 + " VALUES( 1, 'Schuh')");
    inserter.exec("INSERT INTO " + tname2 + " VALUES( 2, 'Hemd')");
    // test
    QVector<dbfield> selected{referenced.Fields()};
    selected.append(referencing["refId"]);
    selected.append(referencing["other"]);
    QString sql = selectQueryFromFields(selected, referencing.ForeignKeys());
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
    QSqlQuery insertQ;
    insertQ.exec("INSERT INTO " + tname + " VALUES( NULL, 'Hugo')");
    insertQ.exec("INSERT INTO " + tname + " VALUES( NULL, 'Franz')");

    QString tname2{"t2"};
    dbtable referencing(tname2);
    referencing.append(dbfield("refId", QVariant::LongLong));
    referencing.append(dbForeignKey(referencing["refId"], referenced["id"]));
    referencing.append(dbfield("other"));
    ensureTable(referencing);
    insertQ.exec("INSERT INTO " + tname2 + " VALUES( 1, 'Hut')");
    qDebug() << insertQ.lastError() << Qt::endl << insertQ.lastQuery();
    insertQ.exec("INSERT INTO " + tname2 + " VALUES( 1, 'Schuh')");
    insertQ.exec("INSERT INTO " + tname2 + " VALUES( 2, 'Hemd')");
    // test
    QVector<dbfield> selected{referenced.Fields()};
    selected.append(referencing["refId"]);
    selected.append(referencing["other"]);
    QString sql = selectQueryFromFields(selected, referencing.ForeignKeys(), "t1.id=2");
    QSqlQuery probe;
    QVERIFY(probe.exec(sql));
    probe.first();
    QCOMPARE( probe.record().value("t1.col").toString(), "Franz");
    QCOMPARE( probe.record().value("t2.other").toString(), "Hemd");
    QSqlQuery("DROP TABLE " + tname);
    QSqlQuery("DROP TABLE " + tname2);
}

void test_sqlhelper::test_executeSingleColumnSql()
{
    QString tablename("test_executeSingleColumnSql");
    dbtable t(tablename);
    t.append(dbfield("col_index", QVariant::LongLong, "PRIMARY KEY").setAutoInc());
    t.append(dbfield("col_ll", QVariant::LongLong));
    t.append(dbfield("col_I", QVariant::Int));
    t.append(dbfield("col_S", QVariant::String));
    t.append(dbfield("col_D", QVariant::Date));
    t.append(dbfield("col_B", QVariant::Bool));
    dbstructure db; db.appendTable(t); db.createDb();

    QVERIFY( executeSingleColumnSql(t["col_ll"]).isEmpty());
    QSqlQuery insertQ;
    insertQ.prepare("INSERT INTO " + tablename + " VALUES (NULL, ?, ?, ?, ?, ?)");
    for ( int i = 0; i < 16; i++) {
        insertQ.addBindValue(i);
        insertQ.addBindValue(-i);
        insertQ.addBindValue(QString::number(i));
        insertQ.addBindValue(QDate::currentDate());
        insertQ.addBindValue(i%2==0);
        insertQ.exec();
    }
    QVector<QVariant> result = executeSingleColumnSql(t["col_ll"]);
    QCOMPARE( result.count(), 16);
    for ( auto& v : result) {
        QVERIFY(v.type() == t["col_ll"].type());
    }
    result = executeSingleColumnSql(t["col_I"]);
    QCOMPARE( result.count(), 16);
    for ( auto& v : result) {
        QVERIFY(v.type() == t["col_I"].type());
    }
    result = executeSingleColumnSql(t["col_S"]);
    QCOMPARE( result.count(), 16);
    for ( auto& v : result) {
        QVERIFY(v.type() == t["col_S"].type());
    }
    result = executeSingleColumnSql(t["col_D"]);
    QCOMPARE( result.count(), 16);
    for ( auto& v : result) {
        QVERIFY(v.type() == t["col_D"].type());
    }
    result = executeSingleColumnSql(t["col_B"]);
    QCOMPARE( result.count(), 16);
    for ( auto& v : result) {
        QVERIFY(v.type() == t["col_B"].type());
    }
}

void test_sqlhelper::test_variantTypeConservation()
{
/* This test is to document, how Varaint and Database types should
 * work together in this application.
 * The dbtable / dbfield classes care about the table creation using
 * five QVariant types;
 * the execuetxxxSqp functions take care of data retrieval. they take
 * the expected datatypes from dbfield and convert back from what the
 * database has done to the data.
*/
    QString tablename("testTable_variantVsDbTypes");
    dbtable t(tablename);
    t.append(dbfield("col_index", QVariant::LongLong, "PRIMARY KEY").setAutoInc());
    t.append(dbfield("col_ll", QVariant::LongLong));
    t.append(dbfield("col_I", QVariant::Int));
    t.append(dbfield("col_S", QVariant::String));
    t.append(dbfield("col_D", QVariant::Date));
    t.append(dbfield("col_B", QVariant::Bool));
    dbstructure db; db.appendTable(t); db.createDb();
/* from CREATE TABLE sql:
   col_index INTEGER PRIMARY KEY AUTOINCREMENT
   col_ll INTEGER,
   col_I  INTEGER,
   col_S  TEXT,
   col_D  TEXT,
   col_B  INTEGER
*/
    // input data
    QVariant ll = QVariant(qlonglong(42));
    QVariant i  = QVariant(13);
    QVariant s  = QVariant(QString("Hallo Welt!"));
    QVariant d  = QVariant(QDate(1965, 5, 6));
    QVariant b  = QVariant(true);

    QString sql= "INSERT INTO " + tablename + " VALUES (NULL, ";
    sql += dbInsertableString(ll) +", ";
    sql += dbInsertableString(i) + ", ";
    sql += dbInsertableString(s) +", ";
    sql += dbInsertableString(d) +", ";
    sql += dbInsertableString(b) +")";

    QSqlQuery q;
    if( ! q.exec(sql)) {
        qDebug() << q.lastError() << Qt::endl << q.lastQuery();
        QFAIL("query execution failed");
    }
//    this is how the record looks:
//    0: QSqlField("col_index", int, ..., generated: yes, typeID: 1, autoValue: false, ro: false) "1"
//    1: QSqlField("col_ll",    int, ..., generated: yes, typeID: 1, autoValue: false, ro: false) "42"
//    2: QSqlField("col_I",     int, ..., generated: yes, typeID: 1, autoValue: false, ro: false) "13"
//    3: QSqlField("col_S", QString, ..., generated: yes, typeID: 3, autoValue: false, ro: false) "Hallo Welt!"
//    4: QSqlField("col_D", QString, ..., generated: yes, typeID: 3, autoValue: false, ro: false) "1965-05-06"
//    5: QSqlField("col_B",     int, ..., generated: yes, typeID: 1, autoValue: false, ro: false) "1"

    // TEST: will this record contain the right data AND right types?
    QSqlRecord record = executeSingleRecordSql(t.Fields());
    qDebug() << record;

    // data types are back to what we put in
    QCOMPARE( ll, record.value("col_ll"));
    QCOMPARE( ll.type(), record.value("col_ll").type());
    QCOMPARE(  i, record.value("col_I"));
    QCOMPARE(  i.type(), record.value("col_I").type());
    QCOMPARE(  s, record.value("col_S"));
    QCOMPARE(  s.type(), record.value("col_S").type());
    QCOMPARE(  d, record.value("col_D"));
    QCOMPARE(  d.type(), record.value("col_D").type());
    QCOMPARE(  b, record.value("col_B"));
    QCOMPARE(  b.type(), record.value("col_B").type());
}

void test_sqlhelper::test_getHighestRowId()
{
    QString tablename("test_getHighestRowId");
    dbtable t(tablename);
    t.append(dbfield("col_index", QVariant::LongLong, "PRIMARY KEY").setAutoInc());
    t.append(dbfield("col_S", QVariant::String));
    QVERIFY(ensureTable(t));
    QSqlQuery q; q.prepare("INSERT INTO "+ tablename + " (col_S) VALUES (?)");
    int maxrow = 100;
    for( int i = 0; i<maxrow; i++) {
        q.addBindValue(QString::number(i));
        if( !q.exec()) qDebug() << q.lastError() << Qt::endl << q.lastQuery();
    }
    QCOMPARE( getHighestRowId(tablename), maxrow);
    q.prepare("DELETE FROM " + tablename + " WHERE col_S=?");
    for( int i = 0; i<maxrow; i+=2) {
        q.addBindValue(QString::number(i));
        if( !q.exec()) qDebug() << q.lastError() << Qt::endl << q.lastQuery();
    }
    QCOMPARE( getHighestRowId(tablename), maxrow);
    QCOMPARE(rowCount(tablename), maxrow/2);
}
