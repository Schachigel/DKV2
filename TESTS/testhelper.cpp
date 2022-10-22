
#include "../DKV2/helpersql.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"

#include "testhelper.h"

void getRidOfFile(QString filename)
{
    if( QFile::exists(filename))
        QFile::remove(filename);
    QVERIFY(not QFile::exists(filename));
}

void initTestDb_InMemory()
{   LOG_CALL;
    init_DKDBStruct();
    openDbConnection_InMemory();
    QVERIFY(dkdbstructur.createDb());
}

void initTestDb()
{   LOG_CALL;
    init_DKDBStruct();
    QDir().mkdir(QString("../data"));
    if (QFile::exists(testDbFilename))
        QFile::remove(testDbFilename);
    if (QFile::exists(testDbFilename))
        QFAIL("test db still in use");
    openDbConnection();
    QVERIFY(dkdbstructur.createDb());
    QVERIFY2( QFile::exists(testDbFilename), "create database failed." );
}
void createTestDb()
{   LOG_CALL;
    initTestDb();
    QVERIFY(fill_DkDbDefaultContent());
}
void createTestDbTemplate()
{
    createTestDb();
    closeAllDatabaseConnections();
    QFile::copy(testDbFilename, testTemplateDb);
}
void cleanupTestDbTemplate()
{
    getRidOfFile(testDbFilename);
    getRidOfFile(testTemplateDb);
}
void initTestDbFromTemplate()
{
    getRidOfFile(testDbFilename);
    QFile::copy(testTemplateDb, testDbFilename);
    openDbConnection();
    switchForeignKeyHandling();
}
void createTestDb_withRandomData()
{   LOG_CALL;
    dbgTimer t(qsl("createTestDb_wRandomData"));
    createTestDb();
    saveRandomCreditors(10);
    saveRandomContracts(8);
    activateRandomContracts(100 /* % */);
}
void cleanupTestDb_InMemory()
{   LOG_CALL;
    closeAllDatabaseConnections();
}
void cleanupTestDb()
{   LOG_CALL;
    closeAllDatabaseConnections();
    if (QFile::exists(testDbFilename))
        QFile::remove(testDbFilename);
//    QDir().rmdir("../data");
    QVERIFY2( (QFile::exists(testDbFilename) == false), "destroy database failed." );
}
void openDbConnection_InMemory()
{
    QSqlDatabase::addDatabase(dbTypeName);
    QSqlDatabase::database().setDatabaseName(qsl(":memory:"));
    QVERIFY(QSqlDatabase::database().open());
}
void openDbConnection(QString file /*testDbFilename*/)
{
    QSqlDatabase::addDatabase(dbTypeName);
    QSqlDatabase::database().setDatabaseName(file);
    QVERIFY(QSqlDatabase::database().open());
}
void closeDbConnection( QSqlDatabase db /*=QSqlDatabase::database()*/)
{
    QSqlDatabase::database(db.connectionName()).close();
    QSqlDatabase::removeDatabase(db.connectionName());
}

void createEmptyFile(const QString& path)
{
    QFileInfo fi(path);
    QDir d(fi.absolutePath());
    d.mkpath(".");
    QFile f(fi.absoluteFilePath());
    f.open(QIODevice::WriteOnly);
    f.close();
}

//int tableRecordCount( const QString& tname, const QSqlDatabase& db /*=QSqlDatabase::database()*/)
//{   // LOG_CALL_W(tname);
//    QSqlQuery q(db);
//    if (q.exec("SELECT COUNT(*) FROM " + tname)) {
//        q.first();
//        qDebug() << "#Datensätze: " << q.record().value(0).toInt();
//        return q.record().value(0).toInt();
//    } else {
//        qCritical() << "tableRecordCount: SELECT failed " << q.lastError() << "\n" << q.lastQuery() << qsl("\n");
//        return -1;
//    }
//}

bool dbHasTable(const QString& tname, const QSqlDatabase& db /*=QSqlDatabase::database()*/)
{   LOG_CALL_W(tname);
    return db.tables().contains(tname);
}

bool dbTableHasField(const QString& tname, const QString& fname, const QSqlDatabase& db /*=QSqlDatabase::database()*/)
{   LOG_CALL_W(tname +": " +fname);
    QSqlRecord r = db.record(tname);
    if( r.field(fname).isValid())
        return true;
    return false;
}

bool dbsHaveSameTables(const QString& fn1, const QString& fn2)
{
    dbCloser closer1(qsl("con1"));
    dbCloser closer2(qsl("con2"));

    QSqlDatabase db1 = QSqlDatabase::addDatabase(dbTypeName, closer1.conName);
    db1.setDatabaseName(fn1);
    bool open =db1.open();
    Q_ASSERT(open);
    QSqlDatabase db2 = QSqlDatabase::addDatabase(dbTypeName, closer2.conName);
    db2.setDatabaseName(fn2);
    open =db2.open();
    Q_ASSERT(open);
    return dbsHaveSameTables(db1, db2);
}

bool dbsHaveSameTables(const QSqlDatabase &db1, const QSqlDatabase &db2)
{   LOG_CALL;
    bool ret =true;
    QStringList tl1 =db1.tables();
    QStringList tl2 =db2.tables();
    if( tl1.count() not_eq tl2.count()) {
         qInfo() << "db comparison: table list count missmatch";
         ret =false;
    }
    for (auto table: tl1) {
        if( tl2.contains(table)){
            qInfo() << "common table: " << table;
            int rc1 =rowCount(table, "", db1);
            int rc2 =rowCount(table, "", db2);
            if( rc1 not_eq rc2) {
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

bool dbTablesHaveSameFields(const QString& table1, const QString& table2, const QSqlDatabase &db)
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
    if( rec1.count() not_eq rec2.count()) {
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

