#include "testhelper.h"

#include <QTest>
#include "../DKV2/helpersql.h"
#include "../DKV2/dkdbhelper.h"
#include "../DKV2/creditor.h"
#include "../DKV2/contract.h"

void getRidOfFile(QString filename)
{
    if( QFile::exists(filename))
        QFile::remove(filename);
    QVERIFY(not QFile::exists(filename));
}

void initTestDkDb_InMemory()
{
    reInit_DKDBStruct();
    openDefaultDbConnection_InMemory();
    QVERIFY(dkdbstructur.createDb());
}

void initTestDkDb()
{
    reInit_DKDBStruct();
    if (QFile::exists(testDbFilename))
        QFile::remove(testDbFilename);
    if (QFile::exists(testDbFilename))
        QFAIL("test db still in use");
    openDefaultDbConnection();
    QVERIFY(dkdbstructur.createDb());
    QVERIFY2( QFile::exists(testDbFilename), "create database failed." );
}

void createTestDkDb_wData()
{
    initTestDkDb();
    QVERIFY(fill_DkDbDefaultContent());
}

void createTestDkDbTemplate()
{
    createTestDkDb_wData();
    closeAllDatabaseConnections();
    QFile::copy(testDbFilename, testTemplateDb);
}
void cleanupTestDkDbTemplate()
{
    getRidOfFile(testDbFilename);
    getRidOfFile(testTemplateDb);
}
void initTestDkDbFromTemplate()
{
    getRidOfFile(testDbFilename);
    QFile::copy(testTemplateDb, testDbFilename);
    openDefaultDbConnection();
    switchForeignKeyHandling(fkh_on);
}
void createTestDb_withRandomData()
{   LOG_CALL;
    dbgTimer t(qsl("createTestDb_wRandomData"));
    createTestDkDb_wData();
    saveRandomCreditors(10);
    saveRandomContracts(8);
    activateRandomContracts(100 /* % */);
}
void cleanupTestDb_InMemory()
{   LOG_CALL;
    closeAllDatabaseConnections();
}
void cleanupTestDkDb()
{   LOG_CALL;
    closeAllDatabaseConnections();
    if (QFile::exists(testDbFilename))
        QFile::remove(testDbFilename);

    QVERIFY2( (QFile::exists(testDbFilename) == false), "destroy database failed." );
}
void openDefaultDatabaseConnection(QString name, QString options =QString())
{
    QSqlDatabase::addDatabase(dbTypeName);
    QSqlDatabase db =QSqlDatabase::database();
    db.setDatabaseName(name);
    db.setConnectOptions(options);
    db.open();

}
void openDefaultDbConnection_InMemory()
{
    openDefaultDatabaseConnection(qsl(":memory:"));
}
void openDefaultDbConnection(QString file /*testDbFilename*/)
{
    openDefaultDatabaseConnection(file, ("QSQLITE_OPEN_URI;QSQLITE_ENABLE_SHARED_CACHE"));
}
void closeDefaultDbConnection( )
{
    QSqlDatabase::database().close();
    //QSqlDatabase::database().connectionName () is no longer available
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    qInfo() << "connections: " << QSqlDatabase::connectionNames ();
    Q_ASSERT( QSqlDatabase::connectionNames ().isEmpty ());
}

void createEmptyFile(const QString& path)
{
    QFileInfo fi(path);
    QDir d(fi.absolutePath());
    d.mkpath(".");
    QFile f(fi.absoluteFilePath());
    if(f.open(QIODevice::WriteOnly))
        f.close();
    else
        qCritical() << "failed to create empty file";
}

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
    if( not open) qDebug() << db1.lastError();
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
    for (const auto &table: std::as_const(tl1)) {
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
    for (const auto &table: std::as_const(tl2)) {
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

// int doAnnualSettlementAllContracts (int year)
// {   LOG_CALL_W(i2s(year));
//     QVector<QVariant> ids =executeSingleColumnSql (dkdbstructur["Vertraege"].Fields ()[0]);
//     int executedSettlements =0;
//     for( const auto& id : std::as_const(ids)) {
//         contract c(id.toLongLong ());
//         if( year == c.annualSettlement (year)){
//             executedSettlements++;
//             qInfo().noquote ().nospace ()<< c.toString ();
//         }
//     }
//     return executedSettlements;
// }

enum DB_COMPARE_MODE
{
    compareTables,
    compareFields,
    compareData
};

void dbgDumpDatabase(const QString& testname)
{   LOG_CALL;
    QString filename { qsl("%1.db").arg(testname).replace (':', '#')};
    filename.replace('<', '#');
    filename.replace('>', '#');

    QFile filePath{filename};
    getRidOfFile(filePath.fileName());

    qInfo() << "removing file for replacement: " << filePath.fileName();
    QVERIFY( not filePath.exists());

    QSqlQuery q;
    q.prepare(qsl("VACUUM INTO '%1'").arg(filePath.fileName()));
    if( !q.exec())
        qDebug() << "Failed vacuum SQL " << q.lastError();

    QVERIFY( filePath.exists());
}

