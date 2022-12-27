
#include "helpersql.h"
#include "helperfin.h"
#include "helperfile.h"
#include "appconfig.h"
#include "tabledatainserter.h"
#include "dkdbcopy.h"
#include "creditor.h"

/*
*  locally used helper functions
*/
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

bool copycreate_views(const QSqlDatabase& db, const QString& alias)
{   LOG_CALL;
     QSqlQuery q(db); q.setForwardOnly(true);
     QString sql {qsl("SELECT name, sql FROM %1.sqlite_master WHERE type='view'").arg(alias)};
     if( not q.exec(sql)) {
         qCritical() << "could not read views from database";
         return false;
     }
     while( q.next()) {
         QString viewSql  = q.record().value(qsl("sql")).toString(); //.replace(qsl("\n"), qsl(" "));
         if( not executeSql_wNoRecords(viewSql, db)) {
             qCritical() << "could not create view using " << viewSql;
             return false;
         }
     }
     return true;
}

/*
*  copy_database will create a 1 : 1 copy of "sourceFName"
*  to a new file "targetFName"
*/
bool vacuum_copy_database( const QString& targetFName)
{   LOG_CALL_W(targetFName);
    return executeSql_wNoRecords (qsl("VACUUM INTO '%1'").arg(targetFName));
}

bool copy_open_DkDatabase( const QString& targetFName)
{
    return vacuum_copy_database( targetFName);
}


/*
*  copy_database_mangled will create a 1 : 1 copy of the currently opened database to a new file
*  with all personal data replaced by random data
*/
bool copy_mangledCreditors(const QSqlDatabase& db =QSqlDatabase::database())
{   LOG_CALL;
    int recCount = 0;
    QSqlQuery q(db); // default database connection -> active database
    q.setForwardOnly(true);
    if( not q.exec(qsl("SELECT * FROM Kreditoren")))
        RETURN_OK(false, qsl( "no data returned from creditor table"));

    TableDataInserter tdi(dkdbstructur[qsl("Kreditoren")]);
    tdi.overrideTablename(qsl("targetDb.Kreditoren"));
    while( q.next()) {
        recCount++;
        QSqlRecord rec = q.record();
        qInfo() << "de-Pers. Copy: working on Record #" << rec;
        tdi.setValue(creditor::fnId, rec.value(creditor::fnId));
        tdi.setValue(creditor::fnVorname,  QVariant(creditor::fnVorname + i2s(recCount)));
        tdi.setValue(creditor::fnNachname, QVariant(creditor::fnNachname + i2s(recCount)));
        tdi.setValue(creditor::fnStrasse, creditor::fnStrasse);
        tdi.setValue(creditor::fnPlz, qsl("D-xxxxx"));
        tdi.setValue(creditor::fnStadt, qsl("Stadt"));
        tdi.setValue(creditor::fnTel, "");
        tdi.setValue(creditor::fnKontakt, "");
        tdi.setValue(creditor::fnIBAN, "");
        tdi.setValue(creditor::fnBIC, "");
        tdi.setValue(creditor::fnBuchungskonto, "");

        if( tdi.InsertData_noAuto() == SQLITE_invalidRowId)
            RETURN_ERR(false, qsl("Error inserting Data into deperso.Copy Table"), q.lastError ().text ());
    }
    return true;
}

bool copy_database_mangled(const QString& targetfn, const QString& source)
{
//    QString con {};
    dbCloser closer(qsl("copy_db"));
    QSqlDatabase db =QSqlDatabase::addDatabase(dbTypeName, closer.conName);
    db.setDatabaseName(source);
    if( not db.open()) {
        qCritical() << "create_DB_copy could not open " << source;
        return false;
    } else {
        qInfo() << "opened " << source << " as db to copy";
    }
    return copy_database_mangled(targetfn, db);
}

bool copy_database_mangled(const QString& targetfn, const QSqlDatabase& dbToBeCopied)
{   LOG_CALL_W(targetfn);
    QString alias{qsl("targetDb")};
    autoRollbackTransaction trans( dbToBeCopied.connectionName());
    autoDetachDb ad( alias, dbToBeCopied.connectionName());
    if( not createFileWithDatabaseStructure (targetfn))
        return false;
    // Attach the new file to the current connection
    if( not ad.attachDb(targetfn))
        return false;

    QVector<dbtable> tables = dkdbstructur.getTables();
    for( auto& table : qAsConst(tables)) {
        if( table.Name() == qsl("Kreditoren")) {
            if( not copy_mangledCreditors( dbToBeCopied))
                return false;
        }
        else if (table.Name() == qsl("BriefElemente")) {
            if( not replace_TableContent(table.Name(), alias +qsl(".") +table.Name(), dbToBeCopied))
                return false;
        }
        else
            if( not copy_TableContent(table.Name(), alias +qsl(".") +table.Name(), dbToBeCopied))
                return false;
    }
    // copy the values from sqlite_sequence, so that autoinc works the same in both databases
    if( not replace_TableContent(qsl("sqlite_sequence"), alias +qsl(".sqlite_sequence")))
        return false;
    // force views creation on next startup
    executeSql_wNoRecords(qsl("DELETE FROM %1.meta WHERE Name='dkv2.exe.Version'").arg(alias), dbToBeCopied);
    trans.commit();
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
