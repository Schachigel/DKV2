
#include <QtSql>
#include <QtTest>

#include "../DKV2/tabledatainserter.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/dbstructure.h"
#include "../DKV2/dkdbcopy.h"

#include "test_dkdbcopy.h"
void test_dkdbcopy::init()
{
    cleanup();
}

void test_dkdbcopy::cleanup() {
    QFile::remove(dbfn1);
    QVERIFY( not QFile::exists(dbfn1));
    QFile::remove(dbfn2);
    QVERIFY( not QFile::exists(dbfn2));
    QFile::remove(testDbFilename);
    QVERIFY( not QFile::exists(testDbFilename));
    QFile::remove(tempFileName);
    QVERIFY( not QFile::exists(tempFileName));
}

void test_dkdbcopy::test_moveToPreconversionBackup()
{
    createEmptyFile(testDbFilename);
    QString result =moveToPreConversionCopy(testDbFilename);
    QVERIFY( result.size());
    QVERIFY( not QFile::exists(testDbFilename));
    QVERIFY( QFile::exists(result));
}

void test_dkdbcopy::test_moveToPreconversionBackup_tmpfn()
{
    createEmptyFile(testDbFilename);
    QString result =moveToPreConversionCopy(testDbFilename);
    QVERIFY( result.size());
    QVERIFY( not QFile::exists(testDbFilename));
    QVERIFY( QFile::exists(result));
    QVERIFY( QFile::remove(result));
}


void test_dkdbcopy::test_dbsHaveSameTables()
{
    dbstructure dbs1;
    dbtable t1("t1");
    t1.append(dbfield("t1f1"));
    dbs1.appendTable(t1);
    QVERIFY(dbs1.createDb(dbfn1));
    QVERIFY(dbs1.createDb(dbfn2));

    QVERIFY(dbsHaveSameTables(dbfn1, dbfn2));
}

void test_dkdbcopy::test_dbsHaveSameTables_mtpl_tables()
{
    dbstructure dbs1;
    dbtable t1("t1");
    t1.append(dbfield("t1f1"));
    t1.append(dbfield("t1f2"));
    dbs1.appendTable(t1);
    dbtable t2("t2");
    t2.append((dbfield("t2f1")));
    dbs1.appendTable(t2);

    QVERIFY(dbs1.createDb(dbfn1));
    QVERIFY(dbs1.createDb(dbfn2));

    QVERIFY(dbsHaveSameTables(dbfn1, dbfn2));
}

void test_dkdbcopy::test_dbsHaveSameTables_fails_more_tables()
{
    dbstructure dbs1;
    dbtable t1("t1");
    t1.append(dbfield("t1f1"));
    dbs1.appendTable(t1);
    dbs1.createDb(dbfn1);

    dbtable t2("t2");
    t2.append(dbfield("t2f1"));
    dbs1.appendTable(t2);
    dbs1.createDb(dbfn2);

    QVERIFY( not dbsHaveSameTables(dbfn1, dbfn2));
}

void test_dkdbcopy::test_dbsHaveSameTables_more_fields()
{
    dbstructure dbs1;
    dbtable t1("t1");
    t1.append(dbfield("t1f1"));
    dbs1.appendTable(t1);
    dbs1.createDb(dbfn1);

    dbstructure dbs2;
    t1.append(dbfield("t1f2"));
    dbs2.appendTable(t1);
    dbs2.createDb(dbfn2);

    QVERIFY( dbsHaveSameTables(dbfn1, dbfn2));
//    dbCloser closer (qsl("con"));
//    QSqlDatabase db =QSqlDatabase::addDatabase(dbTypeName, closer.conName);
//    db.setDatabaseName(dbfn1);
//    db.open();

    autoDb db(dbfn1, "con");

    autoDetachDb ad(qsl("db2"), db.conName());
    ad.attachDb(dbfn2);
    QVERIFY( not dbTablesHaveSameFields(t1.Name(), qsl("db2.") +t1.Name(), db));
}


bool insertData(const QString& dbfn, const QString& table, const QString& field)
{
    static int i =0;
    i++;
    dbCloser closer{qsl("con")};
    QSqlDatabase db =QSqlDatabase::addDatabase(dbTypeName, closer.conName);
    db.setDatabaseName(dbfn);
    if( not db.open())
        return false;
    QString sql (qsl("INSERT INTO %1 (%2) VALUES ('%3')"));
    return executeSql_wNoRecords(sql.arg(table).arg(field).arg(QString::number(i)), QVector<QVariant>(), db);
}

void test_dkdbcopy::test_dbsHaveSameTables_fails_diffRowCount()
{
    QString tname {qsl("t1")};
    QString fname {qsl("t1f1")};
    dbstructure dbs1;
    dbtable t1(tname);
    t1.append(dbfield(fname));
    dbs1.appendTable(t1);

    dbs1.createDb(dbfn1);
    dbs1.createDb(dbfn2);

    QVERIFY(insertData(dbfn1, tname, fname));
    QVERIFY(insertData(dbfn2, tname, fname));
    QVERIFY( dbsHaveSameTables(dbfn1, dbfn2));

    QVERIFY(insertData(dbfn2, tname, fname));
    QVERIFY( not dbsHaveSameTables(dbfn1, dbfn2));
}

void test_dkdbcopy::test_copyDatabase()
{
    // setup
    createTestDb_withRandomData();
//    QSqlDatabase::database().close();
//    QSqlDatabase::database().removeDatabase(QSqlDatabase::database().connectionName());

    // code under test
    copy_database(testDbFilename, tempFileName);
    // verification
    QVERIFY(dbsHaveSameTables(testDbFilename, tempFileName));
    // cleanup
    QSqlDatabase::database().close();
    QSqlDatabase::removeDatabase(QSqlDatabase::database().connectionName());
}

void test_dkdbcopy::test_convertDatabaseInplace() {

    // setup
    createTestDb_withRandomData();
    closeDbConnection();
    // code under test
    QString temp =convert_database_inplace(testDbFilename);
    // verification
    QVERIFY(dbsHaveSameTables(testDbFilename, temp));
    QFile::remove (temp);
}

void test_dkdbcopy::test_convertDatabaseInplace_wNewColumn()
{
    // SETUP
    // define original datastructure before the change: 2 tables
    dbstructure oldDbStructure;
    dbtable t1(qsl("t1"));
    t1.append(dbfield(qsl("id"), QVariant::LongLong).setPrimaryKey().setAutoInc());
    t1.append(dbfield(qsl("f1")));
    oldDbStructure.appendTable(t1);
    dbtable t2(qsl("t2"));
    t2.append(dbfield(qsl("id"), QVariant::LongLong).setPrimaryKey().setAutoInc());
    t2.append(dbfield(qsl("t1id"), QVariant::LongLong).setNotNull());
    t2.append(dbForeignKey(t2[qsl("t1id")], oldDbStructure[qsl("t1")][qsl("id")], qsl("ON DELETE CASCADE")));
    t2.append(dbfield(qsl("t2f1")));
    oldDbStructure.appendTable(t2);
    // create source db with old db structure
    oldDbStructure.createDb(dbfn1);
    // fill the "old" database file with some data
    {
        dbCloser closer {qsl("closer")};
        QSqlDatabase db =QSqlDatabase::addDatabase(dbTypeName, closer.conName);
        db.setDatabaseName(dbfn1); db.open();
        TableDataInserter tdi1{t1};
        tdi1.setValue("fi", "v1");    tdi1.WriteData(db);
        tdi1.setValue("fi", "v2");    tdi1.WriteData(db);
        TableDataInserter tdi2{t2};
        tdi2.setValue("t1id", 1); tdi2.setValue("t2f1", "v1"); tdi2.WriteData(db);
        tdi2.setValue("t1id", 2); tdi2.setValue("t2f1", "v2"); tdi2.WriteData(db);
    }
    // define new, extended data structure
    dbstructure newDbStructure;
    t2.append(dbfield(qsl("newField")));
    newDbStructure.appendTable(t1);
    newDbStructure.appendTable(t2);
    // Code under TEST:
    // convert the old file into a file with the new datastructure
    QVERIFY( convert_database_inplace(dbfn1, newDbStructure).size());
    // VERIFICATION
    dbCloser closer {qsl("closeVerifiy")};
    QSqlDatabase verifyDb =QSqlDatabase::addDatabase(dbTypeName, closer.conName);
    verifyDb.setDatabaseName(dbfn1); verifyDb.open();
    QVERIFY(QFile::exists(dbfn1));
    QVERIFY(hasAllTablesAndFields(verifyDb, newDbStructure));
    QCOMPARE(rowCount("t1", verifyDb), 2);
    QCOMPARE(rowCount("t2", verifyDb), 2);
}
