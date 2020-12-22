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
#include "letterTemplate.h"
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

    dkdbstructur.appendTable(letterTemplate::getTableDef_letterTypes());
    dkdbstructur.appendTable(letterTemplate::getTabelDef_elementTypes());
    dkdbstructur.appendTable(letterTemplate::getTableDef_letterElements());

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
{
    LOG_CALL;
    dbConfig c(dbConfig::FROM_RTD); // get configuration defaults
    c.writeDb(db);
}
bool create_DK_TablesAndContent(QSqlDatabase db)
{
    LOG_CALL;
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON", db);
    bool ret = false;
    db.transaction();
    do {
        if (!dkdbstructur.createDb(db)) break;
        if (!insert_views(db)) break;
        insert_DbProperties(db);
        if (!letterTemplate::insert_letterTypes(db)) break;
        if (!letterTemplate::insert_elementTypes(db)) break;
        if (!letterTemplate::insert_letterElements(db)) break;
        ret = true;
    } while (false);
    if (ret)
        db.commit();
    else
        db.rollback();
    return ret ? isValidDatabase(db) : false;
}

// create db views
struct dbViewDev{
    QString name;
    QString sql;
};

bool createView(QString name, QString sql, QSqlDatabase db =QSqlDatabase::database()) {
    db.transaction();
    QSqlQuery q(db);
    q.exec("DROP VIEW " + name);
    QString createViewSql = "CREATE VIEW %1 AS " + sql;
    createViewSql = createViewSql.arg(name);
    if( q.exec(createViewSql) && q.lastError().type() == QSqlError::NoError) {
        db.commit();
        qInfo() << "successfully created view " << name;
        return true;
    }
    db.rollback();
    qCritical() << "faild to create view " << name << Qt::endl << q.lastQuery() << Qt::endl << q.lastError();
    return false;
}
bool createViews( QVector<dbViewDev>& views, QSqlDatabase db)
{
    for( auto view: views) {
        if( ! createView(view.name, view.sql, db))
            return false;
    }
    return true;
}

bool insert_views( QSqlDatabase db)
{   LOG_CALL;
    QVector<dbViewDev> views = {
        {qsl("vVertraege_aktiv_detail"),          qsl("SELECT "
                                                          "Vertraege.id          AS id, "
                                                          "Vertraege.Kennung     AS Vertragskennung, "
                                                          "Vertraege.ZSatz /100. AS Zinssatz, "
                                                          "SUM(Buchungen.Betrag) /100. as Wert, "
                                                          "MIN(Buchungen.Datum)  AS Aktivierungsdatum, "
                                                          "Vertraege.Kfrist AS Kuendigungsfrist, "
                                                          "Vertraege.LaufzeitEnde AS Vertragsende, "
                                                          "Vertraege.thesaurierend AS thesa, "
                                                          "Kreditoren.Nachname || ', ' || Kreditoren.Vorname AS Kreditorin, "
                                                          "Kreditoren.id AS KreditorId, "
                                                          "Kreditoren.Nachname AS Nachname, "
                                                          "Kreditoren.Vorname AS Vorname, "
                                                          "Kreditoren.Strasse AS Strasse, "
                                                          "Kreditoren.Plz AS Plz, "
                                                          "Kreditoren.Stadt AS Stadt, "
                                                          "Kreditoren.Email AS Email, "
                                                          "Kreditoren.IBAN AS Iban, "
                                                          "Kreditoren.BIC AS Bic "
                                                        "FROM Vertraege "
                                                            "INNER JOIN Buchungen ON Buchungen.VertragsId = Vertraege.id "
                                                            "INNER JOIN Kreditoren ON Kreditoren.id = Vertraege.KreditorId "
                                                        "GROUP BY Vertraege.id")},
        {qsl("vVertraege_aktiv"),      qsl("SELECT id, " // 0
                                            "Kreditorin, " // 1
                                           "Vertragskennung, " // 2
                                            "Zinssatz, " // 3
                                            "Wert, " // 4
                                            "Aktivierungsdatum, " // 5
                                            "Kuendigungsfrist, "  // 6
                                            "Vertragsende, " // 7
                                            "thesa, "       // 8
                                            "KreditorId " // 9
                                            "FROM vVertraege_aktiv_detail")},
        {qsl("vVertraege_passiv"),     qsl("SELECT Vertraege.id AS id, "    // 0: id
                                             "Kreditoren.Nachname || ', ' || Kreditoren.Vorname AS Kreditorin, " // 1: kreditor name
                                             "Vertraege.Kennung AS Vertragskennung, " // 2: contract label
                                             "Vertraege.ZSatz /100. AS Zinssatz, "     // 3: interest in %
                                             "Vertraege.Betrag /100. AS Wert, "         // 4: value in Euro
                                             "Vertraege.Vertragsdatum AS Abschlussdatum, " // 5: Abschlussdatum / akt. Datum
                                             "Vertraege.Kfrist AS Kuendigungsfrist, "      // 6: kfrist (monate)
                                             "Vertraege.LaufzeitEnde AS Vertragsende, "   // 7: LZende
                                             "Vertraege.thesaurierend AS thesa, "          // 8: payout?
                                             "Kreditoren.id AS KreditorId " // 9: kredid
                                           "FROM Vertraege "
                                             "INNER JOIN Kreditoren ON Kreditoren.id = Vertraege.KreditorId "
                                           "WHERE (SELECT count(*) FROM Buchungen WHERE Buchungen.VertragsId=Vertraege.id) = 0")},
        {qsl("vVertraege_alle"),        qsl("SELECT id, Kreditorin, Vertragskennung, Zinssatz, Wert, Aktivierungsdatum AS Datum, Kuendigungsfrist, Vertragsende, thesa, KreditorId "
                                             "FROM vVertraege_aktiv"
                                            "  UNION  "
                                            "SELECT id, Kreditorin, Vertragskennung, Zinssatz, Wert, Abschlussdatum  AS Datum,  Kuendigungsfrist, Vertragsende, thesa, KreditorId "
                                             "FROM vVertraege_passiv")},
        {qsl("vVertraege_alle_4view"),  qsl("SELECT id, Kreditorin, Vertragskennung, Zinssatz, Wert, Aktivierungsdatum AS Datum, Kuendigungsfrist, Vertragsende, thesa, KreditorId "
                                             "FROM vVertraege_aktiv"
                                            "  UNION  "
                                            "SELECT id, Kreditorin, Vertragskennung, Zinssatz, -1*Wert, Abschlussdatum  AS Datum,  Kuendigungsfrist, Vertragsende, thesa, KreditorId "
                                             "FROM vVertraege_passiv")},
        {qsl("vVertraege_geloescht"),    qsl("SELECT "
                                              "exVertraege.id AS id, " // 0: id
                                              "Kreditoren.Nachname || ', ' || Kreditoren.Vorname AS Kreditorin, " // 1: kreditorin
                                              "exVertraege.Kennung AS Vertragskennung, " // 2: contract label
                                              "exVertraege.ZSatz/100. AS Zinssatz, " // 3: interest in %
                                              "SUM(exBuchungen.betrag)  AS Wert, "   //4: value in Euro
                                             "MIN(exBuchungen.Datum) AS Datum, exVertraege.Kfrist AS Kuendigungsfrist, " // 5: act. date, 6: kfrist (monate)
                                             "exVertraege.LaufzeitEnde AS Vertragsende, " // 7: Vertragsende
                                             "thesaurierend AS thesa, " // 8: payout?
                                             "Kreditoren.id AS KreditorId " // 9: kredid
                                           "FROM exVertraege "
                                              "INNER JOIN exBuchungen ON exBuchungen.VertragsId = exVertraege.id "
                                              "INNER JOIN Kreditoren ON Kreditoren.id = exVertraege.KreditorId "
                                           "Group by exVertraege.id")},

        {qsl("NextAnnualS_first"),      qsl("SELECT STRFTIME('%Y-%m-%d', MIN(Datum), '1 year', 'start of year', '-1 day')  as nextInterestDate "
                                            "FROM Buchungen INNER JOIN Vertraege ON Vertraege.id = buchungen.VertragsId "
                                            /* buchungen von Verträgen für die es keine Zinsbuchungen gibt */
                                            "WHERE (SELECT count(*) FROM Buchungen WHERE (Buchungen.BuchungsArt=4 OR Buchungen.BuchungsArt=8) AND Buchungen.VertragsId=Vertraege.id)=0 "
                                            "GROUP BY Vertraege.id ")},

        {qsl("NextAnnualS_next"),       qsl("SELECT STRFTIME('%Y-%m-%d', MAX(Datum), '1 day', '1 year', 'start of year', '-1 day') as nextInterestDate "
                                            "FROM Buchungen INNER JOIN Vertraege ON Vertraege.id=buchungen.VertragsId "
                                            "WHERE Buchungen.BuchungsArt=4 OR Buchungen.BuchungsArt=8 "
                                            "GROUP BY Buchungen.VertragsId "
                                            "ORDER BY Datum ASC LIMIT 1")},
        {qsl("NextAnnualSettlement"),    qsl("SELECT MIN(nextInterestDate) AS date FROM "
                                         "(SELECT nextInterestDate FROM NextAnnualS_first "
                                         "UNION SELECT nextInterestDate from NextAnnualS_next)")},
        {qsl("ContractsByYearByInterest"),qsl("SELECT SUBSTR(Vertraege.Vertragsdatum, 0, 5) as Year, "
                                               "Vertraege.ZSatz /100. AS Zinssatz, count(*) AS Anzahl, sum(Vertraege.Betrag) /100. AS Summe "
                                               "FROM Vertraege "
                                               "GROUP BY Year, Zinssatz ")},

        {qsl("vAnzahl_allerKreditoren"),           qsl("SELECT COUNT(*) AS Anzahl FROM (SELECT DISTINCT KreditorId FROM Vertraege)")},
        {qsl("vAnzahl_allerKreditoren_thesa"),     qsl("SELECT COUNT(*) AS Anzahl FROM (SELECT DISTINCT KreditorId FROM Vertraege WHERE thesaurierend)")},
        {qsl("vAnzahl_allerKreditoren_ausz"),      qsl("SELECT COUNT(*) AS Anzahl FROM (SELECT DISTINCT KreditorId FROM Vertraege WHERE NOT thesaurierend)")},

        {qsl("vAnzahl_aktiverKreditoren"),          qsl("SELECT count(*) AS Anzahl FROM (SELECT DISTINCT KreditorId FROM vVertraege_aktiv)")},
        {qsl("vAnzahl_aktiverKreditoren_thesa"),    qsl("SELECT count(*) AS Anzahl FROM (SELECT DISTINCT KreditorId FROM vVertraege_aktiv WHERE thesa)")},
        {qsl("vAnzahl_aktiverKreditoren_ausz"),     qsl("SELECT count(*) AS Anzahl FROM (SELECT DISTINCT KreditorId FROM vVertraege_aktiv WHERE NOT thesa)")},

        {qsl("vAnzahl_passiverKreditoren"),        qsl("SELECT count(*) AS Anzahl FROM (SELECT DISTINCT KreditorId FROM vVertraege_passiv)")},
        {qsl("vAnzahl_passiverKreditoren_thesa"),  qsl("SELECT count(*) AS Anzahl FROM (SELECT DISTINCT KreditorId FROM vVertraege_passiv WHERE thesa)")},
        {qsl("vAnzahl_passiverKreditoren_ausz"),   qsl("SELECT count(*) AS Anzahl FROM (SELECT DISTINCT KreditorId FROM vVertraege_passiv WHERE NOT thesa)")},

    };

    QString sql_precalc {
        qsl("SELECT *, ROUND(100* Jahreszins/Wert,6) as gewMittel FROM ("
           "SELECT "
              "count(*) as Anzahl, "
              "SUM(Wert) as Wert, "
              "SUM(ROUND(Zinssatz *Wert /100,2)) AS Jahreszins,"
              "ROUND(AVG(Zinssatz),4) as mittlereRate "
           "FROM %1 %2)")};
    views.append({qsl("vStat_allerVertraege"),         sql_precalc.arg(qsl("vVertraege_alle"), qsl(""))});
    views.append({qsl("vStat_allerVertraege_thesa"),   sql_precalc.arg(qsl("vVertraege_alle"), qsl("WHERE thesa"))});
    views.append({qsl("vStat_allerVertraege_ausz"),    sql_precalc.arg(qsl("vVertraege_alle"), qsl("WHERE NOT thesa"))});
    views.append({qsl("vStat_aktiverVertraege"),       sql_precalc.arg(qsl("vVertraege_aktiv"), qsl(""))});
    views.append({qsl("vStat_aktiverVertraege_thesa"), sql_precalc.arg(qsl("vVertraege_aktiv"), qsl("WHERE thesa"))});
    views.append({qsl("vStat_aktiverVertraege_ausz"),  sql_precalc.arg(qsl("vVertraege_aktiv"), qsl("WHERE NOT thesa"))});
    views.append({qsl("vStat_passiverVertraege"),      sql_precalc.arg(qsl("vVertraege_passiv"), qsl(""))});
    views.append({qsl("vStat_passiverVertraege_thesa"),sql_precalc.arg(qsl("vVertraege_passiv"), qsl("WHERE thesa"))});
    views.append({qsl("vStat_passiverVertraege_ausz"), sql_precalc.arg(qsl("vVertraege_passiv"), qsl("WHERE NOT thesa"))});

    return createViews(views, db);
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
    for( auto& table : dkdbstructur.getTables()) {
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

    for( auto s : cl) {
        qInfo()<< "Found " << cl.count() << "connections open, after closing: '" + con +"'";
        QSqlDatabase::removeDatabase(s);
    }
    cl.clear();
    cl = QSqlDatabase::connectionNames();
    if( cl.size())
        qInfo() << "not all connection to the database could be closed";
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
        // set all values
        for( int i =0; i <rec.count(); i++) {
            QSqlField f =rec.field(i);
            if( f.requiredStatus() == QSqlField::Required) {
                // it is "NOT NULL"
                tdi.setValue(f.name(), f.value(), TableDataInserter::treatNull::notAllowNullValues);
            } else {
                // it could be "NULL"
                tdi.setValue(f.name(), f.value(), TableDataInserter::treatNull::allowNullValues);
            }
        }
        if(tdi.InsertData_noAuto(targetDB) == -1) {
            qCritical() << "Error inserting Data into Table copy " << table << ": " << q.record();
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
        QString vn {qsl("Vorname")}, nn {qsl("Nachname")};
        tdi.setValue(qsl("Vorname"),  QVariant(vn + QString::number(recCount)));
        tdi.setValue(qsl("Nachname"), QVariant(nn + QString::number(recCount)));
        tdi.setValue("Strasse", QString("Strasse"));
        tdi.setValue("Plz", QString("D-xxxxx"));
        tdi.setValue("Stadt", QString("Stadt"));

        if( tdi.InsertData(targetDB) == -1) {
            qDebug() << "Error inserting Data into deperso.Copy Table" << q.lastError() << Qt::endl << q.record();
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
    bool result = true;
    { // backupDb Scope - KEEP ! so that the database can be closed
    QSqlDatabase backupDB = QSqlDatabase::addDatabase("QSQLITE", "backup");
    backupDB.setDatabaseName(targetfn);

    if( !backupDB.open()) {
        qDebug() << "faild to open backup database";
        return false;
    }
    else
        closer.set(&backupDB);
    create_DK_TablesAndContent(backupDB);

    QVector<dbtable> tables = dkdbstructur.getTables();
    for( auto& table : qAsConst(tables)) {
        if( deper && table.Name() == qsl("Kreditoren"))
            result = result && copy_mangledCreditors(backupDB);
        else if (table.Name() == qsl("BriefElemente")) {
            QSqlQuery deleteDefaultValues(qsl("DELETE FROM BriefElemente"), backupDB);
            result = result && copy_TableContent(table.Name(), backupDB);
        }
        else
            result = result && copy_TableContent(table.Name(), backupDB);
    }
    }
    return result;
}

// general stuff
QString proposeContractLabel()
{   LOG_CALL;
    static int idOffset = getMetaInfo(STARTINDEX).toInt();
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
bool createCsvActiveContracts()
{   LOG_CALL;
    QString filename(QDate::currentDate().toString(Qt::ISODate) + "-Aktive-Vertraege.csv");
    dbtable t("vVertraege_aktiv_detail");
    t.append(dbfield("Id", QVariant::Type::Int));
    t.append(dbfield("KreditorId", QVariant::Type::Int));
    t.append(dbfield("Vorname"));
    t.append(dbfield("Nachname"));
    t.append(dbfield("Strasse"));
    t.append(dbfield("Plz"));
    t.append(dbfield("Stadt"));
    t.append(dbfield("Email"));
    t.append(dbfield("Iban"));
    t.append(dbfield("Bic"));
    t.append(dbfield("Strasse"));
    t.append(dbfield("Zinssatz", QVariant::Type::Double));
    t.append(dbfield("Wert", QVariant::Type::Double));
    t.append(dbfield("Aktivierungsdatum", QVariant::Type::Date));
    t.append(dbfield("Kuendigungsfrist", QVariant::Type::Int));
    t.append(dbfield("Vertragsende", QVariant::Type::Date));
    t.append(dbfield("thesa", QVariant::Type::Bool));

    if( !table2csv( filename, t.Fields())) {
        qDebug() << "failed to print table";
        return false;
    }
    return true;
}

// statistics, overviews
QVector<rowData> contractRuntimeDistribution()
{   LOG_CALL;
    int AnzahlBisEinJahr=0, AnzahlBisFuenfJahre=0, AnzahlLaenger=0, AnzahlUnbegrenzet = 0;
    double SummeBisEinJahr=0., SummeBisFuenfJahre=0., SummeLaenger=0., SummeUnbegrenzet = 0.;
    QString sql = "SELECT Wert, Aktivierungsdatum, Vertragsende "
                  "FROM vVertraege_aktiv";
    QSqlQuery q;
    if( !q.exec(sql)) {
        qCritical() << "calculation of runtime distribution failed: " << q.lastError() << Qt::endl << q.lastQuery();
        return QVector<rowData>();
    }

    while( q.next()) {
        double wert =   q.value("Wert").toReal();
        QDate von = q.value("Datum").toDate();
        QDate bis = q.value("Vertragsende").toDate();
        if( ! bis.isValid() || bis == EndOfTheFuckingWorld) {
            AnzahlUnbegrenzet++;
            SummeUnbegrenzet += wert;
        } else if( bis > von.addYears(1) && bis < von.addYears(5)) {
            AnzahlLaenger++;
            SummeLaenger += wert;
        } else if( bis < von.addYears(1)) {
            AnzahlBisEinJahr++;
            SummeBisEinJahr +=wert;
        } else {
            AnzahlBisFuenfJahre ++;
            SummeBisFuenfJahre += wert;
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
void calc_payedInterestsByYear(QVector<PayedInterest>& pi)
{   LOG_CALL;
    QString sql {qsl("SELECT  STRFTIME('%Y', Datum) as Year, SUM(Buchungen.Betrag) as Summe, BuchungsArt, Vertraege.thesaurierend as Thesa "
                    "FROM Buchungen INNER JOIN Vertraege ON Buchungen.VertragsId = Vertraege.id "
                    "WHERE Buchungen.BuchungsArt = 8 OR Buchungen.BuchungsArt =4 "
                    "GROUP BY Year, Buchungen.BuchungsArt, thesaurierend "
                    "ORDER BY Year DESC, Buchungen.BuchungsArt DESC, thesaurierend")};

    QVector<QSqlRecord> records;
    if( !executeSql(sql, QVariant(), records)) return;
    for( int i =0; i< records.size(); i++) {
        PayedInterest p;
        p.year = records[i].value("Year").toInt();
        p.value = euroFromCt(records[i].value("Summe").toDouble());
        booking::Type t = static_cast<booking::Type>(records[i].value("BuchungsArt").toInt());
        int thesa =records[i].value("Thesa").toInt();
        if( t == booking::Type::annualInterestDeposit) {
            if(thesa == 1)
                p.interestTypeDesc =qsl("Angerechnete Jahresendzinsen");
            else if( thesa == 0)
                p.interestTypeDesc =qsl("Ausgezahlte Jahresendzinsen");
        }
        else if( t == booking::Type::reInvestInterest)
            p.interestTypeDesc = qsl("Unterjährig angerechnete Zinsen");
        else {
            qCritical() << "Wrong booking type in statistic function";
            p.interestTypeDesc =qsl("-error-");
        }
        pi.push_back(p);
    }
    return;
}

void calc_contractEnd( QVector<ContractEnd>& ces)
{   LOG_CALL;
    QSqlQuery sql;
    sql.setForwardOnly(true);
    sql.exec("SELECT count(*) AS Anzahl, sum(Wert) AS Wert, strftime('%Y',Vertragsende) AS Jahr "
             "FROM vVertraege_aktiv "
             "WHERE Vertragsende < 9999-01-01 GROUP BY strftime('%Y',Vertragsende) ORDER BY Jahr");
    while( sql.next()) {
        ces.push_back({sql.value("Jahr").toInt(), sql.value("Anzahl").toInt(), sql.value("Wert").toDouble()});
    }
    return;
}
void calc_anualInterestDistribution( QVector<YZV>& yzv)
{   LOG_CALL;
    QString sql ="SELECT * FROM ContractsByYearByInterest ORDER BY Year";
    QSqlQuery query;
    query.exec(sql);
    while( query.next()) {
        QSqlRecord r =query.record();
        yzv.push_back({r.value("Year").toInt(), r.value("Zinssatz").toReal(),
                       r.value("Anzahl").toInt(), r.value("Summe").toReal() /100. });
    }
    return;
}

