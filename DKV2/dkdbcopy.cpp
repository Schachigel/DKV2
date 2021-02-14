#include <QFileInfo>
#include <QTemporaryFile>

#include "tabledatainserter.h"
#include "helperfile.h"
#include "dkdbhelper.h"
#include "dkdbcopy.h"

/*
*  locally used helper functions
*/
QString createPreConversionCopy( const QString& file, const QString& fixedTempFileName)
{
    QFileInfo fi(file);
    QString fullFile =fi.canonicalFilePath();
    QString newFile =fullFile +qsl(".preconversion");
    if( fixedTempFileName.isEmpty())
        newFile =getUniqueTempFilename(fullFile +qsl(".preconversion"));
    else
        newFile =fixedTempFileName;
    QFile f(fullFile);
    if( f.rename( newFile))
        return newFile;
    else {
        qCritical() << "file rename failed " << f.error() << ": " << f.errorString();
        Q_ASSERT(!"Faild to rename temporary file");
        return QString();
    }
}

bool copy_TableContent( const QString src, const QString dest, const QSqlDatabase db=QSqlDatabase::database())
{   LOG_CALL_W( src +qsl(", ") +dest);
    QString sql(qsl("INSERT OR REPLACE INTO %1 SELECT * FROM %2"));
    return executeSql_wNoRecords(sql.arg(dest).arg(src), QVector<QVariant>(), db);
}

bool replace_TableContent(const QString src, const QString dest, const QSqlDatabase& db =QSqlDatabase::database())
{   LOG_CALL_W( src +qsl(", ") +dest);
    executeSql_wNoRecords( qsl("DELETE FROM ") + dest, QVector<QVariant>(), db);
    return copy_TableContent(src, dest, db);
}

bool copy_TableContent_byRecord( const QString& src, const QString& dest, const QSqlDatabase& db=QSqlDatabase::database())
{   LOG_CALL_W( src +qsl(", ") +dest);
    QString copyableFields;
    QSqlRecord rec =db.record(dest);
    for( int i =0; i < rec.count(); i++) {
        if( ! copyableFields.isEmpty()) copyableFields +=qsl(", ");
        copyableFields += rec.field(i).name();
    }
    QSqlQuery q (db);
    q.prepare(qsl("SELECT * FROM ") + src);
    if( ! q.exec()) {
        qCritical() << "Could not enumerate source table";
        return false;
    }
    QString sql(qsl("INSERT OR REPLACE INTO %1 (%2) VALUES (%3)"));
    while(q.next()) {
        TableDataInserter tdi(src, q.record());
        if( tdi.InsertOrReplaceData(db) == -1) {
            qCritical() << "could not insert Date to converted db";
            return false;
        }
    }
    return true;
}

/*
*  copy_database will create a 1 : 1 copy of the currently opened database to a new file
*/
bool copy_database( const QString& /*targetFileName*/, const QSqlDatabase& /*sourceDb*/, const QSqlDatabase& db=QSqlDatabase::database())
{
    return true;
}

/*
*  copy_database_anonymous will create a 1 : 1 copy of the currently opened database to a new file
*  with all personal data replaced by random data
*/
bool copy_database_anonymous( QString /*targetFileName*/, QSqlDatabase /*sourceDb */)
{
    return true;
}

/*
*  convert_database will create a copy of a given database file to a new file using
*  the current schema inserting the data given, leaving any new fields empty / to their default value
*/
bool convert_database_inplace( const QString& targetFilename, const QString& tempFileName)
{   LOG_CALL_W(targetFilename);
    QString backupFileName =createPreConversionCopy(targetFilename, tempFileName);
    if( backupFileName.isEmpty()) {
        qCritical() << "Could not create backup copy - abort " << backupFileName << " of " << targetFilename;
        return false;
    }
    QString& sourceFileName =backupFileName;
    // create a new db file with the current database structure
    if( ! createNewEmpty_DKDatabaseFile(targetFilename)) {
        qCritical() << "db creation faild for database conversion -> abort";
        return false;
    }
    // copy the data  - but if fields are missing: use only the available fields, leave the new fields to their default
    dbCloser closer(qsl("convert"));
    QSqlDatabase db =QSqlDatabase::addDatabase(qsl("QSQLITE"), closer.conName);
    db.setDatabaseName(sourceFileName);
    db.open();
    // if foreign_keys are not enforced we can copy the tables in any order
    QSqlQuery enableRefInt("PRAGMA foreign_keys = OFF", db);

    autoRollbackTransaction transact(closer.conName);
    autoDetachDb autodetatch( qsl("targetDb"), closer.conName);
    autodetatch.attachDb(targetFilename);
    QSqlQuery enableRefInt2("PRAGMA " +autodetatch.alias +".foreign_keys = OFF", db);

    // there are tables with default content but w/o primIndex -> replace will not work
    // so they must be deleted first
    QStringList tablesToBeDeleted {"Briefelemente"};
    for( auto& table : qAsConst(tablesToBeDeleted)) {
        QSqlQuery q("DELETE FROM " +autodetatch.alias +"." +table, db);
    }

    QVector<dbtable> tables =dkdbstructur.getTables();
    for(auto& table : qAsConst(tables))
    {
        const int destFields = table.Fields().count();
        const int srcFields  = db.record(table.Name()).count();
        if( destFields < srcFields) {
            qCritical() << "destianation Table misses fields " << table.Name();
            return false;
        } else if( destFields == srcFields) {
            if( ! copy_TableContent(table.Name(), autodetatch.alias +"."+table.Name(), db)) {
                qCritical() << "could not copy table while converting " << table.Name();
                return false;
            }
        } else if( destFields < srcFields) {
            if( ! copy_TableContent_byRecord(table.Name(), autodetatch.alias +"."+table.Name(), db)) {
                qCritical() << "could not copy table by record while converting " << table.Name();
                return false;
            }
        }
    }
    // now we need to update sqlite_sequence, so that autoincrement index fields will be initialized correctly
    if( ! replace_TableContent(qsl("sqlite_sequence"), autodetatch.alias +qsl(".sqlite_sequence"), db)) {
            qCritical() << "could not update sqlite_sequence table";
            return false;
    }
    transact.commit();
    return true;
}
