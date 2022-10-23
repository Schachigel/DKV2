
#include "../DKV2/helper.h"
#include "../DKV2/helperfin.h"
#include "../DKV2/dbstructure.h"
#include "../DKV2/helpersql.h"
#include "testhelper.h"

#include "test_sqlhelper.h"

void test_sqlhelper::init()
{   LOG_CALL;
    openDbConnection_InMemory();
}

void test_sqlhelper::cleanup()
{   LOG_CALL;
    closeDbConnection();
}

void test_sqlhelper::test_rowCount()
{   LOG_CALL;
    QString table {qsl("tablename")};
    QString field {qsl("fieldname")};

    QCOMPARE( rowCount(table), -1); // non existing table

    QSqlQuery q;
    QVERIFY( q.exec(qsl("CREATE TABLE [%1]"
               "([%2] TEXT)").arg(table, field)));
    QCOMPARE( rowCount(table), 0); // empty table

    QString addOneRow {qsl("INSERT INTO [%1] ([%2]) VALUES (%3)").arg(table)};
    qInfo() << addOneRow.arg(field, qsl("'first line'"));
    q.exec (addOneRow.arg(field, qsl("'first line'")));
    QCOMPARE( rowCount(table), 1); // first entry

//    dbgTimer testrowcount ("trc");
    int maxRows = 10;
    for( int i=1; i<=maxRows; i++)
        QVERIFY( q.exec(addOneRow.arg(field, qsl("'%1'").arg(i2s(i)))));
    QCOMPARE( rowCount(table), maxRows +1); // more entries

    for( int i=1; i<=maxRows; i += 2)
        QVERIFY( q.exec(qsl("DELETE FROM %1 WHERE rowid=%2").arg(table, i2s(i))));
    QCOMPARE(rowCount(table), maxRows /2 +1);
}

void test_sqlhelper::test_tableExists()
{
    QVERIFY2( not tableExists("notExistingTable"), "test of tableExists faild on NOT existing table");
    QVERIFY( QSqlQuery().exec("CREATE TABLE testSqlVal (valDate STRING)"));
    QVERIFY2(tableExists("testSqlVal"), "test of tableExists faild on existing table");
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
}
void test_sqlhelper::test_ensureTable_existingTable_tableSizeMismatch()
{
    QString tname{"tableWithFourFields"};
    QSqlQuery query;
    QVERIFY( query.exec("CREATE TABLE " + tname +
               " (id integer PRIMARY KEY AUTOINCREMENT, s STRING, i INTEGER, j INTEGER)"));
    dbtable dbt_With3Fields(tname);
    dbt_With3Fields.append(dbfield("id", QVariant::LongLong));
    dbt_With3Fields.append(dbfield("s"));
    dbt_With3Fields.append(dbfield("i", QVariant::Int));
    QVERIFY( not ensureTable(dbt_With3Fields));
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
    QVERIFY( not ensureTable(t));
}
void test_sqlhelper::test_ensureTable_nonexistingTable()
{
    dbtable t("tableThatDoesNotExist");
    t.append(dbfield("id"));
    QVERIFY(ensureTable(t));
}

void test_sqlhelper::test_eSingleValueSql_field_table_PreservesValues()
{
    // prep ->
    QString tablename("testTable");
    dbtable t(tablename);
    t.append(dbfield("id",      QVariant::LongLong).setAutoInc (true));
    t.append(dbfield("aDate",   QVariant::Date));
    t.append(dbfield("anInt",   QVariant::Int));
    t.append(dbfield("aString", QVariant::String));
    t.append(dbfield("aBool",   QVariant::Bool));
    QVERIFY(ensureTable(t));
    // <- prep
    QString string{"Hallo Welt!"};
    QDate date{1965, 5, 6};
    long i {42};
    bool b = true;
    QString createSql{ qsl("INSERT INTO %1 VALUES (NULL, '%2', %3, '%4', %5)")
                .arg(tablename, date.toString(Qt::ISODate), i2s(i), string, (b ? qsl("true") : qsl("false")))};

    QVERIFY(QSqlQuery().exec(createSql));
    //
    QVariant value =executeSingleValueSql("id",      "testtable", "id=1");
    QCOMPARE( value.toLongLong(), 1);
    QCOMPARE( value, QVariant(1)); value.clear();

    value =executeSingleValueSql("aDate",   "testtable", "id=1");
    QCOMPARE( value.toDate(), date);
    QCOMPARE( value, QVariant(date)); value.clear ();

    value =executeSingleValueSql("anInt",   "testtable", "id=1");
    QCOMPARE( value.toLongLong (), i);
    QCOMPARE( value, QVariant(qlonglong(i))); value.clear();

    value =executeSingleValueSql("aString", "testtable", "id=1");
    QCOMPARE( value.toString(), string);
    QCOMPARE( value, QVariant(string)); value.clear();

    value =executeSingleValueSql("aBool",   "testtable", "id=1");
    QCOMPARE( value.toBool(), b);
    QCOMPARE( value, QVariant(b));

    string =qsl("Hallo Andere Welt!");
    date =QDate(2022, 5, 6);
    i =LONG_MAX;
    b = false;
    createSql = qsl("INSERT INTO %1 VALUES (NULL, %2, %3, %4, %5)")
                    .arg(tablename, DbInsertableString (date.toString(Qt::ISODate)), DbInsertableString (i2s(i)),
                         DbInsertableString (string), DbInsertableString ((b ? qsl("true") : qsl("false"))));
    QVERIFY(QSqlQuery().exec(createSql));

    value =executeSingleValueSql("id",      "testtable", "id=2");
    QCOMPARE( value.toLongLong(), 2);
    QCOMPARE( value, QVariant(qlonglong(2))); value.clear ();

    value =executeSingleValueSql("aDate",   "testtable", "id=2");
    QCOMPARE( value.toDate(), date);
    QCOMPARE( value, QVariant(date)); value.clear ();

    value =executeSingleValueSql("anInt",   "testtable", "id=2");
    QCOMPARE( value.toInt(), i);
    QCOMPARE( value, QVariant(int(i)));

    value =executeSingleValueSql("aString", "testtable", "id=2");
    QCOMPARE( value.toString(), string);
    QCOMPARE( value, QVariant(string)); value.clear ();

    value =executeSingleValueSql("aBool",   "testtable", "id=2");
    QCOMPARE( value.toBool(), b);
    QCOMPARE( value, QVariant(b));
}
void test_sqlhelper::test_eSingleValueSql_dbfield_PreservsValue()
{
    // prep ->
    QString tablename("testTable");
    dbtable t(tablename);
    t.append(dbfield("id",      QVariant::LongLong).setAutoInc ());
    t.append(dbfield("aDate",   QVariant::Date));
    t.append(dbfield("anInt",   QVariant::Int));
    t.append(dbfield("aString", QVariant::String));
    t.append(dbfield("aBool",   QVariant::Bool));
    QVERIFY(ensureTable(t));
    // <- prep

    QVariant string{"Hallo Welt!"};
    QVariant date{QDate (1965, 5, 6)};
    QVariant i {42};
    QVariant b (true);
    QString createSql{ qsl("INSERT INTO %1 VALUES( NULL, %2, %3, %4, %5)")
                .arg( tablename, DbInsertableString(date), DbInsertableString(i), DbInsertableString(string), DbInsertableString(b))};

    QVERIFY(QSqlQuery().exec(createSql));
    //
    QCOMPARE(executeSingleValueSql(t["id"],     "id=1"), QVariant(1));
    QCOMPARE(executeSingleValueSql(t["aDate"],  "id=1"), date);
    QCOMPARE(executeSingleValueSql(t["anInt"],  "id=1"), i);
    QCOMPARE(executeSingleValueSql(t["aString"],"id=1"), string);
    QCOMPARE(executeSingleValueSql(t["aBool"],  "id=1"), b);
}

void test_sqlhelper::test_selectQueryFromFields_noWhere()
{
    // preparation
    QString tname{"simpletable"};
    dbtable simple(tname);
    simple.append(dbfield("id", QVariant::LongLong).setAutoInc ());
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
    simple.append(dbfield("id", QVariant::LongLong).setAutoInc ());
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
    referenced.append(dbfield("id", QVariant::LongLong).setAutoInc());
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
    qDebug() << inserter.lastError() << "\n" << inserter.lastQuery();
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
    referenced.append(dbfield("id", QVariant::LongLong).setAutoInc ());
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
    qDebug() << insertQ.lastError() << "\n" << insertQ.lastQuery();
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
    t.append(dbfield("col_index", QVariant::LongLong).setAutoInc());
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
        insertQ.addBindValue(i2s(i));
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
    t.append(dbfield("col_index", QVariant::LongLong).setAutoInc());
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
    sql += DbInsertableString(ll) +", ";
    sql += DbInsertableString(i) + ", ";
    sql += DbInsertableString(s) +", ";
    sql += DbInsertableString(d) +", ";
    sql += DbInsertableString(b) +")";

    QSqlQuery q;
    if( not q.exec(sql)) {
        qDebug() << q.lastError() << "\n" << q.lastQuery();
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
    t.append(dbfield("col_index", QVariant::LongLong).setAutoInc());
    t.append(dbfield("col_S", QVariant::String));
    QVERIFY(ensureTable(t));
    QSqlQuery q; q.prepare("INSERT INTO "+ tablename + " (col_S) VALUES (?)");
    int maxrow = 100;
    for( int i = 0; i<maxrow; i++) {
        q.addBindValue(i2s(i));
        if( not q.exec()) qDebug() << q.lastError() << "\n" << q.lastQuery();
    }
    QCOMPARE( getHighestRowId(tablename), maxrow);
    q.prepare("DELETE FROM " + tablename + " WHERE col_S=?");
    for( int i = 0; i<maxrow; i+=2) {
        q.addBindValue(i2s(i));
        if( not q.exec()) qDebug() << q.lastError() << "\n" << q.lastQuery();
    }
    QCOMPARE( getHighestRowId(tablename), maxrow);
    QCOMPARE(rowCount(tablename), maxrow/2);
}
