#include "helperfile.h"
#include "helpersql.h"
#include "appconfig.h"
#include "dkv2version.h"
#include "busycursor.h"
#include "csvwriter.h"
#include "contract.h"
#include "creditor.h"
#include "investment.h"
#include "dkdbviews.h"
#include "dkdbindices.h"
#include "dkdbcopy.h"
#include "dkdbhelper.h"


namespace  {

bool updateViewsAndIndices_if_needed(const QSqlDatabase& db =QSqlDatabase::database ())
{   LOG_CALL;
    QString lastProgramVersion = dbConfig::read_DKV2_Version(db);
    QString thisProgramVersion = QCoreApplication::applicationVersion();
    if( lastProgramVersion == thisProgramVersion) {
        qInfo() << "Same Program Version -> no update of views and indices necessary";
        return true;
    }
    qInfo() << "Program versions used differ -> views will be updated";
    qInfo() << qsl("last exe: ") << lastProgramVersion << qsl(" / this exe: ") << thisProgramVersion;
    QString errorMsg;
    if( not insertDKDB_Views (db)) {
        errorMsg.append(qsl("Faild to insert views for current exe version"));
    }
    if( not insertDKDB_Indices (db)) {
        errorMsg.append (qsl("\nFailed to insert indices for current exe version"));
    }
    if( errorMsg.isEmpty ()) {
        dbConfig::writeValue(DKV2_VERSION, thisProgramVersion);
        return true;
    }
    return false;
}

void insert_DbProperties(const QSqlDatabase &db = QSqlDatabase::database())
{
    LOG_CALL;
    dbConfig::writeDefaults(db);
}

int get_db_version(const QSqlDatabase &db)
{   /*LOG_CALL;*/
    QVariant vversion =dbConfig::read_DBVersion(db);
    if( not (vversion.isValid() and vversion.canConvert<double>()))
        return noVersion; // big problem: no db
    int d =vversion.toInt();
    qInfo() << "DB Version Comparison: expected / found: " << CURRENT_DB_VERSION << " / " << d;
    return d;
}

void getBookingDateInfoBySql(const QString &sql, QVector<BookingDateData>& dates)
{
    QVector<QSqlRecord> records;
    if( not executeSql(sql, records)) {
        qInfo() << "getDatesBySql: no dates to found";
        return;
    }
    for (const auto &rec : std::as_const(records)) {
        dates.push_back({rec.value(0).toInt(), rec.value(1).toString(), rec.value(2).toDate()});
    }
    qInfo() << "getDatesBySql added " << dates.size() << " dates to the vector";
}

bool postDB_UpgradeActions(int /*sourceVersion*/, const QString & dbName)
{
    autoDb db(dbName, qsl("postUpgradeActions"));
    bool ret = true;
    //  do stuff to adapt to new db depending on the source version
    QVector<QString> updates {
        // make sure the views are rewritten
        qsl("DELETE FROM Meta WHERE Name = 'dkv2.exe.Version'"),
        // set strings, which might become concatenated in SQL to empty but not NULL
        qsl("UPDATE Kreditoren SET Vorname  = '' WHERE Vorname  IS NULL"),
        qsl("UPDATE Kreditoren SET Nachname = '' WHERE Nachname IS NULL"),
        qsl("UPDATE Kreditoren SET Strasse  = '' WHERE Strasse  IS NULL"),
        qsl("UPDATE Kreditoren SET Plz      = '' WHERE Plz      IS NULL"),
        qsl("UPDATE Kreditoren SET Stadt    = '' WHERE Stadt    IS NULL"),
        qsl("UPDATE Geldanlagen SET Typ     = '' WHERE Typ      IS NULL")
        // other updates...
    };
    for(const auto & sql: std::as_const(updates)) {
        QVector<QVariant> params;
        executeSql_wNoRecords (sql, params, db);
    }
    return ret;
}

}

bool treat_DbIsAlreadyInUse_File(QString filename)
{
    QMessageBox::StandardButton result = QMessageBox::NoButton;

    while (checkSignalFile(filename))
    {
        result = QMessageBox::information((QWidget *)nullptr, qsl("Datenbank bereits geöffnet?"),
                                 qsl("Es scheint, als sei die Datenbank\n%1\n bereits geöffnet. Das kommt vor, "
                                     "wenn DKV2 abgestürzt ist oder bereits läuft.\n"
                                     "Falls die Datenbank auf einem Fileserver läuft, kann auch eine "
                                     "andere Benutzerin die Datenbank gerade verwenden.\n"
                                     "\nIgnore: Wenn du sicher bist, dass kein anderes "
                                     "Programm läuft. (auf eigene Gefahr!)"
                                     "\nCancel: Um eine andere Datenbank zu wählen."
                                     "\nRetry: Wenn das andere Programm beendet ist."
                                     " ").arg(filename),
                                 QMessageBox::Cancel | QMessageBox::Retry | QMessageBox::Ignore);

        if (result == QMessageBox::Cancel)  {
            return false;
        } else if (result == QMessageBox::Ignore) {
            /* QMessageBox::Ignore leaves the file check loop */
            break;
        } else {
            /* QMessageBox::Retry and other repeats the file check */
        }
    }
    createSignalFile (filename);
    return true;
}

bool insertDKDB_Views( const QSqlDatabase &db)
{   LOG_CALL;
    return createDkDbViews (views, db);
}// initial database tables and content

bool insertDKDB_Indices( const QSqlDatabase& db)
{
    return createDkDbIndices( db);
}

bool fill_DkDbDefaultContent(const QSqlDatabase &db, bool includeViews /*=true*/, zinssusance sz /*=zs30360*/)
{
    LOG_CALL;
    switchForeignKeyHandling(true, db);
    bool ret = false;
    autoRollbackTransaction art(db.connectionName());
    do {
        if( includeViews) if ( not insertDKDB_Views(db)) break;
        if( includeViews) if (not insertDKDB_Indices(db)) break;
        insert_DbProperties(db);
        ret = true;
    } while (false);
    dbConfig::writeValue (ZINSUSANCE, (sz==zs_30360) ? qsl("30/360"):qsl ("act/act"), db);
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

bool checkSchema_ConvertIfneeded(const QString &origDbFile)
{
    LOG_CALL;
    busycursor bc;
    int version_of_original_file = get_db_version(origDbFile);
    if (version_of_original_file < CURRENT_DB_VERSION)
    {
        qInfo() << "lower version -> converting";
        bc.finish (); // normal cursor during message box
        if (QMessageBox::Yes not_eq QMessageBox::question(getMainWindow(), qsl("Achtung"), qsl("Das Format der Datenbank \n%1\nist veraltet.\nSoll die Datenbank konvertiert werden?").arg(origDbFile)))
            RETURN_OK(false, qsl("conversion rejected by user"));

        QString backup = convert_database_inplace(origDbFile);
        if (backup.isEmpty()) {
            bc.finish (); // normal cursor during message box
            QMessageBox::critical(getMainWindow(), qsl("Fehler"), qsl("Bei der Konvertierung ist ein Fehler aufgetreten. Die Ausführung muss beendet werden."));
            RETURN_ERR(false, qsl("db converstion of older DB failed"));
        }
        // actions which depend on the source version of the db
        if (not postDB_UpgradeActions(version_of_original_file, origDbFile)) {
            bc.finish (); // normal cursor during message box
            QMessageBox::critical(getMainWindow(), qsl("Fehler"), qsl("Bei der Konvertierung ist ein Fehler aufgetreten. Die Ausführung muss beendet werden."));
            RETURN_ERR(false, qsl("db converstion of older DB failed"));
        }
        bc.finish ();
        QMessageBox::information(nullptr, qsl("Erfolgsmeldung"), qsl("Die Konvertierung ware erfolgreich. Eine Kopie der ursprünglichen Datei liegt unter \n") + backup);
        return true;
    }

    if (version_of_original_file == CURRENT_DB_VERSION)
        return validateDbSchema(origDbFile, dkdbstructur);

    // VERSION is higher?!
    RETURN_ERR(false, qsl("higher version ? there is no way back"));
}

bool open_databaseForApplication( const QString &newDbFile)
{   LOG_CALL_W(newDbFile);
    Q_ASSERT( newDbFile.size());

    closeAllDatabaseConnections();
    backupFile(newDbFile, qsl("db-bak"));

    // setting the default database for the application
    QSqlDatabase db = QSqlDatabase::addDatabase(dbTypeName);
    db.setDatabaseName(newDbFile);
    if( not db.open())
        RETURN_ERR(false, qsl("open database file failed:"), newDbFile);
    if( not updateViewsAndIndices_if_needed (db))
        RETURN_ERR(false, qsl("update views failed on "), newDbFile);
    switchForeignKeyHandling(fkh_on, db);
    return true;
}

bool isValidNewContractLabel(const QString& newLabel)
{
    QVariant existingLabel =executeSingleValueSql(contract::fnKennung, contract::tnContracts, qsl("Kennung = ") +singleQuoted (newLabel ));
    QVariant existingExLabel =executeSingleValueSql(contract::fnKennung, contract::tnExContracts, qsl("Kennung = ") +singleQuoted (newLabel));

    return not (existingLabel.isValid () or existingExLabel.isValid ());
}

QString proposeContractLabel()
{   LOG_CALL;
    qlonglong nextId = dbConfig::readValue(STARTINDEX).toInt() + getHighestRowId(contract::tnContracts);
    QString kennung;
    do {
        QString maxid = i2s(nextId).rightJustified(6, '0');
        QString PI = qsl("DK-") + dbConfig::readValue(GMBH_INITIALS).toString();
        kennung = PI + qsl("-") + i2s(QDate::currentDate().year()) + qsl("-") + maxid;
        if( isValidNewContractLabel(kennung))
            break;
        else
            nextId++;
    } while(1);
    return kennung;
}

int createNewInvestmentsFromContracts( bool fortlaufend)
{   LOG_CALL;
    QString sql{qsl("SELECT ZSatz, Vertragsdatum FROM Vertraege WHERE AnlagenId IS NULL OR AnlagenId <= 0 ORDER BY Vertragsdatum ASC ")};
    QVector<QSqlRecord> res;
    if( not executeSql (sql, res))
        RETURN_ERR( -1, QString(__FUNCTION__), qsl("Info aus Verträgen konnte nicht gelesen werden"));

    int ret =0;
    for( const QSqlRecord& rec : std::as_const(res)) {
        int ZSatz =rec.value(qsl("ZSatz")).toInt();
        QDate vDate =rec.value(qsl("Vertragsdatum")).toDate();
        if( fortlaufend)
            vDate =BeginingOfTime;
        if( 0 < createInvestmentFromContractIfNeeded(ZSatz, vDate))
            ret++;
    }
    return ret;
}

int automatchInvestmentsToContracts()
{   LOG_CALL;
    QString sql{qsl("SELEcT id, ZSatz, Vertragsdatum, AnlagenId FROM Vertraege WHERE AnlagenId =0 OR AnlagenId IS NULL")};
    QVector<QSqlRecord> res;
    if( not executeSql (sql, res))
        RETURN_ERR( -1, QString(__FUNCTION__), qsl("Info aus Verträgen konnte nicht gelesen werden"));

    int successcount =0;
    for( const QSqlRecord& rec : std::as_const(res)) {
        int interestRate   =rec.value(qsl("ZSatz")).toInt();
        QDate contractDate =rec.value(qsl("Vertragsdatum")).toDate();
        QVector<investment> suitableInvestments =openInvestments(interestRate, contractDate);
        if( suitableInvestments.length() not_eq 1)
            continue; // no match if ambiguous or not existing
        contract c(rec.value(qsl("id")).toLongLong());
        if( c.updateInvestment(suitableInvestments[0].rowid))
            successcount++;
        else
            qCritical() << "contract update failed";
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
    if( not createTemporaryDbView(tempViewContractsCsv, sqlContractsActiveDetailsView))
        RETURN_ERR( false, qsl("failed to create view "), tempViewContractsCsv);

    QStringList header;
    dbtable t(tempViewContractsCsv);
    t.append(dbfield(contract::fnId, QMetaType::Int));
    header.append(qsl("Vertragsnummer"));
    t.append(dbfield(creditor::fnId, QMetaType::Int));
    header.append (qsl("Kundennummer"));
    t.append(dbfield(creditor::fnVorname));
    header.append (qsl("Vorname"));
    t.append(dbfield(creditor::fnNachname));
    header.append (qsl("Nachname"));
    t.append(dbfield(creditor::fnStrasse));
    header.append (qsl("Strasse"));
    t.append(dbfield(creditor::fnPlz));
    header.append (qsl("PLZ"));
    t.append(dbfield(creditor::fnStadt));
    header.append (qsl("Stadt"));
    t.append(dbfield(creditor::fnEmail));
    header.append (qsl("E-Mail"));
    t.append(dbfield(creditor::fnIBAN));
    header.append (qsl("IBAN"));
    t.append(dbfield(creditor::fnBIC));
    header.append (qsl("BIC"));
    t.append(dbfield(qsl("Zinssatz"), QMetaType::Double));
    header.append (qsl("Zinssatz"));
    t.append(dbfield(qsl("Wert"), QMetaType::Double));
    header.append (qsl("Vertragswert"));
    t.append(dbfield(qsl("Aktivierungsdatum"), QMetaType::QDate));
    header.append (qsl("Aktivierungsdatum"));
    t.append(dbfield(qsl("Kuendigungsfrist"), QMetaType::Int));
    header.append (qsl("Kündigungsfrist"));
    t.append(dbfield(qsl("Vertragsende"), QMetaType::QDate));
    header.append (qsl("Vertragsende"));
    t.append(dbfield(qsl("thesa"), QMetaType::Int));
    header.append (qsl("Zinsmodus"));

    QVector<QSqlRecord> qResult =executeSql (t.Fields ());

    QVector<QStringList> data;
    for(const auto& record : std::as_const( qResult)) {
        QStringList col;
        col.append (record.value(contract::fnId).toString());
        col.append (record.value(creditor::fnId).toString());
        col.append (record.value(creditor::fnVorname).toString());
        col.append (record.value(creditor::fnNachname).toString());
        col.append (record.value(creditor::fnStrasse).toString());
        col.append (record.value(creditor::fnPlz).toString());
        col.append (record.value(creditor::fnStadt).toString());
        col.append (record.value(creditor::fnEmail).toString());
        col.append (record.value(creditor::fnIBAN).toString());
        col.append (record.value(creditor::fnBIC).toString());
        col.append (prozent2prozent_str (record.value(qsl("Zinssatz")).toDouble ()));
        col.append (s_d2euro (record.value(qsl("Wert")).toDouble ()));
        col.append (record.value(qsl("Aktivierungsdatum")).toDate().toString ("dd.MM.yyyy"));
        col.append (record.value(qsl("Kuendigungsfrist")).toString());
        col.append (record.value(qsl("Vertragsende")).toDate().toString ("dd.MM.yyyy"));
        col.append (interestModelDisplayString(interestModelFromInt(record.value(qsl("thesa")).toInt())));
        data.append(col);
    }

    QString filename(QDate::currentDate().toString(Qt::ISODate) + "-Aktive-Vertraege.csv");
    if( StringLists2csv( filename, header, data))
        return true;
    RETURN_ERR(false, qsl("failed to print table"));
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
    QSqlRecord record =executeSingleRecordSql(sql);
    ret.push_back(QStringList({qsl("Anzahl DK Geber*innen"), record.value(qsl("AnzahlKreditoren")).toString()}));
    ret.push_back(QStringList({qsl("Anzahl der Verträge"), record.value(qsl("AnzahlVertraege")).toString()}));
    ret.push_back(QStringList({qsl("Gesamtvolumen"), s_d2euro(record.value(qsl("GesamtVolumen")).toDouble())}));
    ret.push_back(QStringList({qsl("Mittlerer Vertragswert"), s_d2euro(record.value(qsl("MittlererVertragswert")).toDouble())}));
    ret.push_back(QStringList({qsl("Jahreszins"), s_d2euro(record.value(qsl("JahresZins")).toDouble())}));
    ret.push_back(QStringList({qsl("Durchschn. Zins (gew. Mittel)"), qsl("%1 %").arg(r2(record.value(qsl("ZinsRate")).toDouble()))}));
    ret.push_back(QStringList({qsl("Mittlerer Zins"), qsl("%1 %").arg(r2(record.value(qsl("MittelZins")).toDouble()))}));

    return ret;
}
// calc runtime of contracts for bookkeeper
QVector<contractRuntimeDistrib_rowData> contractRuntimeDistribution()
{   LOG_CALL;
    int AnzahlBisEinJahr=0, AnzahlBisFuenfJahre=0, AnzahlLaenger=0, AnzahlUnbegrenzet = 0;
    double SummeBisEinJahr=0., SummeBisFuenfJahre=0., SummeLaenger=0., SummeUnbegrenzet = 0.;

    QString tname {qsl("activeContracts")};
    createTemporaryDbView (tname, sqlContractsActiveView);
    dbtable t(tname);
    t.append (dbfield(qsl("Wert"), QMetaType::Type::Double));
    t.append (dbfield(qsl("Aktivierungsdatum"), QMetaType::QDate));
    t.append (dbfield(qsl("Vertragsende"), QMetaType::QDate));

    QVector<QSqlRecord> records =executeSql(t.Fields ());
    if( records.empty ())
        RETURN_ERR(QVector<contractRuntimeDistrib_rowData>(), qsl("calculation of runtime distribution failed"));

    for( const auto& record : std::as_const(records)) {
        double wert =record.value(qsl("Wert")).toReal();
        QDate   von =record.value(qsl("Datum")).toDate();
        QDate   bis =record.value(qsl("Vertragsende")).toDate();
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
    QVector<contractRuntimeDistrib_rowData> ret;
    // .ret.push_back({"Zeitraum", "Anzahl", "Wert"});
    ret.push_back({"Bis ein Jahr ",          i2s(AnzahlBisEinJahr), s_d2euro(SummeBisEinJahr)});
    ret.push_back({"Ein bis fünf Jahre ",    i2s(AnzahlBisFuenfJahre), s_d2euro(SummeBisFuenfJahre)});
    ret.push_back({"Länger als fünf Jahre ", i2s(AnzahlLaenger), s_d2euro(SummeLaenger) });
    ret.push_back({"Unbegrenzte Verträge ",  i2s(AnzahlUnbegrenzet), s_d2euro(SummeUnbegrenzet) });
    return ret;
}

namespace {
QString decorateHighValues(double d)
{
    QString ret =s_d2euro (d);
    if (d >=80000.) {
        if (d >=90000.) {
            if (d >=100000.) {
                return qsl("<font color='red'><b>%1</b></font>").arg(ret);
            }
            // 90t - 100t
            return qsl("<font color='red'>%1</font>").arg(ret );
        }
        // 80t - 90t
        return qsl("<b>%1</b>").arg(ret);
    }
    return ret;
}
}

QVector<QStringList> perpetualInvestment_bookings()
{
    QString sql {qsl(R"str(
WITH
fortlaufendeGeldanlagen AS
(
   SELECT Typ, rowid, ZSatz FROM Geldanlagen WHERE Ende = '9999-12-31'
)
, geldBewegungen AS
(
  SELECT
    Buchungen.Datum AS bDatum
    , Buchungen.Betrag AS Buchungsbetrag
    , Anlagen.rowid AS aId
    , Anlagen.Typ AS Anlage
    , Anlagen.ZSatz AS Zinssatz

  FROM Buchungen
  INNER JOIN Vertraege ON Vertraege.id == Buchungen.VertragsId
  INNER JOIN fortlaufendeGeldanlagen AS Anlagen ON Anlagen.rowid = Vertraege.AnlagenId
)
, temp AS
(
  SELECT
    aId
    , Zinssatz
    , Anlage
    , bDatum
    , SUM(Buchungsbetrag) /100. AS Buchungsbetraege
    , COUNT(Buchungsbetrag) AS anzahlBuchungen

    , (SELECT SUM(Buchungen.Betrag) /100. FROM Buchungen WHERE Buchungen.VertragsId IN (SELECT id FROM Vertraege WHERE Vertraege.AnlagenId == aId) AND Buchungen.Datum <= bDatum AND Buchungen.Datum > DATE(bDatum, '-1 year')
    ) AS BuchungsSummenInclZins
    , (SELECT COUNT(*) FROM Buchungen WHERE Buchungen.VertragsId IN (SELECT id FROM Vertraege WHERE Vertraege.AnlagenId == aId) AND Buchungen.Datum <= bDatum AND Buchungen.Datum > DATE(bDatum, '-1 year')
    ) AS BuchungsSummenInclZins_count

    , (SELECT SUM(Buchungen.Betrag) /100. FROM Buchungen WHERE Buchungen.VertragsId IN (SELECT id FROM Vertraege WHERE Vertraege.thesaurierend == 0 AND Vertraege.AnlagenId == aId) AND Buchungen.Datum <= bDatum AND Buchungen.Datum > DATE(bDatum, '-1 year') AND (Buchungen.BuchungsArt == 1 OR Buchungen.BuchungsArt == 2 OR Buchungen.BuchungsArt == 8)
    ) AS BuchungsSummenExclZins_ausz

    , (SELECT COUNT(*) FROM Buchungen WHERE Buchungen.VertragsId IN (SELECT id FROM Vertraege WHERE Vertraege.thesaurierend != 0 AND Vertraege.AnlagenId == aId) AND Buchungen.Datum <= bDatum AND Buchungen.Datum > DATE(bDatum, '-1 year') AND (Buchungen.BuchungsArt == 1 OR Buchungen.BuchungsArt == 2)
    ) AS BuchungsSummenExclZins_N_ausz_count
    , (SELECT SUM(Buchungen.Betrag) /100. FROM Buchungen WHERE Buchungen.VertragsId IN (SELECT id FROM Vertraege WHERE Vertraege.thesaurierend != 0 AND Vertraege.AnlagenId == aId) AND Buchungen.Datum <= bDatum AND Buchungen.Datum > DATE(bDatum, '-1 year') AND (Buchungen.BuchungsArt == 1 OR Buchungen.BuchungsArt == 2)
    ) AS BuchungsSummenExclZins_N_ausz

  FROM geldBewegungen
  GROUP BY aId, bDatum
  ORDER BY aId DESC, bDatum DESC
)
SELECT
  aId
  , Anlage
  , Zinssatz
  , bDatum
  , anzahlBuchungen AS zumDatum_anzahlBuchungen
  , IFNULL(Buchungsbetraege, 0.) AS zumDatum_Gebucht
  , IFNULL(BuchungsSummenExclZins_ausz, 0.) + IFNULL(BuchungsSummenExclZins_N_ausz, 0.) AS ly_einAusZahlungen_oZ
  , IFNULL(BuchungsSummenInclZins, 0.) AS ly_Wert_incl_Zinsen
FROM temp
    )str")};

    QVector<QSqlRecord> rec;
    if( not executeSql (sql, rec)) {
        return QVector<QStringList>();
    }
    QVector<QStringList> result;
    for( int i=0; i< rec.size (); i++) {
        QStringList zeile;
        int col =1;
        QString anlage =qsl("%1 (%2%)").arg(rec[i].value(col++).toString());
        zeile.push_back (anlage.arg( QString::number( rec[i].value(col++).toInt () /100.))); // Anlagenbez.
        zeile.push_back (rec[i].value(col++).toDate().toString ("dd.MM.yyyy")); // Buchungsdatum
        zeile.push_back (i2s(rec[i].value(col++).toInt())); // Anzahl Buchungen
        zeile.push_back (s_d2euro(rec[i].value(col++).toDouble ())); // buchungen zu diesem Buchungsdatum
        zeile.push_back (decorateHighValues (rec[i].value(col++).toDouble ())); // Wert nur Einzahlungen
        zeile.push_back (decorateHighValues (rec[i].value(col++).toDouble ())); // Wert incl. Zinsen
        result.push_back (zeile);
    }
    return result;
}

QVector<QStringList> perpetualInvestmentByContracts()
{
    QString sql {qsl(R"str(
   WITH
   fortlaufendeGeldanlagen AS
   (
      SELECT * FROM Geldanlagen WHERE Ende = '9999-12-31'
   ),
    Abschluesse AS (
      SELECT G.rowid    AS AnlageId
        , G.Typ || ' (' || printf("%.2f%", V.zSatz/100.) || ')' AS Anlage
        , V.Kennung          AS Vertrag
        , V.Vertragsdatum AS Datum
        , V.Betrag        AS Betrag
      FROM Vertraege AS V
      INNER JOIN fortlaufendeGeldanlagen AS G ON G.rowid = V.AnlagenId
    )
    SELECT
    Abschluesse.Anlage AS Anlage
    , Abschluesse.Datum AS Vertragsdatum
    , Abschluesse.Vertrag
    , count(Abschluesse.Vertrag) AS AnzahlVerträge
    , sum(Abschluesse.Betrag/100.) AS AnlageSumme_Tag
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
    ORDER BY Anlage ASC, Datum DESC
    )str")};
    QVector<QSqlRecord> rec;
    if( not executeSql (sql, rec)) {
        return QVector<QStringList>();
    }
    QVector<QStringList> result;
    for( int i=0; i< rec.size (); i++) {
        QStringList zeile;
        int col =0;
        zeile.push_back (rec[i].value(col++).toString ()), // Anlage
        zeile.push_back (rec[i].value(col++).toDate().toString("dd.MM.yyyy")); // Vertragsdatum
        zeile.push_back (rec[i].value(col++).toString());  //Kennung
        zeile.push_back (rec[i].value(col++).toString ()); // contract count
        zeile.push_back (s_d2euro(rec[i].value(col++).toDouble ())); // new contract sum by day
        double periodSum =rec[i].value(col++).toDouble ();
        zeile.push_back (decorateHighValues (periodSum));
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

QString Vor_Nachname_Kreditor(qlonglong id)
{
    return executeSingleValueSql(qsl("Vorname || ' ' || Nachname"), qsl("Kreditoren"),
                                 qsl("id=%1").arg(id)).toString ();
}

// struct changeBookingData {
//     QString VKennung;
//     QString Vorname;
//     QString Nachname;
//     int BetragInCt;
//     QDate Buchungsdatum;
// };

bool getChangeBookingData(changeBookingData& cbd, qlonglong bid)
{
    QString  sql{ qsl(R"str(
SELECT
  Vertraege.Kennung
, Kreditoren.Vorname
, Kreditoren.Nachname
, Buchungen.Betrag
, Buchungen.Datum

FROM Buchungen

INNER JOIN Vertraege ON Buchungen.VertragsId = Vertraege.id
INNER JOIN Kreditoren ON Kreditoren.id = Vertraege.KreditorId

WHERE
    Buchungen.id = %1
)str") };

    QSqlRecord rec =executeSingleRecordSql (sql.arg(bid));
    qDebug() << rec;
    cbd.Vorname  =rec.value("Vorname").toString ();
    cbd.Nachname =rec.value ("Nachname").toString ();
    cbd.BetragInCt =rec.value ("Betrag").toInt ();
    cbd.VKennung =rec.value ("Kennung").toString ();
    cbd.Buchungsdatum =rec.value ("Datum").toDate ();
    return true;
}


