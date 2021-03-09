#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlField>
#include <qdebug.h>
#include <QTest>

#include "../DKV2/appconfig.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"
#include "../DKV2/booking.h"

#include "testhelper.h"


const QString testDbFilename = "../data/testdb.sqlite";

void initTestDb()
{   LOG_CALL;
    QDir().mkdir(QString("../data"));
    if (QFile::exists(testDbFilename))
        QFile::remove(testDbFilename);
    if (QFile::exists(testDbFilename))
        QFAIL("test db still in use");
    QSqlDatabase db = QSqlDatabase::addDatabase(dbTypeName);
    db.setDatabaseName(testDbFilename);
    QVERIFY(db.open());
//    init_DKDBStruct();
    QVERIFY(dkdbstructur.createDb(db));
    QVERIFY2( QFile::exists(testDbFilename), "create database failed." );
}

void initTestDb_withData()
{   LOG_CALL;
    QDir().mkdir(QString("../data"));
    if (QFile::exists(testDbFilename))
        QFile::remove(testDbFilename);
    if (QFile::exists(testDbFilename))
        QFAIL("test db still in use");
//    dbCloser closer(qsl("qt_sql_default_connection"));
    QSqlDatabase db = QSqlDatabase::addDatabase(dbTypeName);
    db.setDatabaseName(testDbFilename);
    QVERIFY(db.open());
    init_DKDBStruct();
    QVERIFY(dkdbstructur.createDb(db));
    QVERIFY2( QFile::exists(testDbFilename), "create database failed." );
    fill_DkDbDefaultContent();
    saveRandomCreditors(10);
    saveRandomContracts(8);
    activateRandomContracts(100 /* % */);
}

void cleanupTestDb()
{   LOG_CALL;
    closeAllDatabaseConnections();
    if (QFile::exists(testDbFilename))
        QFile::remove(testDbFilename);
    QDir().rmdir("../data");
    QVERIFY2( (QFile::exists(testDbFilename) == false), "destroy database failed." );
}

void openDbConnection(QString file)
{
    QSqlDatabase::addDatabase(dbTypeName);
    QSqlDatabase::database().setDatabaseName(file);
    QSqlDatabase::database().open();
}
void closeDbConnection( QSqlDatabase db /*=QSqlDatabase::database()*/)
{
    QSqlDatabase::database(db.connectionName()).close();
    QSqlDatabase::removeDatabase(db.connectionName());
}

void createEmptyFile(QString path)
{
    QFileInfo fi(path);
    QDir d(fi.absolutePath());
    d.mkpath(".");
    QFile f(fi.absoluteFilePath());
    f.open(QIODevice::WriteOnly);
    f.close();
}

int tableRecordCount( const QString tname, const QSqlDatabase db /*=QSqlDatabase::database()*/)
{   // LOG_CALL_W(tname);
    QSqlQuery q(db);
    if (q.exec("SELECT COUNT(*) FROM " + tname)) {
        q.first();
        qDebug() << "#Datensätze: " << q.record().value(0).toInt();
        return q.record().value(0).toInt();
    } else {
        qCritical() << "tableRecordCount: SELECT failed " << q.lastError() << Qt::endl << q.lastQuery() << Qt::endl;
        return -1;
    }
}

bool dbHasTable(const QString tname, const QSqlDatabase db /*=QSqlDatabase::database()*/)
{   LOG_CALL_W(tname);
    return db.tables().contains(tname);
}

bool dbTableHasField(const QString tname, const QString fname, const QSqlDatabase db /*=QSqlDatabase::database()*/)
{   LOG_CALL_W(tname +": " +fname);
    QSqlRecord r = db.record(tname);
    if( r.field(fname).isValid())
        return true;
    return false;
}

bool dbsHaveSameTables(const QString fn1, const QString fn2)
{
    dbCloser closer1(qsl("con1"));
    dbCloser closer2(qsl("con2"));

    QSqlDatabase db1 = QSqlDatabase::addDatabase(dbTypeName, closer1.conName);
    db1.setDatabaseName(fn1);
    Q_ASSERT(db1.open());
    QSqlDatabase db2 = QSqlDatabase::addDatabase(dbTypeName, closer2.conName);
    db2.setDatabaseName(fn2);
    Q_ASSERT(db2.open());
    return dbsHaveSameTables(db1, db2);
}

bool dbsHaveSameTables(const QSqlDatabase db1, const QSqlDatabase db2)
{   LOG_CALL;
    bool ret =true;
    QStringList tl1 =db1.tables();
    QStringList tl2 =db2.tables();
    if( tl1.count() != tl2.count()) {
         qInfo() << "db comparison: table list count missmatch";
         ret =false;
    }
    for (auto table: tl1) {
        if( tl2.contains(table)){
            qInfo() << "common table: " << table;
            int rc1 =rowCount(table, db1);
            int rc2 =rowCount(table, db2);
            if( rc1 != rc2) {
                qCritical() << "Tables " << table << " differ in rowCount: " << rc1 << " / " << rc2;
                ret =false;
            }
            continue;
        }
        qInfo() << "db comparison: table '" << table << "' is missing in second database";
        ret =false;
    }
    for (auto table: tl2) {
        if( tl1.contains(table))
            continue;
        qInfo() << "db comparison: table '" << table << "' is missing in first database";
        ret =false;
    }
    return ret;
}

bool dbTablesHaveSameFields(const QString table1, const QString table2, const QSqlDatabase db)
{   LOG_CALL;
    qInfo() << table1 << ", " << table2;
    bool ret =true;
    QSqlRecord rec1 =db.record(table1);
    QSqlRecord rec2 =db.record(table2);
    if( rec1.isEmpty()){
        qInfo() << "table 1 has no fields";
        ret =false;
    }
    if( rec2.isEmpty()){
        qInfo() << "table 2 has no fields";
        ret =false;
    }
    if( rec1.count() != rec2.count()) {
        qInfo() << "field count of tables to be compared are not equal: " << rec1 << rec2;
        ret =false;
    }
    for ( int i =0; i < rec1.count(); i++) {
        if( rec2.contains(rec1.field(i).name()))
                continue;
        ret =false;
        qInfo() << "rec1 contains field, which is not in rec2: " << rec1.field(i).name();
    }
    for ( int i =0; i < rec2.count(); i++) {
        if( rec1.contains(rec2.field(i).name()))
                continue;
        ret =false;
        qInfo() << "rec2 contains field, which is not in rec1: " << rec2.field(i).name();
    }
    return ret;
}
