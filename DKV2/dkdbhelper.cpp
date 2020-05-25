#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVector>

#include "helper.h"
#include "appconfig.h"
#include "csvwriter.h"
#include "creditor.h"
#include "contract.h"
#include "booking.h"
#include "dbstructure.h"

const double CURRENT_DB_VERSION {2.0};
const QString DB_VERSION {"Version"};

class dbCloser
{   // for use on the stack only
public:
    dbCloser() : Db (nullptr){} // create 'closer' before the database
    ~dbCloser(){if( !Db) return; Db->close(); closeDatabaseConnection(Db->connectionName());}
    void set(QSqlDatabase* p){ Db=p;}
private:
    QSqlDatabase* Db;
};

dbstructure dkdbstructur;
void init_DKDBStruct()
{   LOG_CALL_W("Setting up internal database structures");
    static bool done = false;
    if( done) return; // for tests
    // DB date -> Variant String
    // DB bool -> Variant int

    dkdbstructur.appendTable(creditor::getTableDef());
    dkdbstructur.appendTable(contract::getTableDef());
    dkdbstructur.appendTable(booking::getTableDef());

    dbtable meta("Meta");
    meta.append(dbfield("Name", QVariant::String).setNotNull());
    meta.append(dbfield("Wert", QVariant::String).setNotNull());
    dkdbstructur.appendTable(meta);

    dbtable letters("Briefvorlagen");
    letters.append(dbfield("templateId",    QVariant::Int).setNotNull());
    letters.append(dbfield("EigenschaftId", QVariant::Int).setNotNull());
    letters.append(dbfield("Wert",          QVariant::String).setNotNull());
    dkdbstructur.appendTable(letters);
    done = true;
}

// db config info in 'meta' table
void initMetaInfo( const QString& name, const QString& initialValue)
{   LOG_CALL;
    QVariant value= executeSingleValueSql("SELECT WERT FROM Meta WHERE Name='" + name +"'");
    if( value.type() == QVariant::Type::Invalid)
        setMetaInfo(name, initialValue);
}
void initNumMetaInfo( const QString& name, const double& newValue)
{   LOG_CALL;
    QVariant value= executeSingleValueSql("SELECT WERT FROM Meta WHERE Name='" + name +"'");
    if( value.type() == QVariant::Type::Invalid)
        setNumMetaInfo(name, newValue);
}
QString getMetaInfo(const QString& name)
{   LOG_CALL_W(name);
    QVariant value= executeSingleValueSql("SELECT WERT FROM Meta WHERE Name='" + name +"'").toString();
    if( value.type() == QVariant::Type::Invalid) {
        qInfo() << "read empty property " << name << "; defaulted to empty string";
        return "";
    }
    qInfo() << "Property " << name << " : " << value;
    return value.toString();
}
double getNumMetaInfo(const QString& name, QSqlDatabase db)
{   LOG_CALL_W(name);

    QVariant value= executeSingleValueSql("SELECT WERT FROM Meta WHERE Name='" + name +"'", db);
    if( value.type() == QVariant::Type::Invalid) {
        qInfo() << "getNumProperty read empty property " << name << " defaulted to 0.";
        return 0.;
    }
    qInfo() << "Property " << name << " : " << value.toDouble();
    return value.toDouble();
}
void setMetaInfo(const QString& name, const QString& Wert)
{   LOG_CALL_W(name);
    QSqlQuery q;
    q.prepare("INSERT OR REPLACE INTO Meta (Name, Wert) VALUES ('" + name + "', '" + Wert +"')");
    if( !q.exec())
        qCritical() << "Failed to insert Meta information " << q.lastError() << endl << q.lastQuery();
}
void setNumMetaInfo(const QString& name, const double Wert)
{   LOG_CALL_W(name);
    QSqlQuery q;
    if( !q.exec("INSERT OR REPLACE INTO Meta (Name, Wert) "
                "VALUES ('" + name + "', '" + QString::number(Wert) +"')"))
        qCritical() << "Failed to insert Meta information " << q.lastError() << endl << q.lastQuery();
}

// database creation
bool create_DK_databaseFile(const QString& filename) /*in the default connection*/
{   //LOG_CALL_W("filename: " + filename);
    Q_ASSERT(!filename.isEmpty());
    dbgTimer timer( QString(__func__) + QString(" (") + filename + QString(")"));
    if( QFile(filename).exists()) {
        backupFile(filename, "db-bak");
        QFile(filename).remove();
        if( QFile(filename).exists()) {
            qCritical() << "file to be replaced can not be deleted";
            return false;
        }
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filename);

    if( !db.open()) {
        qCritical() << "DkDatenbankAnlegen failed in db.open";
        return false;
    }
    return create_DK_TablesAndContent(db);
}
// initial database tables and content
void insert_UniqueDbProperties()
{   LOG_CALL;
    initNumMetaInfo(DB_VERSION, CURRENT_DB_VERSION);
    initMetaInfo("ProjektInitialen", "ESP");
    QRandomGenerator *rand = QRandomGenerator::system();
    initMetaInfo("IdOffset", QString::number(rand->bounded(10000,20000)));
}
void insert_defaultGmbHData( )
{   LOG_CALL;
    initMetaInfo("gmbh.address1", "Esperanza Franklin GmbH");
    initMetaInfo("gmbh.address2", "");
    initMetaInfo("gmbh.plz", "68167");
    initMetaInfo("gmbh.stadt", "Mannheim");
    initMetaInfo("gmbh.strasse", "Turley-Platz 9");
    initMetaInfo("gmbh.email","info@esperanza-mannheim.de");
    initMetaInfo("gmbh.url", "www.esperanza-mannheim.de");
}
bool create_DK_TablesAndContent(QSqlDatabase db)
{   LOG_CALL;
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON", db);

    db.transaction();

    if(!dkdbstructur.createDb(db)) {
        db.rollback();
        qCritical() << "creating db structure in new database failed";
        return false;
    }
    insert_UniqueDbProperties();
    insert_defaultGmbHData();
    db.commit();
    return isValidDatabase();
}

// database validation
bool check_db_version(QSqlDatabase db)
{   LOG_CALL;
    double d = getNumMetaInfo(DB_VERSION, db);
    if( d >= CURRENT_DB_VERSION)
        return true;
    qCritical() << "db version check failed: found version " << d << " needed version " << CURRENT_DB_VERSION;
    return false;
}
bool has_allTablesAndFields(QSqlDatabase db)
{   LOG_CALL;
    for( auto table : dkdbstructur.getTables()) {
        if( !verifyTable(table, db))
            return false;
    }
    qInfo() << db.databaseName() << " has all tables expected";
    return true;
}
bool isValidDatabase(const QString& filename)
{   LOG_CALL_W(filename);
    QString msg;
    if( filename == "") msg = "empty filename";
    else if( !QFile::exists(filename)) msg = "file not found";
    else {
            {
            dbCloser closer;
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "validate");
            db.setDatabaseName(filename);
            if( !db.open()) msg = "open db failed";
            else {
                closer.set(&db);
                if( !isValidDatabase(db))
                    msg = "database was found to be NOT valid";
            }
        }
    }
    if( msg.isEmpty())
        return true;
    qCritical() << msg;
    return false;
}
bool isValidDatabase(QSqlDatabase db)
{   LOG_CALL;
    QSqlQuery enableRefInt(db);
    enableRefInt.exec("PRAGMA foreign_keys = ON");
    if( !has_allTablesAndFields(db))
        return false;
    if( !check_db_version(db)) {
        qCritical() << "database version check failed";
        return false;
    }
    qInfo() << db.databaseName() << " is a valid dk database";
    return true;
}

// manage the app wide used database
void closeDatabaseConnection(QString con)
{   LOG_CALL_W(con);

    QSqlDatabase::removeDatabase(con);
    QList<QString> cl = QSqlDatabase::connectionNames();
    if( cl.count() == 0)
        return;
    if( cl.count() > 0) {
        qInfo() << "Found " << cl.count() << "connections open, after closing  \"" + con +"\"";
        return;
    }
    qInfo() << "Database connection " << con << " removed";
}
bool open_databaseForApplication( QString newDbFile)
{   LOG_CALL_W(newDbFile);
    Q_ASSERT(!newDbFile.isEmpty());

    closeDatabaseConnection();
    backupFile(newDbFile, "db-bak");

    // setting the default database for the application
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(newDbFile);
    if( !db.open()) {
        qCritical() << "open database file " << newDbFile << " failed";
        return false;
    }

    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON");
    if( !check_db_version(db))
    {
        closeDatabaseConnection();
        return false;
    }
    return true;
}

// db copy (w & w/o de-personalisation)
bool copy_TableContent(QString table, QSqlDatabase targetDB)
{   LOG_CALL_W(table);
    bool success = true;
    QSqlQuery q; // default database connection -> active database
    q.exec("SELECT * FROM " + table);
    while( q.next()) {
        QSqlRecord rec = q.record();
        qDebug() << "dePe Copy: working on Record " << rec;
        TableDataInserter tdi( dkdbstructur[table]);
        for( int iField = 0; iField < q.record().count(); iField++) {
            QString fieldname = rec.fieldName(iField);
            QVariant value = rec.value(iField);
            qDebug() << "Setting " << fieldname << " to " << value;
            tdi.setValue(fieldname, value);
        }
        if( tdi.InsertData(targetDB) == -1) {
            qCritical() << "Error inserting Data into deperso.Copy Table" << q.record();
            success = false;
            break;
        }
    }
    return success;
}
bool copy_mangledCreditors(QSqlDatabase targetDB)
{
    bool success = true;
    int recCount = 0;
    QSqlQuery q; // default database connection -> active database
    q.exec("SELECT * FROM Kreditoren");
    while( q.next()) {
        recCount++;
        QSqlRecord rec = q.record();
        qDebug() << "dePe Copy: working on Record " << rec;
        TableDataInserter tdi(dkdbstructur["Kreditoren"]);

        tdi.setValue("Vorname", QString("Vorname")+QString::number(recCount));
        tdi.setValue("Nachname", QString("Nachname")+QString::number(recCount));
        tdi.setValue("Strasse", QString("Strasse"));
        tdi.setValue("Plz", QString("D-xxxxx"));
        tdi.setValue("Stadt", QString("Stadt"));

        if( tdi.InsertData(targetDB) == -1) {
            qDebug() << "Error inserting Data into deperso.Copy Table" << q.record();
            success = false;
            break;
        }
    }
    return success;
}
bool create_DB_copy(QString targetfn, bool deper)
{   LOG_CALL_W(targetfn);
    if( QFile::exists(targetfn)) {
        backupFile(targetfn);
        QFile::remove(targetfn);
        if( QFile::exists(targetfn)) {
            qCritical() << "could not remove target file";
            return false;
        }
    }

    dbCloser closer;
    QSqlDatabase backupDB = QSqlDatabase::addDatabase("QSQLITE", "backup");
    backupDB.setDatabaseName(targetfn);

    if( !backupDB.open()) {
        qDebug() << "faild to open backup database";
        return false;
    }
    else
        closer.set(&backupDB);

    bool success = true;
    QVector<dbtable> tables = dkdbstructur.getTables();
    for( auto table : tables) {
        ensureTable(table, backupDB);
        qDebug() << "dePe Copy: working on table " << table.Name();
        if( deper && table.Name() == "Kreditoren")
            success = success && copy_mangledCreditors(backupDB);
        else
            success = success && copy_TableContent(table.Name(), backupDB);
    }
    return success;
}

// general stuff
QString proposeKennung()
{   LOG_CALL;
    static int idOffset = getMetaInfo("IdOffset").toInt();
    static int iMaxid = idOffset + getHighestTableId("Vertraege");
    QString kennung;
    do
    {
        QString maxid = QString::number(iMaxid).rightJustified(6, '0');
        QString PI = "DK-" + getMetaInfo("ProjektInitialen");
        kennung = PI + "-" + QString::number(QDate::currentDate().year()) + "-" + maxid;
        QVariant v = executeSingleValueSql("id", "Vertraege", "Kennung='" + kennung + "'");
        if( v.isValid())
            iMaxid++;
        else
            break;
    } while(1);
    iMaxid++; // prepare for next contract
    return kennung;
}

// todo: update for new contract structure, use sql helper functions
QString contractList_SELECT(const QVector<dbfield>& fields)
{   LOG_CALL;
    QString sql("SELECT ");
    for( int i = 0; i < fields.size(); i++)
    {
        if( i) sql +=", ";
        sql += fields[i].tableName() +"." +fields[i].name();
    }
    return sql;
}
QString contractList_FROM()
{   LOG_CALL;
    return  "FROM Vertraege, Kreditoren, Zinssaetze";
}
QString contractList_WHERE(const QString& Filter)
{   LOG_CALL;
    QString s ("WHERE Kreditoren.id = Vertraege.KreditorId AND Vertraege.ZSatz = Zinssaetze.id");
    bool isNumber (false);
    int index = Filter.toInt(&isNumber);
    if (isNumber)
    {
        s += " AND Kreditoren.id = '" + QString::number(index) + "'";
        return s;
    }
    s += " AND ( "
    "Vorname  LIKE '%" + Filter + "%' OR "
    "Nachname LIKE '%" + Filter + "%' OR "
    "Kennung  LIKE '%" + Filter + "%')";
    return s;
}
QString contractList_SQL(const QVector<dbfield>& fields, const QString& filter)
{   LOG_CALL;
    QString sql = contractList_SELECT(fields) + " "
            + contractList_FROM() + " "
            + contractList_WHERE(filter);
    qDebug() << "ContractList SQL: \n" << sql;
    return sql;
}

// summary stuff
// todo: update for new contract structure
void calculateSummary(DbSummary& dbs)
{   LOG_CALL;
    dbs.AnzahlDkGeber = executeSingleValueSql("count(DISTINCT(KreditorId))", "[Kreditoren],[Vertraege]", "aktiv != 0 AND Kreditoren.id = Vertraege.KreditorId").toInt();

    dbs.AnzahlAuszahlende = executeSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] = 0").toInt();
    dbs.BetragAuszahlende = executeSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] = 0").toReal();

    dbs.AnzahlThesaurierende= executeSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] != 0").toInt();
    dbs.BetragThesaurierende= executeSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] != 0").toReal();
    dbs.WertThesaurierende  = executeSingleValueSql("SUM([Wert])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] != 0").toReal();

    dbs.AnzahlAktive  = executeSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] != 0").toInt();
    dbs.BetragAktive  = executeSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] != 0").toReal();
    dbs.WertAktive    = dbs.BetragAuszahlende+dbs.WertThesaurierende;

    dbs.DurchschnittZins = executeSingleValueSql("SELECT SUM( w*z ) / SUM( W ) FROM  (SELECT MAX(Betrag, Wert) AS W, Zinssaetze.Zinssatz AS Z FROM Vertraege, Zinssaetze WHERE Zinssaetze.id = Vertraege.ZSatz AND Vertraege.aktiv)").toReal();
    dbs.MittlererZins = executeSingleValueSql("SELECT AVG(Zinssaetze.Zinssatz) FROM Vertraege, Zinssaetze WHERE Zinssaetze.id = Vertraege.ZSatz AND Vertraege.aktiv").toDouble();
    dbs.AnzahlPassive = executeSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] = 0").toInt();
    dbs.BetragPassive = executeSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] = 0").toReal();
}
bool createCsvActiveContracts()
{   LOG_CALL;
    QDate today = QDate::currentDate();
    QString filename(today.toString(Qt::ISODate) + "-Aktive-Vertraege.csv");

    filename = appConfig::Outdir() + "/" + filename;

    QVector<dbfield> fields; QVector<QVariant::Type> types;
    fields.append(dkdbstructur["Vertraege"]["id"]);
    types.append(QVariant::Int);
    fields.append(dkdbstructur["Vertraege"]["KreditorId"]);
    types.append(QVariant::Int);
    fields.append(dkdbstructur["Kreditoren"]["Vorname"]);
    types.append(QVariant::String);
    fields.append(dkdbstructur["Kreditoren"]["Nachname"]);
    types.append(QVariant::String);
    fields.append(dkdbstructur["Kreditoren"]["Strasse"]);
    types.append(QVariant::String);
    fields.append(dkdbstructur["Kreditoren"]["Stadt"]);
    types.append(QVariant::String);
    fields.append(dkdbstructur["Kreditoren"]["Nachname"]);
    types.append(QVariant::String);
    fields.append(dkdbstructur["Vertraege"]["Betrag"]);
    types.append(QVariant::Double);
    fields.append(dkdbstructur["Vertraege"]["Wert"]);
    types.append(QVariant::Double);
    fields.append(dkdbstructur["Vertraege"]["Vertragsdatum"]);
    types.append(QVariant::Date);

    if( table2csv( fields, types, "[aktiv] = 1", filename))
    {
        showFileInFolder(filename);
        return true;
    }
    return false;
}
void calc_contractEnd( QVector<ContractEnd>& ce)
{   LOG_CALL;

    QMap<int, int> m_count;
    QMap<int, double> m_sum;
    const int maxYear = QDate::currentDate().year() +99;

    QSqlQuery sql;
    sql.setForwardOnly(true);
    sql.exec("SELECT * FROM [Vertraege] WHERE [aktiv] = 1");
    while( sql.next())
    {
        QDate end = sql.record().value("LaufzeitEnde").toDate();
        if( !end.isValid()) continue;
        if( end.year() > maxYear) continue;
        if( end.year() < QDate::currentDate().year()) continue;
        if( m_count.contains(end.year()))
        {
            m_count[end.year()] = m_count[end.year()] +1;
            m_sum[end.year()]   = m_sum[end.year()] +
                                (sql.record().value("thesaurierend").toBool() ? sql.record().value("Wert").toReal() : sql.record().value("Betrag").toReal());
        }
        else
        {
            m_count[end.year()] = 1;
            m_sum[end.year()]   = (sql.record().value("thesaurierend").toBool() ? sql.record().value("Wert").toReal() : sql.record().value("Betrag").toReal());
        }
    }
    QMapIterator<int, int> i(m_count);
    while (i.hasNext()) {
        i.next();
        ce.append({i.key(), i.value(), m_sum[i.key()]});
    }
    return;
}
void calc_anualInterestDistribution( QVector<YZV>& yzv)
{   LOG_CALL;
    QString sql = "SELECT Substr([Vertragsdatum], 0, 5), [Zinssaetze].[Zinssatz], count(*), sum([Betrag]) "
                  "FROM [Vertraege], [Zinssaetze] "
                  "WHERE [ZSatz] = [Zinssaetze].[id] "
                  "GROUP BY Substr([Vertragsdatum], 0, 4), [ZSatz]";
    QSqlQuery query;
    query.exec(sql);
    while( query.next())
    {
        QSqlRecord r =query.record();
        yzv.push_back({r.value(0).toInt(), r.value(1).toReal(), r.value(2).toInt(), r.value(3).toReal() });
    }
    return;
}
QVector<rowData> contractRuntimeDistribution()
{
    int AnzahlBisEinJahr=0, AnzahlBisFuenfJahre=0, AnzahlLaenger=0, AnzahlUnbegrenzet = 0;
    double SummeBisEinJahr=0., SummeBisFuenfJahre=0., SummeLaenger=0., SummeUnbegrenzet = 0.;
    QString sql = "SELECT [Betrag], [Wert], [Vertragsdatum], [LaufzeitEnde] FROM [Vertraege]";
    QSqlQuery q;
    q.exec(sql);
    while( q.next())
    {
        double betrag = q.value("Betrag").toReal();
        double wert =   q.value("Wert").toReal();
        QDate von = q.value("Vertragsdatum").toDate();
        QDate bis = q.value("LaufzeitEnde").toDate();
        if(! bis.isValid() || bis == EndOfTheFuckingWorld)
        {
            AnzahlUnbegrenzet++;
            SummeUnbegrenzet += wert > betrag ? wert : betrag;
        }
        else if( von.addYears(5) < bis)
        {
            AnzahlLaenger++;
            SummeLaenger+= wert > betrag ? wert : betrag;
        }
        else if( von.addYears(1) > bis)
        {
            AnzahlBisEinJahr++;
            SummeBisEinJahr += wert > betrag ? wert : betrag;
        }
        else
        {
            AnzahlBisFuenfJahre ++;
            SummeBisFuenfJahre += wert > betrag ? wert : betrag;
        }
    }
    QLocale locale;
    QVector<rowData> ret;
    ret.push_back({"Zeitraum", "Anzahl", "Wert"});
    ret.push_back({"Bis ein Jahr ", QString::number(AnzahlBisEinJahr), locale.toCurrencyString(SummeBisEinJahr)});
    ret.push_back({"Ein bis fünf Jahre ", QString::number(AnzahlBisFuenfJahre), locale.toCurrencyString(SummeBisFuenfJahre)});
    ret.push_back({"Länger als fünf Jahre ", QString::number(AnzahlLaenger), locale.toCurrencyString(SummeLaenger) });
    ret.push_back({"Unbegrenzte Verträge ", QString::number(AnzahlUnbegrenzet), locale.toCurrencyString(SummeUnbegrenzet) });
    return ret;
}

