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
    autoDb db(dbFileName, qsl("createDb"));
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
bool createFileWithDatabaseStructure (QString targetfn, const dbstructure& dbs)
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
    bool ret =dbs.createDb(newDb);
    return ret;
}

// database creation
bool createNewDatabaseFile(const QString& filename, const dbstructure& dbs/* =dkdbstructu*/)
{   LOG_CALL_W(qsl("filename: ") + filename);
    Q_ASSERT(!filename.isEmpty());
    dbgTimer timer( qsl("Db Creation Time"));

    // create file an schema
    if( ! createFileWithDatabaseStructure (filename, dbs)) {
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
    if( &dbs == &dkdbstructur)
        return fill_DkDbDefaultContent(db);
    else
        return true;
}

// database validation
bool hasAllTablesAndFields(QSqlDatabase db, const dbstructure& dbs /*=dkdbstructur*/)
{   LOG_CALL;
    for( auto& table : dbs.getTables()) {
        if( !verifyTable(table, db))
            return false;
    }
    qInfo() << db.databaseName() << " has all tables expected";
    return true;
}

bool validateDbSchema(const QString& filename, const dbstructure& dbs /*=dkdbstructur*/)
{
    LOG_CALL_W(filename);
    QString msg;
    if( filename == "")
        msg = "no filename";
    else if( !QFile::exists(filename))
        msg = "file not found";
    else {
        dbCloser closer{ qsl("validateDbSchema") };
        QSqlDatabase db = QSqlDatabase::addDatabase(qsl("QSQLITE"), closer.conName);
        db.setDatabaseName(filename);
        db.open();
        if (hasAllTablesAndFields(db, dbs)) {
            qInfo() << filename << " is a valid Database";
        }
    }
    if( msg.isEmpty())
        return true;
    qCritical() << msg;
    return false;
}


