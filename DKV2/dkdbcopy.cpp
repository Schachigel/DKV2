
#include "helpersql.h"
#include "helperfile.h"
#include "appconfig.h"
#include "tabledatainserter.h"
#include "dkdbcopy.h"

/*
*  locally used helper functions
*/
namespace
{
bool copy_TableContent( const QString& srcTbl, const QString& dstTbl, const QSqlDatabase& db=QSqlDatabase::database())
{
    LOG_CALL_W( srcTbl +qsl(", ") +dstTbl);
    // usually used from "table" to "targetScheema.table"
    QString sql(qsl("INSERT OR REPLACE INTO %1 SELECT * FROM %2"));
    return executeSql_wNoRecords(sql.arg(dstTbl, srcTbl), db);
}

bool replace_TableContent(const QString& srcTbl, const QString& destTbl, const QSqlDatabase& db =QSqlDatabase::database())
{
    LOG_CALL_W( srcTbl +qsl(", ") +destTbl);
    executeSql_wNoRecords( qsl("DELETE FROM ") + destTbl, db);
    return copy_TableContent(srcTbl, destTbl, db);
}

bool copy_TableContent_byRecord( const QString& srcTbl, const QString& dstTbl, const QSqlDatabase& db=QSqlDatabase::database())
{   LOG_CALL_W( srcTbl +qsl(", ") +dstTbl);
    // by copying "field by field" we can copy into tables that have extra fields
    QString copyableFields;
    QSqlRecord rec =db.record(srcTbl);
    for( int i =0; i < rec.count(); i++) {
        if( copyableFields.size()) copyableFields +=qsl(", ");
        copyableFields += rec.field(i).name();
    }
    QSqlQuery q(db); q.setForwardOnly(true);
    if( not q.prepare(qsl("SELECT * FROM ") + srcTbl)) {
        qCritical() << "Could not prepare query to enumerate table";
        return false;
    }
    if( not q.exec()) {
        qCritical() << "Could not enumerate source table";
        return false;
    }
    while(q.next()) {
        TableDataInserter tdi(dstTbl, q.record());
        if( tdi.InsertOrReplaceData(db) == SQLITE_invalidRowId) {
            qCritical() << "could not insert Date to converted db" << tdi.getRecord();
            return false;
        }
    }
    return true;
}
}
QString moveToPreConversionCopy( const QString& file)
{
    QFileInfo fi(file);
    QString newFile = getUniqueTempFilename(fi.absoluteFilePath (), qsl("preconversion"));
    Q_ASSERT( newFile.size() >0);
    QFile f(fi.absoluteFilePath ());
    if( f.rename( newFile)){
        RETURN_OK( newFile, QString(__FUNCTION__), file, newFile);
    }
    else {
        qCritical() << "file rename failed " << f.error() << ": " << f.errorString();
        Q_ASSERT( not "Faild to rename temporary file");
        return QString();
    }
}

/*
*  copy_database will create a 1 : 1 copy of "sourceFName"
*  to a new file "targetFName"
*/

bool copy_open_DkDatabase( const QString& targetFName)
{   LOG_CALL_W(targetFName);
    return executeSql_wNoRecords (qsl("VACUUM INTO '%1'").arg(targetFName));
}

bool depersonalize(QString connection)
{
    QVector<QString> updateSql ={
        {qsl("UPDATE Kreditoren SET Vorname = 'vorname_' || CAST(rowid as TEXT)")},
        {qsl("UPDATE Kreditoren SET Nachname = 'nachname_' || CAST(rowid as TEXT)")},
        {qsl("UPDATE Kreditoren SET Strasse = 'Strasse_' || CAST(rowid as TEXT)")},
        {qsl("UPDATE Kreditoren SET Plz = printf('%04d', abs(random()%10000))")},
        {qsl("UPDATE Kreditoren SET Stadt = 'Stadt_' || CAST(rowid as TEXT)")},
        {qsl("UPDATE Kreditoren SET Email = 'email@server.fun'")},
        {qsl("UPDATE Kreditoren SET Telefon = '01234567890'")},
        {qsl("UPDATE Kreditoren SET Land = ''")},
        {qsl("UPDATE Kreditoren SET Anmerkung = ''")},
        {qsl("UPDATE Kreditoren SET Kontakt = ''")},
        {qsl("UPDATE Kreditoren SET IBAN = ''")},
        {qsl("UPDATE Kreditoren SET BIC = ''")},
        {qsl("UPDATE Kreditoren SET Buchungskonto = ''")},
        {qsl("UPDATE Vertraege SET Kennung = printf('Vertrag_%04d', rowid)")},
        {qsl("UPDATE Vertraege SET Anmerkung = ''")},
        {qsl("UPDATE exVertraege SET Kennung = printf('Vertrag_%04d', rowid)")},
        {qsl("UPDATE exVertraege SET Anmerkung = ''")}
    };
    //QSqlQuery q(QSqlDatabase::database (connection));
    for( const auto &sql : qAsConst(updateSql)) {
        if( not executeSql_wNoRecords (sql, QSqlDatabase::database (connection)))
            RETURN_ERR(false, qsl("depersonilize sql failed"));
    }
    RETURN_OK( true, QString(__FUNCTION__));
}

bool copy_database_mangled(const QString& targetfn, const QSqlDatabase& db /*=QSqlDatabase::database()*/)
{
    if( QFile::exists(targetfn)) {
        backupFile(targetfn, qsl("db-bak"));
        QFile::remove (targetfn);
    }
    // vacuum copy db into memory
    dbCloser closer(qsl("inMemoryDbConnection"));
    QSqlDatabase inMemoryDb =QSqlDatabase::addDatabase ( dbTypeName, closer.conName);
    inMemoryDb.setConnectOptions (qsl("QSQLITE_OPEN_URI;QSQLITE_ENABLE_SHARED_CACHE"));
    QString inMemoryDbName {qsl("file::memory:")};
    inMemoryDb.setDatabaseName (inMemoryDbName);

    if( not inMemoryDb.open())
        qCritical() << "db open failed on in memory db " << inMemoryDb.lastError ();

    if( not executeSql_wNoRecords( qsl("VACUUM INTO '%1'").arg(inMemoryDbName), db))
        RETURN_ERR( false, qsl("Vacuum into in memory db failed"));

    if( not depersonalize(closer.conName)){
        qCritical() <<  qsl("failed to replace personal data");
        return false;
    }
    if( not executeSql_wNoRecords (qsl("VACUUM INTO '%1'").arg(targetfn), inMemoryDb))
        RETURN_ERR( false, qsl("Vacuum into disk db failed"));

    return true;
}

/*
*  convert_database will create a copy of a given database file to a new file using
*  the current schema inserting the data given, leaving any new fields empty / to their default value
*/
QString convert_database_inplace( const QString& targetFilename, const dbstructure& dbs)
{   LOG_CALL_W(targetFilename);
    QString backupFileName =moveToPreConversionCopy(targetFilename);
    if( backupFileName.isEmpty()) {
        qCritical() << "Could not create backup copy - abort " << backupFileName << " of " << targetFilename;
        return QString();
    }
    const QString& sourceFileName =backupFileName;
    // create a new db file with the current database structure
    if( not createNewDatabaseFileWDefaultContent(targetFilename, zs_30360, dbs)) {
        qCritical() << "db creation faild for database conversion -> abort";
        return QString();
    }
    // copy the data  - but if fields are missing: use only the available fields, leave the new fields to their default
    autoDb db(sourceFileName, qsl("convert"));
    // if foreign_keys are not enforced we can copy the tables in any order
    switchForeignKeyHandling(fkh_off, db);

    autoRollbackTransaction transact(db.conName());
    autoDetachDb autodetatch( qsl("targetDb"), db.conName());
    autodetatch.attachDb(targetFilename);
    switchForeignKeyHandling(fkh_off, autodetatch.alias(), db);

    // there are tables with default content but w/o primIndex -> replace will not work
    // so they must be deleted first
    if( &dbs == &dkdbstructur) {
        QStringList tablesToBeDeleted {"Briefelemente"};
        for( auto& table : qAsConst(tablesToBeDeleted)) {
            executeSql_wNoRecords(qsl("DELETE FROM ") +autodetatch.alias() +qsl(".") +table, db);
        }
    }

    QVector<dbtable> tables =dbs.getTables();
    for(auto& table : qAsConst(tables))
    {
        const int destFields = table.Fields().count();
        const int srcFields  = QSqlDatabase(db).record(table.Name()).count();
        if( destFields < srcFields) {
            qCritical() << "destianation Table misses fields " << table.Name();
            return QString();
        } else if( destFields == srcFields) {
            if( not copy_TableContent(table.Name(), autodetatch.alias() +"."+table.Name(), db)) {
                qCritical() << "could not copy table while converting " << table.Name();
                return QString();
            }
        } else if( destFields > srcFields) {
            if( not copy_TableContent_byRecord(table.Name(), autodetatch.alias() +"."+table.Name(), db)) {
                qCritical() << "could not copy table by record while converting " << table.Name();
                return QString();
            }
        }
    }
    // now we need to update sqlite_sequence, so that autoincrement index fields will be initialized correctly
    if( not replace_TableContent(qsl("sqlite_sequence"), autodetatch.alias() +qsl(".sqlite_sequence"), db)) {
            qCritical() << "could not update sqlite_sequence table";
            return QString();
    }
    dbConfig::write_DBVersion(db, autodetatch.alias());
    transact.commit();
    return backupFileName;
}
