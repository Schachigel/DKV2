#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVector>

#include "helper.h"
#include "helpersql.h"
#include "appconfig.h"
#include "csvwriter.h"
#include "creditor.h"
#include "contract.h"
#include "booking.h"
#include "investment.h"
#include "letterTemplate.h"
#include "dkdbviews.h"
#include "dkdbcopy.h"
#include "dbstructure.h"
#include "investment.h"

bool insertDKDB_Views( const QSqlDatabase &db)
{   LOG_CALL;
    return createDbViews(getViews(), db);
}// initial database tables and content

void insert_DbProperties(const QSqlDatabase &db = QSqlDatabase::database())
{
    LOG_CALL;
    dbConfig::writeDefaults(db);
}

bool fill_DkDbDefaultContent(const QSqlDatabase &db, bool includeViews /*=true*/, zinssusance sz /*=zs30360*/)
{
    LOG_CALL;
    switchForeignKeyHandling(db, true);
    bool ret = false;
    autoRollbackTransaction art(db.connectionName());
    do {
        if( includeViews) if ( not insertDKDB_Views(db)) break;
        insert_DbProperties(db);
        if ( not letterTemplate::insert_letterTypes(db)) break;
        if ( not letterTemplate::insert_elementTypes(db)) break;
        if ( not letterTemplate::insert_letterElements(db)) break;
        ret = true;
    } while (false);
    dbConfig::writeValue (ZINSUSANCE, (sz==zs30360) ? qsl("30/360"):qsl ("act/act"), db);
    if (ret)
        art.commit();
    return ret;
}

int get_db_version(const QString &file)
{
    LOG_CALL_W(file);
    dbCloser closer(qsl("db_version_check"));
    QSqlDatabase db =QSqlDatabase::addDatabase(dbTypeName, closer.conName);
    db.setDatabaseName(file);
    if( not db.open()) {
        qCritical() << "could not open db " << file << " for version check ";
        return noVersion;
    } else {
        return get_db_version(db);
    }
}

int get_db_version(const QSqlDatabase &db)
{   /*LOG_CALL;*/
    QVariant vversion =dbConfig::read_DBVersion(db);
    if( not (vversion.isValid() and vversion.canConvert(QMetaType::Double)))
        return noVersion; // big problem: no db
    int d =vversion.toInt();
    qInfo() << "DB Version Comparison: expected / found: " << CURRENT_DB_VERSION << " / " << d;
    return d;
}

// manage the app wide used database
void closeAllDatabaseConnections()
{   LOG_CALL;
    QList<QString> cl = QSqlDatabase::connectionNames();
    if( cl.count())
        qInfo()<< "Found " << cl.count() << "connections open";

    for( const auto &s : qAsConst(cl)) {
        QSqlDatabase::database(s).close();
        QSqlDatabase::removeDatabase(s);
    }
    cl.clear();
    cl = QSqlDatabase::connectionNames();
    if( cl.size())
        qInfo() << "not all connection to the database could be closed";
    qInfo() << "All Database connections were removed";
}

bool open_databaseForApplication( const QString &newDbFile)
{   LOG_CALL_W(newDbFile);
    Q_ASSERT( newDbFile.size());

    closeAllDatabaseConnections();
    backupFile(newDbFile, qsl("db-bak"));

    // setting the default database for the application
    QSqlDatabase db = QSqlDatabase::addDatabase(dbTypeName);
    db.setDatabaseName(newDbFile);
    if( not db.open()) {
        qCritical() << "open database file " << newDbFile << " failed";
        return false;
    }

    switchForeignKeyHandling(db, fkh_on);
    if( not insertDKDB_Views ())
        return false;
    return true;
}

// general stuff
bool isExistingContractLabel( const QString& newLabel)
{
    QVariant existingLabel =executeSingleValueSql(contract::fnKennung, contract::tnContracts, qsl("Kennung = '") +newLabel +qsl("'"));
    return existingLabel.isValid();
}

bool isExistingExContractLabel( const QString& newLabel)
{
    QVariant existingLabel =executeSingleValueSql(contract::fnKennung, contract::tnExContracts, qsl("Kennung = '") +newLabel +qsl("'"));
    return existingLabel.isValid();
}

bool isValidNewContractLabel(const QString& label)
{
    return ( not isExistingContractLabel(label)) and ( not isExistingExContractLabel(label));
}

QString proposeContractLabel()
{   LOG_CALL;
    static int iMaxid = dbConfig::readValue(STARTINDEX).toInt() + getHighestRowId(contract::tnContracts);
    QString kennung;
    do {
        QString maxid = QString::number(iMaxid).rightJustified(6, '0');
        QString PI = qsl("DK-") + dbConfig::readValue(GMBH_INITIALS).toString();
        kennung = PI + qsl("-") + QString::number(QDate::currentDate().year()) + qsl("-") + maxid;
        if( isValidNewContractLabel(kennung))
            break;
        else
            iMaxid++;
    } while(1);
    iMaxid++; // prepare for next contract
    return kennung;
}

int createNewInvestmentsFromContracts()
{   LOG_CALL;
    QString sql{qsl("SELECT ZSatz, Vertragsdatum FROM Vertraege WHERE AnlagenId IS NULL OR AnlagenId <= 0 ORDER BY Vertragsdatum ASC ")};
    QSqlQuery q; q.setForwardOnly(true);
    if( not q.exec(sql)) {
        qCritical() << "query execute faild with error " << q.lastError();
        qDebug() << q.lastQuery();
        return -1;
    }
    int ret =0;
    while(q.next()) {
        int ZSatz =q.record().value(qsl("ZSatz")).toInt();
        QDate vDate =q.record().value(qsl("Vertragsdatum")).toDate();
        if( 0 < createInvestmentFromContractIfNeeded(ZSatz, vDate))
            ret++;
    }
    return ret;
}

int automatchInvestmentsToContracts()
{   LOG_CALL;
    QString sql{qsl("SELEcT id, ZSatz, Vertragsdatum, AnlagenId FROM Vertraege WHERE AnlagenId =0 OR AnlagenId IS NULL")};
    QSqlQuery q;
    if( not q.exec(sql)) {
        qDebug() << "failed to exec query";
        return -1;
    }
    int successcount =0;
    while(q.next()) {
        int interestRate   =q.record().value(qsl("ZSatz")).toInt();
        QDate contractDate =q.record().value(qsl("Vertragsdatum")).toDate();
        QVector<investment> suitableInvestments =openInvestments(interestRate, contractDate);
        if( suitableInvestments.length() not_eq 1)
            continue;
        contract c(q.record().value(qsl("id")).toLongLong());
        if( c.updateInvestment(suitableInvestments[0].rowid))
            successcount++;
        else
            qDebug() << "contract update failed";
    }
    return successcount;
}

void create_sampleData(int datensaetze)
{
    saveRandomCreditors(datensaetze);
    saveRandomContracts(datensaetze);
    activateRandomContracts(90);

}
bool createCsvActiveContracts()
{   LOG_CALL;
    QString tempViewContractsCsv {qsl("tvActiveContractsCsv")};
    if( not createTemporaryDbView(tempViewContractsCsv, sqlContractsActiveDetailsView)) {
        qCritical() << "failed to create view " << tempViewContractsCsv;
        return false;
    }
    dbtable t(tempViewContractsCsv);
    t.append(dbfield(contract::fnId, QVariant::Type::Int));
    t.append(dbfield(creditor::fnId, QVariant::Type::Int));
    t.append(dbfield(creditor::fnVorname));
    t.append(dbfield(creditor::fnNachname));
    t.append(dbfield(creditor::fnStrasse));
    t.append(dbfield(creditor::fnPlz));
    t.append(dbfield(creditor::fnStadt));
    t.append(dbfield(creditor::fnEmail));
    t.append(dbfield(creditor::fnIBAN));
    t.append(dbfield(creditor::fnBIC));
//    t.append(dbfield(creditor::fnStrasse));
    t.append(dbfield(contract::fnZSatz, QVariant::Type::Double));
    t.append(dbfield(qsl("Wert"), QVariant::Type::Double));
    t.append(dbfield(qsl("Aktivierungsdatum"), QVariant::Type::Date));
    t.append(dbfield(qsl("Kuendigungsfrist"), QVariant::Type::Int));
    t.append(dbfield(qsl("Vertragsende"), QVariant::Type::Date));
    t.append(dbfield(qsl("thesa"), QVariant::Type::Bool));

    QString filename(QDate::currentDate().toString(Qt::ISODate) + "-Aktive-Vertraege.csv");
    bool res =table2csv( filename, t.Fields());
    if( not res)
        qDebug() << "failed to print table";

    return res;
}

// calculate data for start page
double valueOfAllContracts()
{
    QString sqlInactive {qsl(R"str(
SELECT SUM(Betrag)/100. AS Gesamtbetrag
FROM Vertraege
WHERE id NOT IN (SELECT DISTINCT VertragsId FROM Buchungen)
)str")};
    double inactiveSum =executeSingleValueSql(sqlInactive).toDouble();
    QString sqlActive {qsl(R"str(
SELEcT SUM(Betrag) /100. as Gesamtbetrag
FROM Buchungen
)str")};
    double activeSum =executeSingleValueSql(sqlActive).toDouble();
    return inactiveSum + activeSum;
}

// statistics, overviews
// format data for Uebersicht -> kurzinfo
QVector<QStringList> overviewShortInfo(const QString& sql)
{
    QVector<QStringList> ret;
    QLocale locale;
    QSqlRecord record =executeSingleRecordSql(sql);
    ret.push_back(QStringList({qsl("Anzahl DK Geber*innen"), record.value(qsl("AnzahlKreditoren")).toString()}));
    ret.push_back(QStringList({qsl("Anzahl der Verträge"), record.value(qsl("AnzahlVertraege")).toString()}));
    ret.push_back(QStringList({qsl("Gesamtvolumen"), locale.toCurrencyString(record.value(qsl("GesamtVolumen")).toDouble())}));
    ret.push_back(QStringList({qsl("Mittlerer Vertragswert"), locale.toCurrencyString(record.value(qsl("MittlererVertragswert")).toDouble())}));
    ret.push_back(QStringList({qsl("Jahreszins"), locale.toCurrencyString(record.value(qsl("JahresZins")).toDouble())}));
    ret.push_back(QStringList({qsl("Durchschn. Zins (gew. Mittel)"), qsl("%1 %").arg(r2(record.value(qsl("ZinsRate")).toDouble()))}));
    ret.push_back(QStringList({qsl("Mittlerer Zins"), qsl("%1 %").arg(r2(record.value(qsl("MittelZins")).toDouble()))}));

    return ret;
}
// calc runtime of contracts for bookkeeper
QVector<contractRuntimeDistrib_rowData> contractRuntimeDistribution()
{   LOG_CALL;
    int AnzahlBisEinJahr=0, AnzahlBisFuenfJahre=0, AnzahlLaenger=0, AnzahlUnbegrenzet = 0;
    double SummeBisEinJahr=0., SummeBisFuenfJahre=0., SummeLaenger=0., SummeUnbegrenzet = 0.;
    QString sql = qsl("SELECT Wert, Aktivierungsdatum, Vertragsende "
                  "FROM (%1)").arg(sqlContractsActiveView);
    QSqlQuery q; q.setForwardOnly(true);
    if( not q.exec(sql)) {
        qCritical() << "calculation of runtime distribution failed: " << q.lastError() << Qt::endl << q.lastQuery();
        return QVector<contractRuntimeDistrib_rowData>();
    }

    while( q.next()) {
        double wert =   q.value(qsl("Wert")).toReal();
        QDate von = q.value(qsl("Datum")).toDate();
        QDate bis = q.value(qsl("Vertragsende")).toDate();
        if( not bis.isValid() or bis == EndOfTheFuckingWorld) {
            AnzahlUnbegrenzet++;
            SummeUnbegrenzet += wert;
        } else if( bis > von.addYears(1) and bis < von.addYears(5)) {
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
    QVector<contractRuntimeDistrib_rowData> ret;
    // .ret.push_back({"Zeitraum", "Anzahl", "Wert"});
    ret.push_back({"Bis ein Jahr ", QString::number(AnzahlBisEinJahr), locale.toCurrencyString(SummeBisEinJahr)});
    ret.push_back({"Ein bis fünf Jahre ", QString::number(AnzahlBisFuenfJahre), locale.toCurrencyString(SummeBisFuenfJahre)});
    ret.push_back({"Länger als fünf Jahre ", QString::number(AnzahlLaenger), locale.toCurrencyString(SummeLaenger) });
    ret.push_back({"Unbegrenzte Verträge ", QString::number(AnzahlUnbegrenzet), locale.toCurrencyString(SummeUnbegrenzet) });
    return ret;
}

QVector<QStringList> perpetualInvestmentCheck()
{
    QString sql {qsl(R"str(
    WITH Abschluesse AS (
      SELECT G.rowid    AS AnlageId
        , G.Typ         AS Anlage
        , V.Kennung          AS Vertrag
        , V.Vertragsdatum AS Datum
        , V.Betrag        AS Betrag
      FROM Vertraege AS V
      INNER JOIN Geldanlagen AS G ON G.rowid = V.AnlagenId
    )
    SELECT Abschluesse.Datum AS Vertragsdatum
    , Abschluesse.Vertrag
    , DATE(Abschluesse.Datum, '-1 years') AS periodenStart
    , Abschluesse.Anlage AS Anlage
    , sum(Abschluesse.Betrag/100.) AS AnlageSumme
    , Abschluesse.Betrag/100. AS Anlagebetrag
    , (SELECT SUM(Betrag)/100.
      FROM ( SELECT Betrag
             FROM Abschluesse AS _ab
             WHERE _ab.AnlageId = Abschluesse.AnlageId
               AND _ab.Datum > DATE(Abschluesse.Datum, '-1 years')
               AND _ab.Datum <= Abschluesse.Datum
           )
      ) AS periodenSumme
    FROM Abschluesse
    GROUP BY Datum, AnlageId
    ORDER BY Datum DESC, Anlage ASC
    )str")};

    QVector<QSqlRecord> rec;
    if( not executeSql (sql, rec)) {
        return QVector<QStringList>();
    }
    QLocale l;
    QVector<QStringList> result;
    for( int i=0; i< rec.size (); i++) {
        QStringList zeile;
        zeile.push_back (rec[i].value(0).toDate().toString("dd.MM.yyyy"));
        zeile.push_back (rec[i].value(1).toString()); // Kennung
        zeile.push_back (rec[i].value(2).toDate().toString ("dd.MM.yyyy"));
        zeile.push_back (rec[i].value(3).toString()); // Geldanlage Typ
        zeile.push_back (l.toCurrencyString (rec[i].value(4).toDouble ())); // Kreditvol.
        zeile.push_back (l.toCurrencyString (rec[i].value(5).toDouble ())); // Summe über Periode
        result.push_back (zeile);
    }
    return result;
}

// calc how many contracts end each year
void calc_contractEnd( QVector<contractEnd_rowData>& ces)
{   LOG_CALL;
    QString sql {qsl("SELECT count(*) AS Anzahl, sum(Wert) AS Wert, strftime('%Y',Vertragsende) AS Jahr "
             "FROM (%1) "
             "WHERE Vertragsende < 9999-01-01 GROUP BY strftime('%Y',Vertragsende) ORDER BY Jahr").arg(sqlContractsActiveView)};
    QSqlQuery q; q.setForwardOnly(true);
    if( q.exec(sql)) {
        while( q.next()) {
            ces.push_back({q.value("Jahr").toInt(), q.value("Anzahl").toInt(), q.value("Wert").toDouble()});
        }
    } else
        qCritical() << "sql exec failed";
    return;
}

void getBookingDateInfoBySql(const QString &sql, QVector<BookingDateData>& dates)
{
    QVector<QSqlRecord> records;
    if( not executeSql(sql, records)) {
        qInfo() << "getDatesBySql: no dates to found";
        return;
    }
    for (const auto &rec : qAsConst(records)) {
        dates.push_back({rec.value(0).toInt(), rec.value(1).toString(), rec.value(2).toDate()});
    }
    qInfo() << "getDatesBySql added " << dates.size() << " dates to the vector";
}

// get dates for statistic page
void getActiveContracsBookingDates( QVector<BookingDateData>& dates)
{
    // Statistic of active contracts changes by
    // contract activation, contract termination and all other bookings
    // from Buchungen and exBuchungen
    QString sql {qsl(R"str(
WITH allBookings AS
(
  SELECT Buchungen.BuchungsArt
    , Datum
  FROM Buchungen
    UNION ALL
  SELECT exBuchungen.BuchungsArt
    , Datum
  FROM exBuchungen
)
SELECT COUNT(*) AS Anzahl
  , BuchungsArt AS Typ
  , Datum
FROM allBookings
GROUP BY Datum
ORDER BY Datum DESC
)str")};
    return getBookingDateInfoBySql(sql, dates);
}

void getInactiveContractBookingDates( QVector<BookingDateData>& dates)
{
    // statistics of inactive contracts change by
    // contract conclusion and contract activation
    QString sql {qsl(R"str(
WITH allDatesInactiveContracts AS (
  SELECT Vertragsdatum AS Datum
    , 'VD' as Typ
  FROM Vertraege
  GROUP BY Datum
    UNION ALL
  SELECT Vertragsdatum AS Datum
    , 'VDex' as Typ
  FROM exVertraege
  GROUP BY Datum
    UNION ALL
  SELECT MIN(Datum) AS Datum
    , 'AD' as Typ
  FROM Buchungen
  GROUP BY VertragsId
      UNION ALL
  SELECT MIN(Datum) AS Datum
    , 'ADex' as Typ
  FROM exBuchungen
  GROUP BY VertragsId
)
SELECT count(*) AS Anzahl, Typ, Datum
FROM allDatesInactiveContracts
GROUP BY Datum
ORDER BY Datum DESC
)str")};
    return getBookingDateInfoBySql(sql, dates);
}

void getFinishedContractBookingDates( QVector<BookingDateData>& dates) {
    // dates at which a contract was finished
    QString sql{qsl(R"str(
    SELECT COUNT(VertragsId) AS Anzahl
      , 'CT'
      , Datum
    FROM (
    SELECT VertragsId
      , MAX(exBuchungen.Datum) AS Datum
    FROM exBuchungen
    GROUP BY VertragsId
    )
    GROUP BY Datum
    ORDER BY Datum DESC
    )str")};
    return getBookingDateInfoBySql(sql, dates);
}

void getAllContractBookingDates( QVector<BookingDateData>& dates)
{
    // statistics of all contracts changes by
    // contract conclusions and all kinds of bookings
    // in Buchungen and exBuchungen
    QString sql {qsl(R"str(
  SELECT COUNT(*) AS Anzahl
    , 'VD' AS Typ
    , Vertragsdatum AS Datum
  FROM Vertraege
  GROUP BY Datum
UNION ALL
  SELECT COUNT(*) AS Anzahl
    , 'VDex' AS Typ
    , Vertragsdatum AS Datum
  FROM exVertraege
  GROUP BY Datum
UNION ALL
  -- alle Buchungen laufender Verträge
  SELECT COUNT(*) AS Anzahl
    , BuchungsArt AS Typ
    , Datum
  FROM Buchungen
  GROUP BY Datum
UNION ALL
  -- alle Buchungen beendeter Verträge
  SELECT COUNT(*) AS Anzahl
    , BuchungsArt AS Typ
    , Datum
  FROM exBuchungen
  GROUP BY Datum
  ORDER BY Datum DESC
)str")};
    return getBookingDateInfoBySql(sql, dates);
}
