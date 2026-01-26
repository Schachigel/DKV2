
#include "../DKV2/helperfin.h"
#include "../DKV2/dbstructure.h"
#include "../DKV2/helpersql.h"
#include "testhelper.h"

#include "test_sqlhelper.h"

void test_sqlhelper::init()
{
    reInit_DKDBStruct();
    openDefaultDbConnection_InMemory();
}

void test_sqlhelper::cleanup()
{
    closeDefaultDbConnection();
}
void test_sqlhelper::test_init_cleanup()
{
    // this should not leak connections
    // log must not contain
    // QSqlDatabasePrivate::removeDatabase: connection 'qt_sql_default_connection' is still in use, all queries will cease to work.
}

void test_sqlhelper::test_rowCount()
{
    QString table {qsl("tablename")};
    QString field {qsl("fieldname")};
    qInfo() << "expect error";
    QCOMPARE( rowCount(qsl("notexistingTable")), -1); // non existing table

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
void test_sqlhelper::test_getHighestRowId()
{
    QString tablename("test_getHighestRowId");
    dbtable t(tablename);
    t.append(dbfield("col_index", QMetaType::LongLong).setAutoInc());
    t.append(dbfield("col_S", QMetaType::QString));
    QVERIFY(ensureTable(t));
    QSqlQuery q; q.prepare("INSERT INTO "+ tablename + " (col_S) VALUES (?)");
    int maxrow = 15;
    for( int i = 0; i<maxrow; i++) {
        q.addBindValue(i2s(i));
        if( not q.exec())
            qInfo() << q.lastError() << "\n" << q.lastQuery();
    }
    QCOMPARE( getHighestRowId(tablename), maxrow);
    q.prepare("DELETE FROM " + tablename + " WHERE col_index=?");
    for( int i = 1; i<=3; i++) { // deleting any row will not change max rowid
        q.addBindValue(i);
        if( not q.exec())
            qInfo() << q.lastError() << "\n" << q.lastQuery();
    }
    QCOMPARE( getHighestRowId(tablename), maxrow);
    QCOMPARE(rowCount(tablename), maxrow-3);
    // but deleting the last row will change max rowid
    q.addBindValue (maxrow);
    q.exec();
    QCOMPARE(rowCount(tablename), maxrow -4);
    QCOMPARE(getHighestRowId (tablename), maxrow -1);
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
    t.append(dbfield("id", QMetaType::LongLong));
    t.append(dbfield("s"));
    t.append(dbfield("i", QMetaType::Int));
    QVERIFY(ensureTable(t));
}
void test_sqlhelper::test_ensureTable_existingTable_tableSizeMismatch()
{
    QString tname{"tableWithFourFields"};
    QSqlQuery query;
    QVERIFY( query.exec("CREATE TABLE " + tname +
               " (id integer PRIMARY KEY AUTOINCREMENT, s STRING, i INTEGER, j INTEGER)"));
    dbtable dbt_With3Fields(tname);
    dbt_With3Fields.append(dbfield("id", QMetaType::LongLong));
    dbt_With3Fields.append(dbfield("s"));
    dbt_With3Fields.append(dbfield("i", QMetaType::Int));
    QVERIFY( not ensureTable(dbt_With3Fields));
}
void test_sqlhelper::test_ensureTable_existingtable_fieldTypeMismatch()
{
    QString tname{"ensureTable03"};
    QSqlQuery query;
    QVERIFY( query.exec("CREATE TABLE " + tname
               + " (id integer PRIMARY KEY AUTOINCREMENT, s STRING, i INTEGER)"));
    dbtable t(tname);
    t.append(dbfield("id", QMetaType::QString));
    t.append(dbfield("s"));
    t.append(dbfield("i", QMetaType::Int));
    QVERIFY( not ensureTable(t));
}
void test_sqlhelper::test_ensureTable_nonexistingTable()
{
    dbtable t("tableThatDoesNotExist");
    t.append(dbfield("id"));
    QVERIFY(ensureTable(t));
}

void test_sqlhelper::test_executeSql_noParams()
{
    {
        // no sql
        qInfo() << "expect error (no sql)";
        QVector<QSqlRecord> result;
        QVERIFY (not executeSql( QString(), result));
        QVERIFY (result.isEmpty ());
    }
    {
        // no table
        qInfo() << "expect error (no table)";
        QVector<QSqlRecord> result;
        QVERIFY (not executeSql(qsl("SELECT * FROM nonexisting"), result));
        QVERIFY (result.isEmpty ());
    }
    // perp
    QString tname {qsl("t1")};
    QVERIFY( QSqlQuery().exec("CREATE TABLE " + tname
               + " (id integer PRIMARY KEY AUTOINCREMENT, s STRING, i INTEGER)"));
    {
        // empty table
        QVector<QSqlRecord> result;
        QVERIFY (executeSql (qsl("SELECT * FROM %1").arg(tname), result));
        QVERIFY (result.isEmpty ());
    }
    {
        QVERIFY (QSqlQuery().exec (qsl("INSERT INTO %1 (s, i) VALUES ('string', 42)").arg(tname)));
        // one record
        QVector<QSqlRecord> result;
        QVERIFY (executeSql (qsl("SELECT * FROM %1").arg(tname), result));
        QCOMPARE (result.size(), 1);
    }
    {
        // w single value in result
        QVector<QSqlRecord> result;
        QVERIFY (executeSql (qsl("SELECT count(*) FROM %1").arg(tname), result));
        QCOMPARE (result.size(), 1);
        QCOMPARE( result.at(0).value (0), QVariant(1));
    }

}
void test_sqlhelper::test_executeSql_namedParams()
{
    // perp
    QString tname {qsl("t1")};
    QVERIFY( QSqlQuery().exec("CREATE TABLE " + tname
               + " (id integer PRIMARY KEY AUTOINCREMENT, s STRING, i INTEGER)"));
    {
        QVERIFY (executeSql_wNoRecords (qsl("INSERT INTO %1 (s, i) VALUES ('string', :param)").arg(tname), QVariant(42)));
        QVERIFY (rowCount(tname) == 1);
        // one record
        QVector<QSqlRecord> result;
        QVERIFY (executeSql (qsl("SELECT * FROM %1 WHERE i=:param").arg(tname),  QVector<QPair<QString, QVariant>>{{qsl(":param"), QVariant(42)}}, result));
        QCOMPARE (result.size(), 1);
    }
    {
        // params in where clause
        QVector<QPair<QString, QVariant>> params {{qsl(":p"), QVariant(1)}};
        QVector<QSqlRecord> result;
        QVERIFY (executeSql (qsl("SELECT * FROM %1 WHERE id=:p").arg(tname), params, result));
        QCOMPARE (result.size(), 1);
    }
    {
        // params in VALUES clause
        QVector<QSqlRecord> result;
        QVector<QPair<QString, QVariant>> params {{qsl(":pa2"), QVariant(13)}, {qsl(":pa1"), QVariant( qsl("Hallo World"))}};
        QVERIFY( executeSql( qsl("INSERT INTO %1 (s, i) VALUES( :pa1, :pa2)").arg(tname), params, result));
        // too many parameters for this query
        QVERIFY( not executeSql( qsl("SELECT * FROM %1 WHERE i=:pa2").arg(tname), params, result));
        // right number of parameters
        params ={{qsl(":pa2"), QVariant(13)}};
        QVERIFY( executeSql( qsl("SELECT * FROM %1 WHERE i=:pa2").arg(tname), params, result));
        QVERIFY( not result.isEmpty ());
        QCOMPARE( result.at(0).value (qsl("s")), qsl("Hallo World"));
    }
    // longer parameter names
    {
        QVector<QSqlRecord> result;
        QVector<QPair<QString, QVariant>> params {{qsl(":long1"), QVariant( qsl("Hallo World-2"))}, {qsl(":long2"), QVariant(55)}};
        QVERIFY( executeSql(qsl("INSERT INTO %1 (s, i) VALUES(:long1, :long2)").arg(tname), params, result));
        QVector<QSqlRecord> selectResult;
        params ={{qsl(":long2"), QVariant(55)}};
        QVERIFY( executeSql(qsl("SELECT * FROM %1 WHERE i=:long2").arg(tname), params, selectResult));
        QCOMPARE( selectResult.at(0).value (qsl("s")), qsl("Hallo World-2"));
    }
    // parameter formats like @VVV and $VVV DO NOT WORK!
    {
        QVector<QSqlRecord> result;
        QVector<QPair<QString, QVariant>> params {{qsl("@VVV"), QVariant( qsl("Hallo World2"))}, {qsl("@WWW"), QVariant(14)}};
        QVERIFY ( not executeSql( qsl("INSERT INTO %1 (s, i) VALUES( @VVV, @WWW)").arg(tname), params, result));
        QVERIFY ( not executeSql( qsl("SELECT * FROM %1 WHERE i=@WWW").arg(tname), params, result));
        QVERIFY( result.isEmpty ());
    }
}
void test_sqlhelper::test_executeSql_posParams()
{
    QVector<QVariant> noparams;
    {
        qInfo() <<  __FUNCTION__ << "Test: no sql";
        QVector<QSqlRecord> result;
        QVERIFY (not executeSql( QString(), noparams, result));
        QVERIFY (result.isEmpty ());
    }
    {
        qInfo() <<  __FUNCTION__ << "Test: no table";
        QVector<QSqlRecord> result;
        QVERIFY (not executeSql(qsl("SELECT * FROM nonexisting"), result));
        QVERIFY (result.isEmpty ());
    }
    // perp
    QString tname {qsl("t1")};
    QVERIFY( QSqlQuery().exec("CREATE TABLE " + tname
               + " (id integer PRIMARY KEY AUTOINCREMENT, s STRING, i INTEGER)"));
    {
        qInfo() <<  __FUNCTION__ << "Test: empty table";
        QVector<QSqlRecord> result;
        QVERIFY (executeSql (qsl("SELECT * FROM %1").arg(tname), noparams, result));
        QVERIFY (result.isEmpty ());
    }
    {
        QVERIFY (QSqlQuery().exec (qsl("INSERT INTO %1 (s, i) VALUES ('string', 42)").arg(tname)));
        qInfo() <<  __FUNCTION__ << "Test: one record";
        QVector<QSqlRecord> result;
        QVERIFY (executeSql (qsl("SELECT * FROM %1").arg(tname), noparams, result));
        QCOMPARE (result.size(), 1);
    }
    {
        qInfo() <<  __FUNCTION__ << "Test: w no records in result";
        QVector<QSqlRecord> result;
        QVERIFY (executeSql (qsl("SELECT count(*) FROM %1").arg(tname), noparams, result));
        QCOMPARE (result.size(), 1);
        QCOMPARE( result.at(0).value (0), QVariant(1));
    }
    {
        qInfo() <<  __FUNCTION__ << "Test: params in where clause";
        QVector<QVariant> params {QVariant(1)};
        QVector<QSqlRecord> result;
        QVERIFY (executeSql (qsl("SELECT * FROM %1 WHERE id=?").arg(tname), params, result));
        QCOMPARE (result.size(), 1);
    }
    {
        qInfo() <<  __FUNCTION__ << "Test: params in VALUES clause";
        QVector<QSqlRecord> result;
        QVector<QVariant> params { QVariant( qsl("Hallo World")), QVariant(13)};
        QVERIFY (executeSql (qsl("INSERT INTO %1 (s, i) VALUES(?, ?)").arg(tname), params, result));
        result.clear ();
        QVERIFY (executeSql (qsl("SELECT * FROM %1 WHERE s=? AND i=?").arg(tname), params, result));
        QCOMPARE( result.at(0).value (qsl("s")), qsl("Hallo World"));
    }
}

void test_sqlhelper::test_switchForeignKeyHandling()
{
    QVERIFY( not getForeignKeyHandlingStatus ());
    switchForeignKeyHandling (fkh_on);
    QVERIFY( getForeignKeyHandlingStatus ());
    switchForeignKeyHandling (fkh_off);
    QVERIFY( not getForeignKeyHandlingStatus ());
    // check the correct sql was build in the log
    QVERIFY( not switchForeignKeyHandling (fkh_on, qsl("alias")));
}

// continue here

void test_sqlhelper::test_eSingleValueSql_query()
{
    // prep ->
    QString tablename("testTable");
    dbtable t(tablename);
    t.append(dbfield("id",      QMetaType::LongLong).setAutoInc (true));
    t.append(dbfield("aDate",   QMetaType::QDate));
    t.append(dbfield("anInt",   QMetaType::Int));
    t.append(dbfield("aString", QMetaType::QString));
    t.append(dbfield("aBool",   QMetaType::Bool));
    QVERIFY(ensureTable(t));
    QString string{"Hallo Welt!"};
    QDate date{1965, 5, 6};
    long i {42};
    bool b = true;
    QString createSql{ qsl("INSERT INTO %1 VALUES (NULL, '%2', %3, '%4', %5)")
                .arg(tablename, date.toString(Qt::ISODate), i2s(i), string, (b ? qsl("true") : qsl("false")))};
    QVERIFY(QSqlQuery().exec(createSql));
    // test // too many values
    QVariant v =executeSingleValueSql (qsl("SELECT %1, %2 FROM %3").arg(t["id"].name (), t["aDate"].name(), t.Name ()));
    QVERIFY( not v.isValid());
}
void test_sqlhelper::test_eSingleValueSql_field_table_PreservesValues()
{
    // prep ->
    QString tablename("testTable");
    dbtable t(tablename);
    t.append(dbfield("id",      QMetaType::LongLong).setAutoInc (true));
    t.append(dbfield("aDate",   QMetaType::QDate));
    t.append(dbfield("anInt",   QMetaType::Int));
    t.append(dbfield("aString", QMetaType::QString));
    t.append(dbfield("aBool",   QMetaType::Bool));
    QVERIFY(ensureTable(t));
    QString string{"Hallo Welt!"};
    QDate date{1965, 5, 6};
    long i {42};
    bool b = true;
    QString createSql{ qsl("INSERT INTO %1 VALUES (NULL, '%2', %3, '%4', %5)")
                .arg(tablename, date.toString(Qt::ISODate), i2s(i), string, (b ? qsl("true") : qsl("false")))};
    QVERIFY(QSqlQuery().exec(createSql));
    // test ->
    QVariant value =executeSingleValueSql("id",      "testtable", "id=1");
    QCOMPARE( value.toLongLong(), 1);
    QCOMPARE( value, QVariant(1)); value.clear();

    value =executeSingleValueSql("aDate",   "testtable", "id=1");
    QCOMPARE( value.toDate(), date);
    // QCOMPARE( value, QVariant(date)); changed in Qt6; QVariant can not know the type here and there is no more implizit conversion
    value.clear ();

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
    // QCOMPARE( value, QVariant(date)); changed in Qt6; QVariant can not know the type here and there is no more implizit conversion
    value.clear ();

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
    t.append(dbfield("id",      QMetaType::LongLong).setAutoInc ());
    t.append(dbfield("aDate",   QMetaType::QDate));
    t.append(dbfield("anInt",   QMetaType::Int));
    t.append(dbfield("aString", QMetaType::QString));
    t.append(dbfield("aBool",   QMetaType::Bool));

    dkdbstructur.appendTable(t);

    QVERIFY(ensureTable(t));

    // <- prep
    QVariant string{"Hallo Welt!"};
    QVariant date{QDate (1965, 5, 6)};
    QVariant i {42};
    QVariant b (true);
    QString createSql{ qsl("INSERT INTO %1 VALUES( NULL, %2, %3, %4, %5)")
                .arg( tablename, DbInsertableString(date), DbInsertableString(i), DbInsertableString(string), DbInsertableString(b))};

    QVERIFY(QSqlQuery().exec(createSql));

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
    simple.append(dbfield("id", QMetaType::LongLong).setAutoInc ());
    simple.append(dbfield("col"));
    ensureTable(simple);
    QString sqltemp {qsl("INSERT INTO [%1] VALUES( NULL, %2)").arg(tname)};
    QSqlQuery().exec(sqltemp.arg(qsl("'Hallo '")));
    QSqlQuery().exec(sqltemp.arg(qsl("'Welt!'")));
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
    simple.append(dbfield("id", QMetaType::LongLong).setAutoInc ());
    simple.append(dbfield("col"));
    ensureTable(simple);
    QString sqltemp {qsl("INSERT INTO [%1] VALUES( NULL, %2)").arg(tname)};
    QSqlQuery().exec(sqltemp.arg(qsl("'Hallo '")));
    QSqlQuery().exec(sqltemp.arg(qsl("'Welt!'")));
    // test
    QString sql = selectQueryFromFields(simple.Fields(), "id=2");
    QSqlQuery probe;
    QVERIFY( probe.exec(sql));
    probe.first();
    QCOMPARE( probe.record().value("col").toString(), "Welt!");
}

void test_sqlhelper::test_executeSingleColumnSql()
{
    // prep
    QString tablename("test_executeSingleColumnSql");
    dbtable t(tablename);
    t.append(dbfield("col_index", QMetaType::LongLong).setAutoInc());
    t.append(dbfield("col_ll", QMetaType::LongLong));
    t.append(dbfield("col_I", QMetaType::Int));
    t.append(dbfield("col_S", QMetaType::QString));
    t.append(dbfield("col_D", QMetaType::QDate));
    t.append(dbfield("col_B", QMetaType::Bool));
    dbstructure db; db.appendTable(t); db.createDb();

    QVERIFY( executeSingleColumnSql(t["col_ll"]).isEmpty());
    QSqlQuery insertQ;
    insertQ.prepare("INSERT INTO " + tablename + " VALUES (NULL, ?, ?, ?, ?, ?)");
    int rowcount =3;
    for ( int i = 0; i < rowcount; i++) {
        insertQ.addBindValue(i);
        insertQ.addBindValue(-i);
        insertQ.addBindValue(i2s(i));
        insertQ.addBindValue(QDate::currentDate().addDays (i));
        insertQ.addBindValue(i%2==0);
        insertQ.exec();
    }
    // TESTs
    QVector<QVariant> result = executeSingleColumnSql(t["col_ll"]);
    QCOMPARE( result.count(), rowcount);
    qlonglong i =0;
    for ( auto& v : result) {
        QVERIFY(v.metaType() == t["col_ll"].metaType());
        QCOMPARE( v.toLongLong (), i++);
    }
    //
    result = executeSingleColumnSql(t["col_I"]);
    int j =0;
    QCOMPARE( result.count(), rowcount);
    for ( auto& v : result) {
        QVERIFY(v.metaType() == t["col_I"].metaType());
        QCOMPARE( v.toInt (), j--);
    }
    //
    result = executeSingleColumnSql(t["col_S"]);
    QCOMPARE( result.count(), rowcount);
    i =0;
    for ( auto& v : result) {
        QVERIFY(v.metaType() == t["col_S"].metaType());
        QCOMPARE (v.toString (), i2s(i++));
    }
    //
    result = executeSingleColumnSql(t["col_D"]);
    QCOMPARE( result.count(), rowcount);
    i =0;
    for ( auto& v : result) {
        QVERIFY(v.metaType() == t["col_D"].metaType());
        QCOMPARE(v.toDate (), QDate::currentDate ().addDays (i++));
    }
    //
    result = executeSingleColumnSql(t["col_B"]);
    QCOMPARE( result.count(), rowcount);
    i =0;
    for ( auto& v : result) {
        QVERIFY(v.metaType() == t["col_B"].metaType());
        QCOMPARE( v.toBool (), i++%2 == 0);
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
    t.append(dbfield("col_index", QMetaType::LongLong).setAutoInc());
    t.append(dbfield("col_ll", QMetaType::LongLong));
    t.append(dbfield("col_I", QMetaType::Int));
    t.append(dbfield("col_S", QMetaType::QString));
    t.append(dbfield("col_D", QMetaType::QDate));
    t.append(dbfield("col_B", QMetaType::Bool));
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
        qInfo() << q.lastError() << "\n" << q.lastQuery();
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
    qInfo() << record;

    // data types are back to what we put in
    QCOMPARE( ll, record.value("col_ll"));
    QCOMPARE( ll.metaType(), record.value("col_ll").metaType());
    QCOMPARE(  i, record.value("col_I"));
    QCOMPARE(  i.metaType(), record.value("col_I").metaType());
    QCOMPARE(  s, record.value("col_S"));
    QCOMPARE(  s.metaType(), record.value("col_S").metaType());
    QCOMPARE(  d, record.value("col_D"));
    QCOMPARE(  d.metaType(), record.value("col_D").metaType());
    QCOMPARE(  b, record.value("col_B"));
    QCOMPARE(  b.metaType(), record.value("col_B").metaType());
}

namespace {
bool viewExistsInSqlite_Master(const QString& name)
{
    return executeSingleValueSql (qsl("SELECT name FROM sqlite_master WHERE name=? AND type=?"), QVector<QVariant>({name, qsl("view")})).toBool ();
}
bool viewCanBeExecuted(const QString& name)
{
    QVector<QSqlRecord> res;
    return executeSql (qsl("SELECT * FROM %1").arg(name), res);
}
}

void test_sqlhelper::test_createDbViews()
{
    // prep
    dbtable table(qsl("t1"));
    table.append (dbfield(qsl("f1"), QMetaType::Int));
    table.append (dbfield(qsl("f2"), QMetaType::QString));
    ensureTable (table);
    // test test function
    QVERIFY(not viewExistsInSqlite_Master(qsl("notExistingView")));
    // tests
    {
        QString viewName {qsl("View1")};
        QVERIFY( createPersistentDbView(viewName, qsl("SELECT * FROM t1")));
        QVERIFY(viewExistsInSqlite_Master( viewName));
        QVERIFY(viewCanBeExecuted (viewName));
    }
    {
        QString viewName2 {qsl("View2")};
        QVERIFY( createTemporaryDbView(viewName2, qsl("SELECT * FROM t1")));
        // temporary views do not show up in the sqlite_master
        QVERIFY(not viewExistsInSqlite_Master( viewName2));
        QVERIFY(viewCanBeExecuted (viewName2));
    }
}
