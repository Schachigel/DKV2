
#include <QtSql>
#include <QtTest>

#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"
#include "../DKV2/booking.h"
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
    QVERIFY( ! QFile::exists(dbfn1));
    QFile::remove(dbfn2);
    QVERIFY( ! QFile::exists(dbfn2));
    QFile::remove(testDbFilename);
    QVERIFY( ! QFile::exists(testDbFilename));
    QFile::remove(tempFileName);
    QVERIFY( ! QFile::exists(tempFileName));
}

void test_dkdbcopy::test_moveToPreconversionBackup()
{
    QFile file(testDbFilename);
    file.open(QIODevice::WriteOnly);
    file.close();
    QString result =createPreConversionCopy(testDbFilename, tempFileName);
    QVERIFY( ! result.isEmpty());
    QVERIFY( ! QFile::exists(testDbFilename));
    QVERIFY( QFile::exists(result));
}

void test_dkdbcopy::test_moveToPreconversionBackup_tmpfn()
{
    QFile file(testDbFilename);
    file.open(QIODevice::WriteOnly);
    file.close();
    QString result =createPreConversionCopy(testDbFilename);
    QVERIFY( ! result.isEmpty());
    QVERIFY( ! QFile::exists(testDbFilename));
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

    QVERIFY( ! dbsHaveSameTables(dbfn1, dbfn2));
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
    dbCloser closer (qsl("con"));
    QSqlDatabase db =QSqlDatabase::addDatabase(qsl("QSQLITE"), closer.conName);
    db.setDatabaseName(dbfn1);
    db.open();
    autoDetachDb ad(qsl("db2"), closer.conName);
    ad.attachDb(dbfn2);
    QVERIFY( ! dbTablesHaveSameFields(t1.Name(), qsl("db2.") +t1.Name(), db));
}


bool insertData(const QString& dbfn, const QString& table, const QString& field)
{
    static int i =0;
    i++;
    dbCloser closer{qsl("con")};
    QSqlDatabase db =QSqlDatabase::addDatabase(qsl("QSQLITE"), closer.conName);
    db.setDatabaseName(dbfn);
    if( ! db.open())
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
    QVERIFY( ! dbsHaveSameTables(dbfn1, dbfn2));
}

void test_dkdbcopy::test_convertDatabaseInplace() {

    // setup
    init_DKDBStruct();
    initTestDb();
    fill_dbDefaultContent();
    saveRandomCreditors(10);
    saveRandomContracts(8);
    activateRandomContracts(100 /* % */);
    QSqlDatabase::database().close();
    QSqlDatabase::removeDatabase(QSqlDatabase::database().connectionName());
    // code under test
    convert_database_inplace(testDbFilename, tempFileName);
    // verification
    QVERIFY(dbsHaveSameTables(testDbFilename, tempFileName));
}
