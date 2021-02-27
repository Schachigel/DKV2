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

bool replace_TableContent(const QString srcTbl, const QString destTbl, const QSqlDatabase& db =QSqlDatabase::database())
{   LOG_CALL_W( srcTbl +qsl(", ") +destTbl);
    executeSql_wNoRecords( qsl("DELETE FROM ") + destTbl, QVector<QVariant>(), db);
    return copy_TableContent(srcTbl, destTbl, db);
}

bool copy_TableContent_byRecord( const QString& src, const QString& dest, const QSqlDatabase& db=QSqlDatabase::database())
{   LOG_CALL_W( src +qsl(", ") +dest);
    QString copyableFields;
    QSqlRecord rec =db.record(src);
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
        TableDataInserter tdi(dest, q.record());
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
bool copy_database( const QString& targetFileName, const QSqlDatabase& db/*=QSqlDatabase::database()*/, const dbstructure& dbs)
{
    LOG_CALL_W(targetFileName);
    autoRollbackTransaction transaction(db.connectionName());
    autoDetachDb ad(qsl("targetDb"), db.connectionName());

    if (!createNewDatabaseFile(targetFileName, dbs))
        return false;
    if (!ad.attachDb(targetFileName))
        return false;

    executeSql_wNoRecords(qsl("PRAGMA ") + ad.alias + qsl(".foreign_keys = OFF"), QVector<QVariant>(), db);

    QVector<dbtable> tables = dkdbstructur.getTables();
    for (auto& table : qAsConst(tables)) {
        if (!copy_TableContent(table.Name(), ad.alias + qsl(".") + table.Name(), db))
            return false;
    }
    if (!replace_TableContent(qsl("sqlite_sequence"), ad.alias +qsl(".sqlite_sequence"), db))
        return false;
//    // force views creation on next startup
//    QString sql{qsl("DELETE FROM %1.meta WHERE Name='dkv2.exe.Version'")};
//    executeSql_wNoRecords(sql.arg(ad.alias), QVector<QVariant>(), db);
    /////////// all done
    transaction.commit();
    /////////////////////

    return true;
}

/*
*  copy_database_mangled will create a 1 : 1 copy of the currently opened database to a new file
*  with all personal data replaced by random data
*/
bool copy_mangledCreditors(QSqlDatabase db =QSqlDatabase::database())
{   LOG_CALL;
    bool success = true;
    int recCount = 0;
    QSqlQuery q(db); // default database connection -> active database
    if( ! q.exec("SELECT * FROM Kreditoren")) {
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

bool copy_database_mangled(QString targetfn, QString source)
{
//    QString con {};
    dbCloser closer(qsl("copy_db"));
    QSqlDatabase db =QSqlDatabase::addDatabase("QSQLITE", closer.conName);
    db.setDatabaseName(source);
    if( ! db.open()) {
        qCritical() << "create_DB_copy could not open " << source;
        return false;
    } else {
        qInfo() << "opened " << source << " as db to copy";
    }
    return copy_database_mangled(targetfn, db);
}

bool copy_database_mangled(QString targetfn, QSqlDatabase dbToBeCopied)
{   LOG_CALL_W(targetfn);
    QString alias{qsl("targetDb")};
    autoRollbackTransaction trans( dbToBeCopied.connectionName());
    autoDetachDb ad( alias, dbToBeCopied.connectionName());
    if( ! createFileWithDatabaseStructure (targetfn))
        return false;
    // Attach the new file to the current connection
    if( ! ad.attachDb(targetfn))
        return false;

    QVector<dbtable> tables = dkdbstructur.getTables();
    for( auto& table : qAsConst(tables)) {
        if( table.Name() == qsl("Kreditoren")) {
            if( ! copy_mangledCreditors( dbToBeCopied))
                return false;
        }
        else if (table.Name() == qsl("BriefElemente")) {
            if( ! replace_TableContent(table.Name(), alias +qsl(".") +table.Name(), dbToBeCopied))
                return false;
        }
        else
            if( ! copy_TableContent(table.Name(), alias +qsl(".") +table.Name(), dbToBeCopied))
                return false;
    }
    // copy the values from sqlite_sequence, so that autoinc works the same in both databases
    if( ! replace_TableContent(qsl("sqlite_sequence"), alias +qsl(".sqlite_sequence")))
        return false;
    // force views creation on next startup
    executeSql_wNoRecords(qsl("DELETE FROM targetDb.meta WHERE Name='dkv2.exe.Version'"), QVector<QVariant>(), dbToBeCopied);
    trans.commit();
    return true;
}

/*
*  convert_database will create a copy of a given database file to a new file using
*  the current schema inserting the data given, leaving any new fields empty / to their default value
*/
bool convert_database_inplace( const QString& targetFilename, const QString& tempFileName, const dbstructure& dbs)
{   LOG_CALL_W(targetFilename);
    QString backupFileName =createPreConversionCopy(targetFilename, tempFileName);
    if( backupFileName.isEmpty()) {
        qCritical() << "Could not create backup copy - abort " << backupFileName << " of " << targetFilename;
        return false;
    }
    QString& sourceFileName =backupFileName;
    // create a new db file with the current database structure
    if( ! createNewDatabaseFile(targetFilename, dbs)) {
        qCritical() << "db creation faild for database conversion -> abort";
        return false;
    }
    // copy the data  - but if fields are missing: use only the available fields, leave the new fields to their default
    autoDb db(sourceFileName, qsl("convert"));
    // if foreign_keys are not enforced we can copy the tables in any order
    QSqlQuery enableRefInt("PRAGMA foreign_keys = OFF", db);

    autoRollbackTransaction transact(db.conName());
    autoDetachDb autodetatch( qsl("targetDb"), db.conName());
    autodetatch.attachDb(targetFilename);
    QSqlQuery enableRefInt2("PRAGMA " +autodetatch.alias +".foreign_keys = OFF", db);

    // there are tables with default content but w/o primIndex -> replace will not work
    // so they must be deleted first
    if( &dbs == &dkdbstructur) {
        QStringList tablesToBeDeleted {"Briefelemente"};
        for( auto& table : qAsConst(tablesToBeDeleted)) {
            QSqlQuery q("DELETE FROM " +autodetatch.alias +"." +table, db);
        }
    }

    QVector<dbtable> tables =dbs.getTables();
    for(auto& table : qAsConst(tables))
    {
        const int destFields = table.Fields().count();
        const int srcFields  = QSqlDatabase(db).record(table.Name()).count();
        if( destFields < srcFields) {
            qCritical() << "destianation Table misses fields " << table.Name();
            return false;
        } else if( destFields == srcFields) {
            if( ! copy_TableContent(table.Name(), autodetatch.alias +"."+table.Name(), db)) {
                qCritical() << "could not copy table while converting " << table.Name();
                return false;
            }
        } else if( destFields > srcFields) {
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
    dbConfig::writeVersion(db, autodetatch.alias);
    transact.commit();
    return true;
}
