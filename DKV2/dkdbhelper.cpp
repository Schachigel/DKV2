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

const QString sqlContractView {
    qsl("SELECT \
  V.id                                     AS VertragsId, \
  K.id                                    AS KreditorId, \
  K.Nachname || ', ' || K.Vorname          AS KreditorIn, \
  V.Kennung                                AS Vertragskennung, \
  strftime('%d.%m.%Y',V.Vertragsdatum)     AS Vertragsdatum, \
  ifnull(AktivierungsWert, V.Betrag / 100.) AS Nominalwert, \
  CAST(V.Zsatz / 100. AS VARCHAR) || ' %'    AS Zinssatz, \
  CASE WHEN V.thesaurierend = 0 THEN 'Auszahlend' \
  ELSE CASE WHEN V.thesaurierend = 1 THEN 'Thesaur.' \
       ELSE CASE WHEN V.thesaurierend = 2 THEN 'Fester Zins' \
            ELSE 'ERROR' \
            END \
       END \
  END                                      AS Zinsmodus, \
  ifnull(Aktivierungsdatum, ' - ')         AS Aktivierung, \
  ifnull(CASE WHEN V.thesaurierend = 1 THEN GesamtWert \
  ELSE AktivierungsWert \
  END, 0.)                                 AS VerzinslGuthaben, \
  ifnull(ZinsSumme, 0.)                    AS angespZins, \
  ifnull(LetzteBuchung, ' - ')             AS LetzteBuchung, \
  CASE WHEN V.Kfrist = -1 THEN strftime('%d.%m.%Y', V.LaufzeitEnde) \
  ELSE '(' || CAST(V.Kfrist AS VARCHAR) || ' Monate)' \
  END                                      AS KdgFristVertragsende \
FROM Vertraege AS V \
 \
INNER JOIN Kreditoren AS K ON V.KreditorId = K.id \
 \
LEFT JOIN \
(SELECT vid_min, minDate AS Aktivierungsdatum, minValue AS AktivierungsWert, GesamtWert, round(GesamtWert - minValue,2) AS ZinsSumme \
FROM \
  (SELECT B.VertragsId AS vid_min, min(B.Datum) AS minDate, B.Betrag / 100. AS minValue, sum(B.Betrag) / 100. AS GesamtWert \
   FROM Buchungen AS B GROUP BY B.VertragsId) \
) ON V.id = vid_min \
 \
LEFT JOIN \
(SELECT vid_max, maxDate AS LetzteBuchung \
FROM \
  (SELECT B_.VertragsId AS vid_max, max(B_.Datum) AS maxDate \
   FROM Buchungen AS B_ GROUP BY B_.VertragsId) \
) ON V.id = vid_max "
       )
};

const QString sqlExContractView {qsl(
"SELECT \
  V.id AS VertragsId, \
  K.id AS KreditorId, \
  K.Nachname || ', ' || K.Vorname AS KreditorIn, \
  V.Kennung AS Vertragskennung, \
  strftime('%d.%m.%Y',Aktivierungsdatum) AS Aktivierung, \
  strftime('%d.%m.%Y', Vertragsende) AS Vertragsende, \
  ifnull(AktivierungsWert, V.Betrag / 100.) AS Anfangswert, \
  CAST(V.Zsatz / 100. AS VARCHAR) || ' %'    AS Zinssatz, \
  CASE WHEN V.thesaurierend = 0 \
  THEN 'Auszahlend' \
  ELSE CASE WHEN V.thesaurierend = 1 \
       THEN 'Thesaur.' \
        ELSE CASE WHEN V.thesaurierend = 2 \
             THEN 'Fester Zins'  \
             ELSE 'ERROR'  \
             END \
        END \
  END AS Zinsmodus, \
 \
  CASE WHEN V.thesaurierend  = 0 THEN Zinsen_ausz \
  ELSE Zinsen_thesa  \
  END AS Zinsen, \
  CASE WHEN V.thesaurierend  > 0 \
  THEN -1* letzte_Auszahlung + sum_o_Zinsen \
  ELSE -1* letzte_Auszahlung + sum_mit_Zinsen \
  END AS Einlagen, \
 \
  maxValue AS Endauszahlung \
 \
FROM exVertraege AS V  INNER JOIN Kreditoren AS K ON V.KreditorId = K.id \
 \
LEFT JOIN ( \
     SELECT  \
        B.VertragsId AS vid_min,  \
    min(B.id) AS idErsteBuchung, \
    B.Datum AS Aktivierungsdatum,  \
    B.Betrag / 100. AS AktivierungsWert,  \
    sum(B.Betrag) / 100. AS GesamtWert \
     FROM exBuchungen AS B GROUP BY B.VertragsId ) \
ON V.id = vid_min \
 \
LEFT JOIN (SELECT  \
         B.VertragsId AS vid_max,  \
         max(B.id) as idLetzteBuchung, \
         B.Datum AS Vertragsende, \
         B.Betrag /100. AS maxValue \
       FROM exBuchungen AS B GROUP BY B.VertragsId )  \
ON V.id = vid_max \
 \
LEFT JOIN (SELECT \
            B.VertragsId AS vidZinsThesa, \
            SUM(B.betrag) /100. AS Zinsen_thesa \
          FROM exBuchungen AS B \
          WHERE B.BuchungsArt = 4 OR B.BuchungsArt = 8 \
          GROUP BY B.VertragsId ) \
ON V.id = vidZinsThesa \
 \
LEFT JOIN ( SELECT \
              B.VertragsId AS vidZinsAus, \
              SUM(B.betrag) /-100. AS Zinsen_ausz \
            FROM exBuchungen AS B \
            WHERE B.BuchungsArt = 1 OR B.BuchungsArt = 2 OR B.BuchungsArt = 8 GROUP BY B.VertragsId ) \
ON V.id = vidZinsAus \
 \
LEFT JOIN ( SELECT  \
                B.VertragsId as vid_o_Zinsen, \
                sum(B.Betrag) /100. AS sum_o_Zinsen \
            FROM exBuchungen AS B  \
            WHERE B.BuchungsArt = 1 OR B.BuchungsArt = 2 \
            GROUP BY B.VertragsId  ) \
ON V.id = vid_o_Zinsen \
 \
LEFT JOIN ( SELECT  \
              B.VertragsId as vid_mit_Zinsen, \
              sum(B.Betrag) /100. AS sum_mit_Zinsen, \
              max(B.Datum) AS letzte_buchung, \
              B.Betrag /100. AS letzte_Auszahlung \
            FROM exBuchungen AS B  \
            WHERE B.BuchungsArt = 1 OR B.BuchungsArt = 2 OR B.BuchungsArt = 8 \
            GROUP BY B.VertragsId  ) \
ON V.id = vid_mit_Zinsen \
")};

const QString sqlBookingsOverview {
    "SELECT B.Datum, V.id, V.Kennung, \
    CASE V.thesaurierend  \
    WHEN 0 THEN 'Ausz.' \
    WHEN 1 THEN 'Thesa.' \
    WHEN 2 THEN 'Fix' \
    ELSE 'ERROR' \
    END AS Zinsmodus, \
    B.Betrag, \
    CASE B.BuchungsArt \
    WHEN 1 THEN 'Einzahlung' \
    WHEN 2 THEN 'Auszahlung' \
    WHEN 4 THEN 'unterj.Zins' \
    WHEN 8 THEN 'Jahreszins' \
    ELSE 'ERROR' \
    END AS BuchungsArt \
     \
    FROM Vertraege AS V \
     \
    LEFT JOIN Buchungen AS B ON V.id = B.VertragsId \
    ORDER BY V.id, B.Datum"
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
{   LOG_CALL_W(qsl("filename: ") + filename);
    Q_ASSERT(!filename.isEmpty());
//    dbgTimer timer( QString(__func__) + QString(" (") + filename + QString(")"));
    if( QFile(filename).exists()) {
        backupFile(filename, "db-bak");
        QFile(filename).remove();
        if( QFile(filename).exists()) {
            qCritical() << "File to be replaced can not be deleted";
            return false;
        }
    }

    dbCloser closer{qsl("conCreateDb")};
    QSqlDatabase db = QSqlDatabase::addDatabase(qsl("QSQLITE"), closer.conName);
    db.setDatabaseName(filename);

    if( !db.open()) {
        qCritical() << "DkDatenbankAnlegen failed in db.open";
        return false;
    }
    return create_DK_TablesAndContent(db);
}
// database validation
bool has_allTablesAndFields(QSqlDatabase db)
{   LOG_CALL;
    for( auto& table : dkdbstructur.getTables()) {
        if( !verifyTable(table, db))
            return false;
    }
    qInfo() << db.databaseName() << " has all tables expected";
    return true;
}

bool check_db_version(QSqlDatabase db =QSqlDatabase::database())
{   LOG_CALL;
    double d =dbConfig::readValue(DB_VERSION, db).toDouble();
    if( d >= CURRENT_DB_VERSION)
        return true;
    qCritical() << "db version check failed: found version " << d << " needed version " << CURRENT_DB_VERSION;
    return false;
}

bool isValidDatabase(QSqlDatabase db =QSqlDatabase::database())
{
    LOG_CALL;
    {QSqlQuery enableRefInt(db);
    enableRefInt.exec("PRAGMA foreign_keys = ON");}
    if( !has_allTablesAndFields(db))
        return false;
    if( !check_db_version(db)) {
        qCritical() << "database version check failed";
        return false;
    }
    qInfo() << db.databaseName() << " is a valid dk database";
    return true;
}

bool isValidDatabase(const QString& filename)
{
    LOG_CALL_W(filename);
    QString msg;
    QString con(qsl("validate"));
    dbCloser closer(con);
    if( filename == "") msg = "no filename";
    else if( !QFile::exists(filename)) msg = "file not found";
    else {
        {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", con);
        db.setDatabaseName(filename);
        if( db.open()) {
            if( !isValidDatabase(db))
                msg = "database was found to be NOT valid";
        } else
            msg = "open db failed";
        } // End of db scope
    }
    if( msg.isEmpty())
        return true;
    qCritical() << msg;
    return false;
}


// initial database tables and content
void insert_DbProperties(QSqlDatabase db = QSqlDatabase::database())
{
    LOG_CALL;
    dbConfig::writeDefaults(db);
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

bool createView(QString name, QString sql, QSqlDatabase db = QSqlDatabase::database()) {
    db.transaction();
    QSqlQuery q(db);
    if( q.exec("DROP VIEW " + name))
        qInfo() << "Successfully dropped view " << name;
    else
        qInfo() << "Failed to drop view " << name << " Error: " << q.lastError() << Qt::endl << q.lastQuery();
    QString createViewSql = "CREATE VIEW %1 AS " + sql;
    createViewSql = createViewSql.arg(name);
    if( q.exec(createViewSql) && q.lastError().type() == QSqlError::NoError) {
        db.commit();
        qInfo() << "Successfully created view " << name << Qt::endl << sql;
        return true;
    }
    db.rollback();
    qCritical() << "Faild to create view " << name << Qt::endl << q.lastQuery() << Qt::endl << q.lastError() << Qt::endl << sql;
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
                                                          "SUM(Buchungen.Betrag) /100. AS Wert, "
                                                          "MIN(Buchungen.Datum)  AS Aktivierungsdatum, "
                                                          "Vertraege.Kfrist      AS Kuendigungsfrist, "
                                                          "Vertraege.LaufzeitEnde  AS Vertragsende, "
                                                          "Vertraege.thesaurierend AS thesa, "
                                                          "Kreditoren.Nachname || ', ' || Kreditoren.Vorname AS Kreditorin, "
                                                          "Kreditoren.id         AS KreditorId, "
                                                          "Kreditoren.Nachname   AS Nachname, "
                                                          "Kreditoren.Vorname    AS Vorname, "
                                                          "Kreditoren.Strasse    AS Strasse, "
                                                          "Kreditoren.Plz        AS Plz, "
                                                          "Kreditoren.Stadt      AS Stadt, "
                                                          "Kreditoren.Email      AS Email, "
                                                          "Kreditoren.IBAN       AS Iban, "
                                                          "Kreditoren.BIC        AS Bic "
                                                        "FROM Vertraege "
                                                            "INNER JOIN Buchungen  ON Buchungen.VertragsId = Vertraege.id "
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
        {qsl("vVertraege_alle_4view"),  sqlContractView},
        {qsl("vVertraege_geloescht"),   sqlExContractView},

        {qsl("vNextAnnualS_first"),      qsl("SELECT STRFTIME('%Y-%m-%d', MIN(Datum), '1 year', 'start of year', '-1 day')  as nextInterestDate "
                                            "FROM Buchungen INNER JOIN Vertraege ON Vertraege.id = buchungen.VertragsId "
                                            /* buchungen von Verträgen für die es keine Zinsbuchungen gibt */
                                            "WHERE (SELECT count(*) FROM Buchungen WHERE (Buchungen.BuchungsArt=4 OR Buchungen.BuchungsArt=8) AND Buchungen.VertragsId=Vertraege.id)=0 "
                                            "GROUP BY Vertraege.id ")},

        {qsl("vNextAnnualS_next"),       qsl("SELECT STRFTIME('%Y-%m-%d', MAX(Datum), '1 day', '1 year', 'start of year', '-1 day') as nextInterestDate "
                                            "FROM Buchungen INNER JOIN Vertraege ON Vertraege.id=buchungen.VertragsId "
                                            "WHERE Buchungen.BuchungsArt=4 OR Buchungen.BuchungsArt=8 "
                                            "GROUP BY Buchungen.VertragsId "
                                            "ORDER BY Datum ASC LIMIT 1")},
        {qsl("vNextAnnualSettlement"),    qsl("SELECT MIN(nextInterestDate) AS date FROM "
                                         "(SELECT nextInterestDate FROM vNextAnnualS_first "
                                         "UNION SELECT nextInterestDate FROM vNextAnnualS_next)")},
        {qsl("vContractsByYearByInterest"),qsl("SELECT SUBSTR(Vertraege.Vertragsdatum, 0, 5) as Year, "
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

    // for convenience only, not used in code
    views.append({qsl("vBuchungen"), sqlBookingsOverview});

    return createViews(views, db);
}

bool updateViews(QSqlDatabase db =QSqlDatabase::database())
{   LOG_CALL;
    QString lastProgramVersion = dbConfig::readValue(DKV2_VERSION).toString();
    QString thisProgramVersion = QCoreApplication::applicationVersion();
    if( lastProgramVersion != thisProgramVersion) {
        if( !insert_views(db)) {
            qDebug() << "Faild to insert views for current exe version";
            return false;
        }
        else {
            dbConfig::writeValue(DKV2_VERSION, thisProgramVersion);
            return true;
        }
    }
    return true;
}
// manage the app wide used database
void closeAllDatabaseConnections()
{   LOG_CALL;
    QList<QString> cl = QSqlDatabase::connectionNames();
    if( cl.count())
        qInfo()<< "Found " << cl.count() << "connections open";

    for( auto s : cl) {
        QSqlDatabase::database(s).close();
        QSqlDatabase::removeDatabase(s);
    }
    cl.clear();
    cl = QSqlDatabase::connectionNames();
    if( cl.size())
        qInfo() << "not all connection to the database could be closed";
    qInfo() << "All Database connections were removed";
}

bool open_databaseForApplication( QString newDbFile)
{   LOG_CALL_W(newDbFile);
    Q_ASSERT(!newDbFile.isEmpty());

    closeAllDatabaseConnections();
    backupFile(newDbFile, "db-bak");

    // setting the default database for the application
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(newDbFile);
    if( !db.open()) {
        qCritical() << "open database file " << newDbFile << " failed";
        return false;
    }

    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON");
    if( ! updateViews())
        return false;
    return true;
}

// db copy (w & w/o de-personalisation)
bool createCompatibleDatabaseFile(QString targetfn)
{
    if( QFile::exists(targetfn)) {
        backupFile(targetfn);
        QFile::remove(targetfn);
        if( QFile::exists(targetfn)) {
            qCritical() << "db_copy: could not remove target file";
            return false;
        }
    }
    QString conName(qsl("backup"));
    dbCloser closer(conName);
    QSqlDatabase backupDB = QSqlDatabase::addDatabase("QSQLITE", conName);
    backupDB.setDatabaseName(targetfn);
    if( !backupDB.open()) {
        qDebug() << "faild to open backup database";
        return false;
    }
    return dkdbstructur.createDb(backupDB);
}

bool copy_TableContent(QString table)
{   LOG_CALL_W(table);
    QString sql(qsl("INSERT OR REPLACE INTO targetDb.%1 SELECT * FROM %1"));
    return executeSql_wNoRecords(sql.arg(table));
}

bool replace_TableContent(QString table) {
    QSqlQuery deleteDefaultValues(qsl("DELETE FROM targetDb.") + table);
    return copy_TableContent(table);
}

bool copy_mangledCreditors()
{
    bool success = true;
    int recCount = 0;
    QSqlQuery q; // default database connection -> active database
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

bool create_DB_copy(QString targetfn, bool deper)
{   LOG_CALL_W(targetfn);
    autoRollbackTransaction trans;
    autoDetachDb ad;
    if( ! createCompatibleDatabaseFile(targetfn))
        return false;
    // Attach the new file to the current connection
    QString alias{qsl("targetDb")};
    if( ! ad.attachDb(targetfn, alias))
        return false;

    QVector<dbtable> tables = dkdbstructur.getTables();
    for( auto& table : qAsConst(tables)) {
        if( deper && table.Name() == qsl("Kreditoren")) {
            if( ! copy_mangledCreditors())
                return false;
        }
        else if (table.Name() == qsl("BriefElemente")) {
            if( ! replace_TableContent(table.Name()))
                return false;
        }
        else
            if( ! copy_TableContent(table.Name()))
                return false;
    }
    // copy the values from sqlite_sequence, so that autoinc works the same in both databases
    if( ! replace_TableContent(qsl("sqlite_sequence")))
        return false;
    // force views creation on next startup
    executeSql_wNoRecords(qsl("DELETE FROM targetDb.meta WHERE Name='dkv2.exe.Version'"));
    trans.commit();
    return true;
}

// general stuff
QString proposeContractLabel()
{   LOG_CALL;
    static int iMaxid = dbConfig::readValue(STARTINDEX).toInt() + getHighestRowId("Vertraege");
    QString kennung;
    do {
        QString maxid = QString::number(iMaxid).rightJustified(6, '0');
        QString PI = "DK-" + dbConfig::readValue(GMBH_INITIALS).toString();
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
    QString sql ="SELECT * FROM vContractsByYearByInterest ORDER BY Year";
    QSqlQuery query;
    query.exec(sql);
    while( query.next()) {
        QSqlRecord r =query.record();
        yzv.push_back({r.value("Year").toInt(), r.value("Zinssatz").toReal(),
                       r.value("Anzahl").toInt(), r.value("Summe").toReal() /100. });
    }
    return;
}
