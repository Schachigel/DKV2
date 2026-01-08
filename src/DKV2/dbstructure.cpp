#include "dbstructure.h"
#include "helper.h"
#include "helperfile.h"
#include "helpersql.h"
#include "dkdbhelper.h"

#include "appconfig.h"
#include "creditor.h"
#include "contract.h"
#include "booking.h"
#include "investment.h"


// dbstructure dkdbstructur;

dbstructure dbstructure::appendTable(const dbtable& t)
{
    for (auto& table: std::as_const(Tables)) {
        if( table.Name() == t.Name()) {
            qCritical() << "Versuch eine Tabelle wiederholt zur Datenbank hinzuzufügen";
            Q_ASSERT( not bool("redundent table in structure"));
        }
    }
    Tables.append(t);
    return *this;
}

dbtable dbstructure::operator[](const QString& name) const
{
    for( dbtable table : Tables) {
        if( table.Name() == name)
            return table;
    }
    qCritical() << "trying to access unknown table " << name;
    Q_ASSERT( not bool("access to unknown database table"));
    return dbtable();
}

bool dbstructure::createDb(const QString& dbFileName) const
{   LOG_CALL_W(dbFileName);
    autoDb db(dbFileName, qsl("createDb"));
    return createDb( db);
}

bool dbstructure::createDb(const QSqlDatabase& db) const
{   // LOG_CALL;
    switchForeignKeyHandling(fkh_on, db);
    for(dbtable& table :getTables()) {
        if( not ensureTable(table, db))
            RETURN_ERR(false, qsl("could not create table "), table.Name());
    } // for
    return true;
}

QString dbstructure::toString()
{
    QString ret;
    for (const dbtable& t : std::as_const(Tables)) {
        for (const dbfield& f : t.Fields() ) {
            ret += f.name() + ", " +f.tableName() +", " +f.metaType().name() +"\n";
        }
    }
    return ret;
}



void init_DKDBStruct()
{   LOG_CALL_W("Setting up internal database structures");
    if( dkdbstructur.getTables ().size ())
        return; // init only once
    // DB date -> Variant String
    // DB bool -> Variant int

    dkdbstructur.appendTable(creditor::getTableDef());
    dkdbstructur.appendTable(investment::getTableDef());
    dkdbstructur.appendTable(contract::getTableDef());
    dkdbstructur.appendTable(contract::getTableDef_deletedContracts());
    dkdbstructur.appendTable(booking::getTableDef());
    dkdbstructur.appendTable(booking::getTableDef_deletedBookings());
    dkdbstructur.appendTable(appConfig::getTableDef());
}
void reInit_DKDBStruct()
{
    dkdbstructur.clear();
    init_DKDBStruct();
}


// db creation for newDb and copy (w & w/o de-personalisation)
bool createFileWithDatabaseStructure (const QString& targetfn, const dbstructure& dbs/* =dkdbstructu*/)
{   LOG_CALL_W(targetfn);
    if( not moveToBackup(targetfn))
        return false;

    dbCloser closer(qsl("createDbFile"));

    QSqlDatabase newDb = QSqlDatabase::addDatabase(dbTypeName, closer.conName);
    newDb.setDatabaseName(targetfn);
    if( not newDb.open())
    RETURN_ERR(false, qsl("faild to open new database"));

    return dbs.createDb(newDb);
}

// database creation
bool createNewDatabaseFileWDefaultContent(const QString& filename, zinssusance zs /*=zs30360*/, const dbstructure& dbs/* =dkdbstructu*/)
{   LOG_CALL_W(qsl("filename: ") + filename);
    Q_ASSERT( filename.size());

    // create file an schema
    if( not createFileWithDatabaseStructure (filename, dbs))
        return false;

    // create content
    dbCloser closer{qsl("conCreateDb")};
    QSqlDatabase db = QSqlDatabase::addDatabase(dbTypeName, closer.conName);
    db.setDatabaseName(filename);

    if( not db.open())
        RETURN_ERR(false, qsl("DkDatenbankAnlegen failed in db.open"));
    if( &dbs == &dkdbstructur)
        return fill_DkDbDefaultContent(db, true, zs);
    else
        return true;
}

// database validation
bool hasAllTablesAndFields(const QSqlDatabase& db, const dbstructure& dbs /*=dkdbstructur*/)
{   LOG_CALL;
    for( auto& table : dbs.getTables()) {
        if( not verifyTable(table, db))
            return false;
    }
    qInfo() << db.databaseName() << " has all tables expected";
    return true;
}

bool validateDbSchema(const QString& filename, const dbstructure& dbs /*=dkdbstructur*/)
{   LOG_CALL_W(filename);
    QString msg;
    if( filename.isEmpty ())
        msg = qsl("no filename");
    else if( not QFile::exists(filename))
        msg = qsl("file not found");
    else {
        dbCloser closer{ qsl("validateDbSchema") };
        QSqlDatabase db = QSqlDatabase::addDatabase(dbTypeName, closer.conName);
        db.setDatabaseName(filename);
        if( db.open()) {
            if (hasAllTablesAndFields(db, dbs)) {
                qInfo() << filename << " is a valid Database";
                ////
                return true;
                ////
            } else {
                msg =qsl("db does not have all tables or fields");
            }
        } else {
            msg =qsl("db open failed");
        }
    }
    if( msg.isEmpty())
        Q_ASSERT("one should not reach this code");
    qCritical() << msg;
    return false;
}


