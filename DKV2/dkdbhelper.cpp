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
    dkdbstructur.appendTable(contract::getTableDef_deletedContracts());

    dkdbstructur.appendTable(booking::getTableDef());
    dkdbstructur.appendTable(booking::getTableDef_deletedBookings());

    dbtable meta("Meta");
    meta.append(dbfield("Name", QVariant::String).setNotNull().setUnique());
    meta.append(dbfield("Wert", QVariant::String).setNotNull());
    dkdbstructur.appendTable(meta);

    dbtable letters("Briefvorlagen");
    letters.append(dbfield("templateId",    QVariant::Int).setNotNull());
    letters.append(dbfield("EigenschaftId", QVariant::Int).setNotNull());
    letters.append(dbfield("Wert",          QVariant::String).setNotNull());
    QVector<dbfield> uniqueLetterFields;
    uniqueLetterFields.append(letters["templateId"]);
    uniqueLetterFields.append(letters["EigenschaftId"]);
    letters.setUnique(uniqueLetterFields);
    dkdbstructur.appendTable(letters);

    done = true;
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
void insert_DbProperties(QSqlDatabase db = QSqlDatabase::database())
{   LOG_CALL;
    dbConfig c(dbConfig::FROM_RTD); // get configuration defaults
    c.writeDb(db);
}

bool createView(QString name, QString sql, QSqlDatabase db) {
    QSqlQuery q(db);
    q.exec("DROP VIEW " + name);
    QString createViewSql = "CREATE VIEW %1 AS " + sql;
    createViewSql = createViewSql.arg(name);
    if( q.exec(createViewSql)) {
        qInfo() << "successfully created view " << name;
        return true;
    }
    qCritical() << "faild to create view " << name << endl << q.lastQuery() << endl << q.lastError();
    return false;
}
bool insert_views( QSqlDatabase db)
{   LOG_CALL;
    QString sqlWertAktiveVertraege =
            "SELECT Vertraege.id AS id, Kreditoren.Nachname || ', ' || Kreditoren.Vorname AS Kreditorin, Vertraege.Kennung AS Vertragskennung, Vertraege.ZSatz/100. AS Zinssatz, "
            "(SELECT sum(Buchungen.betrag) FROM Buchungen "
            "WHERE Vertraege.id = Buchungen.VertragsId) AS Wert, "
            "MIN(Buchungen.Datum) AS Datum, Vertraege.Kfrist AS Kündigungsfrist, Vertraege.LaufzeitEnde AS Vertragsende, thesaurierend AS thesa, Kreditoren.id AS KreditorId "
            "FROM Vertraege "
            "INNER JOIN Buchungen ON Buchungen.VertragsId = Vertraege.id "
            "INNER JOIN Kreditoren ON Kreditoren.id = Vertraege.KreditorId "
            "Group by Vertraege.id";
    bool ret = createView( "WertAktiveVertraege", sqlWertAktiveVertraege, db);

    QString sqlWertPassiveVertraege =
            "SELECT Vertraege.id AS id, "    // 0
            "Kreditoren.Nachname || ', ' || Kreditoren.Vorname AS Kreditorin, " // 1
            "Vertraege.Kennung AS Vertragskennung, " // 2
            "Vertraege.ZSatz/100. AS Zinssatz, "     // 3
            "-1* Vertraege.Betrag AS Wert, "         // 4
            "Vertraege.Vertragsdatum AS Datum, " // 5
            "Vertraege.Kfrist AS Kündigungsfrist, "      // 6
            "Vertraege.LaufzeitEnde AS Vertragsende, "   // 7
            "Vertraege.thesaurierend AS thesa, "          // 8
            "Kreditoren.id AS KreditorId " // 9
            "FROM Vertraege "
            "INNER JOIN Kreditoren ON Kreditoren.id = Vertraege.KreditorId "
            "WHERE (SELECT count(*) FROM Buchungen WHERE Buchungen.VertragsId=Vertraege.id) = 0";
    ret &= createView("WertPassiveVertraege", sqlWertPassiveVertraege, db);

    QString sqlWertAlleVertraege ="SELECT * FROM WertAktiveVertraege "
                                  "UNION "
                                  "SELECT * FROM WertPassiveVertraege ";
    ret &= createView("WertAlleVertraege", sqlWertAlleVertraege, db);

//    QString sqlAktiveVertraege ="SELECT DISTINCT Vertraege.* "
//            "FROM Buchungen INNER JOIN Vertraege ON Vertraege.id=buchungen.VertragsId ";
//    ret &= createView("AktiveVertraege", sqlAktiveVertraege, db);

    QString sqlWertExVertraege ="SELECT exVertraege.id AS id, "
                             "Kreditoren.Nachname || ', ' || Kreditoren.Vorname AS Kreditorin, "
                             "exVertraege.Kennung AS Vertragskennung, exVertraege.ZSatz/100. AS Zinssatz, "
                             "(SELECT sum(exBuchungen.betrag) FROM exBuchungen WHERE exVertraege.id = exBuchungen.VertragsId) AS Wert, "
                             "MIN(exBuchungen.Datum) AS Datum, exVertraege.Kfrist AS Kündigungsfrist, "
                             "exVertraege.LaufzeitEnde AS Vertragsende, thesaurierend AS thesa, Kreditoren.id AS KreditorId "
                             "FROM Vertraege "
                             "INNER JOIN Buchungen ON Buchungen.VertragsId = Vertraege.id "
                             "INNER JOIN Kreditoren ON Kreditoren.id = exVertraege.KreditorId "
                             "Group by exVertraege.id";
    ret &= createView( "WertExVertraege", sqlWertExVertraege, db);

    /* Wann muss die Abrechnung für Verträge gemacht werden, die noch keine Zinsabrechnung hatten? */
    QString sqlFirstInterestDates =
            "SELECT STRFTIME('%Y-%m-%d', MIN(Datum), '1 year', 'start of year')  as nextInterestDate "
            "FROM Buchungen INNER JOIN Vertraege ON Vertraege.id = buchungen.VertragsId "
            /* buchungen von Verträgen für die es keine Zinsbuchungen gibt */
            "WHERE (SELECT count(*) FROM Buchungen WHERE (Buchungen.BuchungsArt=4 OR Buchungen.BuchungsArt=8) AND Buchungen.VertragsId=Vertraege.id)=0 "
            "GROUP BY Vertraege.id ";
    createView("NextAnnualS_first", sqlFirstInterestDates, db);

    /* Wann muss die Abrechnung für Verträge gemacht werden, die bereits Zinsabrechnung(en) hatten? */
    QString sqlNextInterestDates =
        "SELECT STRFTIME('%Y-%m-%d', MAX(Datum), '1 year', 'start of year') as nextInterestDate "
        "FROM Buchungen INNER JOIN Vertraege ON Vertraege.id=buchungen.VertragsId "
        "WHERE Buchungen.BuchungsArt=4 OR Buchungen.BuchungsArt=8 "
        "GROUP BY Buchungen.VertragsId "
        "ORDER BY Datum ASC LIMIT 1";
    ret &= createView("NextAnnualS_next", sqlNextInterestDates, db);

    QString sqlNextInterestDate( "SELECT MIN(nextInterestDate) AS date FROM "
                              "(SELECT nextInterestDate FROM NextAnnualS_first "
                              "UNION SELECT nextInterestDate from NextAnnualS_next)");
    ret &= createView("NextAnnualSettlement", sqlNextInterestDate, db);

    QString sqlDeletedContracts =
            "SELECT exVertraege.id AS id, Kreditoren.Nachname || ', ' || Kreditoren.Vorname AS Kreditorin, exVertraege.Kennung AS Vertragskennung, exVertraege.ZSatz/100. AS Zinssatz, "
            "(SELECT sum(exBuchungen.betrag) FROM exBuchungen "
            "WHERE exVertraege.id = exBuchungen.VertragsId) AS Wert, "
            "MIN(exBuchungen.Datum) AS Datum, exVertraege.Kfrist AS Kündigungsfrist, exVertraege.LaufzeitEnde AS Vertragsende, thesaurierend AS thesa, Kreditoren.id AS KreditorId "
            "FROM exVertraege "
            "INNER JOIN exBuchungen ON exBuchungen.VertragsId = exVertraege.id "
            "INNER JOIN Kreditoren ON Kreditoren.id = exVertraege.KreditorId "
            "Group by exVertraege.id";
    ret &= createView("WertBeendeteVertraege", sqlDeletedContracts, db);

    QString sqlAnzahlAktiveDkGeber = "SELECT COUNT(DISTINCT KreditorId) AS AnzahlDkgeber FROM AktiveVertraege";
    ret &= createView("AnzahlAktiveDkGeber", sqlAnzahlAktiveDkGeber, db);
    return ret;
}

bool create_DK_TablesAndContent(QSqlDatabase db)
{   LOG_CALL;
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON", db);

    db.transaction();
    if( ! (dkdbstructur.createDb(db) && insert_views(db))) {
        db.rollback();
        return false;
    }
    insert_DbProperties(db);
    db.commit();
    return isValidDatabase(db);
}

// database validation
bool check_db_version(QSqlDatabase db)
{   LOG_CALL;
    double d = getNumMetaInfo(DB_VERSION, -1., db);
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
    // todo : check views
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
    if( !check_db_version(db)) {
        closeDatabaseConnection();
        return false;
    }
    dbConfig c(dbConfig::FROM_DB);
    c.storeRuntimeData();
    return true;
}

// db copy (w & w/o de-personalisation)
bool copy_TableContent(QString table, QSqlDatabase targetDB)
{   LOG_CALL_W(table);
    bool success = true;
    QSqlQuery q(QSqlDatabase::database()); // default database connection -> active database, the data base to be copied
    q.exec("SELECT * FROM " + table);
    while( q.next()) {
        QSqlRecord rec = q.record();
        qDebug() << "dePe Copy: working on Record " << rec;
        TableDataInserter tdi( dkdbstructur[table]);
        if( (!tdi.setValues(rec)) || (tdi.InsertOrReplaceData(targetDB) == -1)) {
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
    QSqlQuery q(QSqlDatabase::database()); // default database connection -> active database
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
            qCritical() << "db_copy: could not remove target file";
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
    create_DK_TablesAndContent(backupDB);

    bool result = true;
    QVector<dbtable> tables = dkdbstructur.getTables();
    for( auto table : tables) {
        if( deper && table.Name() == "Kreditoren")
            result = result && copy_mangledCreditors(backupDB);
        else
            result = result && copy_TableContent(table.Name(), backupDB);
    }
    return result;
}

// general stuff
QString proposeKennung()
{   LOG_CALL;
    static int idOffset = getMetaInfo("IdOffset").toInt();
    static int iMaxid = idOffset + getHighestRowId("Vertraege");
    QString kennung;
    do {
        QString maxid = QString::number(iMaxid).rightJustified(6, '0');
        QString PI = "DK-" + getMetaInfo(GMBH_PI);
        kennung = PI + "-" + QString::number(QDate::currentDate().year()) + "-" + maxid;
        QVariant v = executeSingleValueSql(dkdbstructur["Vertraege"]["id"], "Kennung='" + kennung + "'");
        if( v.isValid())
            iMaxid++;
        else
            break;
    } while(1);
    iMaxid++; // prepare for next contract
    return kennung;
}
void create_sampleData(int datensaetze)
{
    saveRandomCreditors(datensaetze);
    saveRandomContracts(datensaetze);
    activateRandomContracts(90);

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
    dbs.AnzahlDkGeber = executeSingleValueSql("AnzahlDkgeber", "AnzahlAktiveDkGeber").toInt();
    dbs.AnzahlAktive  = executeSingleValueSql("COUNT(*)", "WertAktiveVertraege").toInt();
    dbs.WertAktive  = executeSingleValueSql("SUM(Wert)", "WertAktiveVertraege").toInt() /100.;

    dbs.DurchschnittZins = executeSingleValueSql("z/w AS median", "(SELECT SUM(Zinssatz *Wert) AS z, SUM(Wert) AS w FROM WertAktiveVertraege)").toReal();
    dbs.MittlererZins = executeSingleValueSql("AVG(Zinssatz)", "WertAktiveVertraege").toDouble();

    dbs.AnzahlAuszahlende = executeSingleValueSql("COUNT(*)", "WertAktiveVertraege", "NOT thesa").toInt();
    dbs.BetragAuszahlende = executeSingleValueSql("SUM(Wert)", "WertAktiveVertraege", "NOT thesa").toInt() /100.;

    dbs.AnzahlThesaurierende= executeSingleValueSql("COUNT(*)", "WertAktiveVertraege", "thesa").toInt();
    dbs.WertThesaurierende  = executeSingleValueSql("SUM(Wert)", "WertAktiveVertraege", "thesa").toInt() /100.;

    dbs.AnzahlPassive = executeSingleValueSql("COUNT(*)", "WertPassiveVertraege").toInt();
    dbs.BetragPassive = executeSingleValueSql("SUM(Wert)", "WertPassiveVertraege").toReal() /-100.;
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
    sql.exec("SELECT * FROM [Vertraege] WHERE [aktiv] = 1");// TODO
    while( sql.next())
    {
        QDate end = sql.record().value("LaufzeitEnde").toDate();
        if( !end.isValid()) continue;
        if( end.year() > maxYear) continue;
        if( end.year() < QDate::currentDate().year()) continue;
        if( m_count.contains(end.year())) {
            m_count[end.year()] = m_count[end.year()] +1;
            m_sum[end.year()]   = m_sum[end.year()] +
                                (sql.record().value("thesaurierend").toBool() ? sql.record().value("Wert").toReal() : sql.record().value("Betrag").toReal());
        } else {
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
                  "WHERE [ZSatz] = [Zinssaetze].[id] " // TODO
                  "GROUP BY Substr([Vertragsdatum], 0, 4), [ZSatz]";
    QSqlQuery query;
    query.exec(sql);
    while( query.next()) {
        QSqlRecord r =query.record();
        yzv.push_back({r.value(0).toInt(), r.value(1).toReal(), r.value(2).toInt(), r.value(3).toReal() });
    }
    return;
}
QVector<rowData> contractRuntimeDistribution()
{
    int AnzahlBisEinJahr=0, AnzahlBisFuenfJahre=0, AnzahlLaenger=0, AnzahlUnbegrenzet = 0;
    double SummeBisEinJahr=0., SummeBisFuenfJahre=0., SummeLaenger=0., SummeUnbegrenzet = 0.;
    QString sql = "SELECT [Betrag], [Wert], [Vertragsdatum], [LaufzeitEnde] FROM [Vertraege]"; // TODO
    QSqlQuery q;
    q.exec(sql);
    while( q.next()) {
        double betrag = q.value("Betrag").toReal();
        double wert =   q.value("Wert").toReal();
        QDate von = q.value("Vertragsdatum").toDate();
        QDate bis = q.value("LaufzeitEnde").toDate();
        if(! bis.isValid() || bis == EndOfTheFuckingWorld) {
            AnzahlUnbegrenzet++;
            SummeUnbegrenzet += wert > betrag ? wert : betrag;
        } else if( von.addYears(5) < bis) {
            AnzahlLaenger++;
            SummeLaenger+= wert > betrag ? wert : betrag;
        } else if( von.addYears(1) > bis) {
            AnzahlBisEinJahr++;
            SummeBisEinJahr += wert > betrag ? wert : betrag;
        } else {
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

