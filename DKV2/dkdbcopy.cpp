#include <QFileInfo>
#include <QTemporaryFile>

#include "tabledatainserter.h"
#include "helperfile.h"
#include "appconfig.h"
#include "dkdbhelper.h"
#include "dkdbcopy.h"

/*
*  locally used helper functions
*/
QString createPreConversionCopy( const QString& file, const QString& fixedTempFileName)
{
    QFileInfo fi(file);
    QString fullFile =fi.canonicalFilePath();
    QString newFile = fixedTempFileName.isEmpty() ?
        getUniqueTempFilename(fullFile +qsl(".preconversion")) :
        fixedTempFileName;
    QFile f(fullFile);
    if( f.rename( newFile))
        return newFile;
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
    return executeSql_wNoRecords(sql.arg(dstTbl).arg(srcTbl), db);
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
        if( not copyableFields.isEmpty()) copyableFields +=qsl(", ");
        copyableFields += rec.field(i).name();
    }
    QSqlQuery q (db);
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
        if( tdi.InsertOrReplaceData(db) == -1) {
            qCritical() << "could not insert Date to converted db" << tdi.getRecord();
            return false;
        }
    }
    return true;
}

bool copycreate_views(const QSqlDatabase& db, const QString& alias)
{   LOG_CALL;
     QSqlQuery q(db);
     QString sql {qsl("SELECT name, sql FROM %1.sqlite_master WHERE type='view'").arg(alias)};
     if( not q.exec(sql)) {
         qCritical() << "query execute failed";
         return false;
     }
     while( q.next()) {
         QString name = q.record().value(qsl("name")).toString();
         QString viewSql  = q.record().value(qsl("sql")).toString(); //.replace(qsl("\n"), qsl(" "));

         //sql =sql.replace(name , alias + qsl(".") +name);
         if( not executeSql_wNoRecords(viewSql, db)) {
             return false;
         }
     }
     return true;
}

/*
*  copy_database will create a 1 : 1 copy of the currently opened database to a new file
*/
bool copy_database( const QString& sourceFName,
                    const QString& targetFName,
                    const dbstructure& targetDbStructure/*=dkdbstructur*/)
{
    qInfo() << "copy_database(" << sourceFName << ", " << targetFName << ")";

    if( not createFileWithDatabaseStructure(targetFName))
        return false;

    autoDb autoTarget(targetFName, qsl("copyDbTarget"));
    autoRollbackTransaction transaction(autoTarget.db.connectionName());
    autoDetachDb ad(qsl("SourceDb"), autoTarget.conName());
    if ( not ad.attachDb(sourceFName))
        return false;

    switchForeignKeyHandling(autoTarget.db, false);
    switchForeignKeyHandling(autoTarget.db, ad.alias(), false);

    QVector<dbtable> tables = targetDbStructure.getTables();
    for (auto& table : qAsConst(tables)) {
        if ( not replace_TableContent(ad.alias() + qsl(".") + table.Name(), table.Name(), autoTarget.db))
            return false;
    }
    if ( not replace_TableContent(ad.alias() +qsl(".sqlite_sequence"), qsl("sqlite_sequence"), autoTarget.db))
        return false;
    if( !copycreate_views(autoTarget.db, ad.alias()))
        return false;
    // if we had indices which do not come from table creation, they should also be copied
    /////////// all done
    transaction.commit();
    /////////////////////
    return true;
}

//bool copy_dkdb_database( const QString& sourceFName,
//                         const QString& targetFName)
//{
//    copy_database(sourceFName, targetFName);
//    // force views creation on next startup
//    QString sql{qsl("DELETE FROM meta WHERE Name='dkv2.exe.Version'")};
//    autoDb target(targetFName, qsl("copy_dkdb"));
//    return executeSql_wNoRecords(qsl("INSERT INTO sqlite_master FROM "), target.db);
//}


/*
*  copy_database_mangled will create a 1 : 1 copy of the currently opened database to a new file
*  with all personal data replaced by random data
*/
bool copy_mangledCreditors(const QSqlDatabase& db =QSqlDatabase::database())
{   LOG_CALL;
    bool success = true;
    int recCount = 0;
    QSqlQuery q(db); // default database connection -> active database
    if( not q.exec("SELECT * FROM Kreditoren")) {
        qInfo() << "no data returned from creditor table";
        return false;
    }
    TableDataInserter tdi(dkdbstructur["Kreditoren"]);
    tdi.overrideTablename(qsl("targetDb.Kreditoren"));
    while( q.next()) {
        recCount++;
        QSqlRecord rec = q.record();
        qDebug() << "de-Pers. Copy: working on Record #" << rec;
        QString vn {qsl("Vorname")}, nn {qsl("Nachname")};
        tdi.setValue(qsl("id"), rec.value(qsl("id")));
        tdi.setValue(qsl("Vorname"),  QVariant(vn + QString::number(recCount)));
        tdi.setValue(qsl("Nachname"), QVariant(nn + QString::number(recCount)));
        tdi.setValue("Strasse", QString("Strasse"));
        tdi.setValue("Plz", QString("D-xxxxx"));
        tdi.setValue("Stadt", QString("Stadt"));

        if( tdi.InsertData_noAuto() == -1) {
            qDebug() << "Error inserting Data into deperso.Copy Table" << q.lastError() << Qt::endl << q.record();
            success = false;
            break;
        }
    }
    return success;
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
QString convert_database_inplace( const QString& targetFilename, const QString& tempFileName, const dbstructure& dbs)
{   LOG_CALL_W(targetFilename);
    QString backupFileName =createPreConversionCopy(targetFilename, tempFileName);
    if( backupFileName.isEmpty()) {
        qCritical() << "Could not create backup copy - abort " << backupFileName << " of " << targetFilename;
        return QString();
    }
    const QString& sourceFileName =backupFileName;
    // create a new db file with the current database structure
    if( not createNewDatabaseFileWDefaultContent(targetFilename, dbs)) {
        qCritical() << "db creation faild for database conversion -> abort";
        return QString();
    }
    // copy the data  - but if fields are missing: use only the available fields, leave the new fields to their default
    autoDb db(sourceFileName, qsl("convert"));
    // if foreign_keys are not enforced we can copy the tables in any order
    QSqlQuery enableRefInt("PRAGMA foreign_keys = OFF", db);

    autoRollbackTransaction transact(db.conName());
    autoDetachDb autodetatch( qsl("targetDb"), db.conName());
    autodetatch.attachDb(targetFilename);
    QSqlQuery enableRefInt2("PRAGMA " +autodetatch.alias() +".foreign_keys = OFF", db);

    // there are tables with default content but w/o primIndex -> replace will not work
    // so they must be deleted first
    if( &dbs == &dkdbstructur) {
        QStringList tablesToBeDeleted {"Briefelemente"};
        for( auto& table : qAsConst(tablesToBeDeleted)) {
            QSqlQuery q("DELETE FROM " +autodetatch.alias() +"." +table, db);
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
