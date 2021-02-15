#include <QSqlDatabase>

#include "creditor.h"
#include "contract.h"
#include "booking.h"
#include "letterTemplate.h"


#include "helper.h"
#include "helpersql.h"
#include "dbstructure.h"

dbstructure dkdbstructur;

dbstructure dbstructure::appendTable(dbtable t)
{   // LOG_CALL;
//    qInfo() << "adding table to db structure " << t.name;
    for (auto& table: qAsConst(Tables)) {
        if( table.Name() == t.Name()) {
            qCritical() << "Versuch eine Tabelle wiederholt zur Datenbank hinzuzufügen";
            Q_ASSERT(!bool("redundent table in structure"));
        }
    }
    Tables.append(t);
    return *this;
}

dbtable dbstructure::operator[](const QString& name) const
{   // LOG_CALL;
    // qDebug() << "accessing db table " << name;
    for( dbtable table : Tables)
    {
        if( table.Name() == name)
            return table;
    }
    qCritical() << "trying to access unknown table " << name;
    Q_ASSERT(!bool("access to unknown database table"));
    return dbtable();
}

bool dbstructure::createDb(QString dbFileName) const
{   LOG_CALL_W(dbFileName);
//    QString connection{qsl("createDb")};
    dbCloser closer(qsl("createDb"));

    QSqlDatabase db =QSqlDatabase::addDatabase(qsl("QSQLITE"), closer.conName);
    db.setDatabaseName(dbFileName);
    if( ! db.open()){
        qCritical() << "db creation failed " << dbFileName;
        return false;
    }
    return createDb( db);
}

bool dbstructure::createDb(QSqlDatabase db) const
{   LOG_CALL;
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON", db);
    for(dbtable& table :getTables()) {
        if(!ensureTable(table, db)) {
            qCritical() << "could not create table " << table.name;
            return false;
        }
    }
    return true;
}

void init_DKDBStruct()
{   LOG_CALL_W("Setting up internal database structures");
    static bool done = false;
    if( done) return; // for tests
    // DB date -> Variant String
    // DB bool -> Variant int

    dkdbstructur.appendTable(creditor::getTableDef());

    dkdbstructur.appendTable(contract::getTableDef());
    dkdbstructur.appendTable(contract::getTableDef_deletedContracts());

    dkdbstructur.appendTable(booking::getTableDef());
    dkdbstructur.appendTable(booking::getTableDef_deletedBookings());

    dbtable meta("Meta");
    meta.append(dbfield("Name", QVariant::String).setPrimaryKey());
    meta.append(dbfield("Wert", QVariant::String).setNotNull());
    dkdbstructur.appendTable(meta);

    dkdbstructur.appendTable(letterTemplate::getTableDef_letterTypes());
    dkdbstructur.appendTable(letterTemplate::getTabelDef_elementTypes());
    dkdbstructur.appendTable(letterTemplate::getTableDef_letterElements());

    done = true;
}

// db creation for newDb and copy (w & w/o de-personalisation)
bool createFileWithDkDatabaseStructure (QString targetfn)
{   LOG_CALL_W(targetfn);
    if( ! moveToBackup(targetfn)) {
        return false;
    }
    dbCloser closer(qsl("createDbFile"));

    QSqlDatabase newDb = QSqlDatabase::addDatabase("QSQLITE", closer.conName);
    newDb.setDatabaseName(targetfn);
    if( !newDb.open()) {
        qDebug() << "faild to open new database";
        return false;
    }
    bool ret =dkdbstructur.createDb(newDb);
    return ret;
}

// database creation
bool createNew_DKDatabaseFile(const QString& filename) /*in the default connection*/
{   LOG_CALL_W(qsl("filename: ") + filename);
    Q_ASSERT(!filename.isEmpty());
    dbgTimer timer( qsl("Db Creation Time"));

    // create file an schema
    if( ! createFileWithDkDatabaseStructure (filename)) {
        return false;
    }
    // create content
    dbCloser closer{qsl("conCreateDb")};
    QSqlDatabase db = QSqlDatabase::addDatabase(qsl("QSQLITE"), closer.conName);
    db.setDatabaseName(filename);

    if( !db.open()) {
        qCritical() << "DkDatenbankAnlegen failed in db.open";
        return false;
    }
    return fill_dbDefaultContent(db);
}

// database validation
bool hasAllTablesAndFields(QSqlDatabase db)
{   LOG_CALL;
    for( auto& table : dkdbstructur.getTables()) {
        if( !verifyTable(table, db))
            return false;
    }
    qInfo() << db.databaseName() << " has all tables expected";
    return true;
}

bool validDbSchema(QSqlDatabase db)
{
    LOG_CALL;
    {QSqlQuery enableRefInt(db);
    enableRefInt.exec("PRAGMA foreign_keys = ON");}
    if( ! hasAllTablesAndFields(db))
        return false;
    qInfo() << db.databaseName() << " is a valid dk database";
    return true;
}

bool validDbSchema(const QString& filename)
{
    LOG_CALL_W(filename);
    QString msg;
    if( filename == "") msg = "no filename";
    else if( !QFile::exists(filename)) msg = "file not found";
    else {
        if( convertToNewSchemaIfNeeded(filename)) {

        } else {
            qCritical() << "conversion failed";
        }
    }
    if( msg.isEmpty())
        return true;
    qCritical() << msg;
    return false;
}


