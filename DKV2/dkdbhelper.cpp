
#include "busycursor.h"
#include "helperfile.h"
#include "contract.h"
#include "dkv2version.h"
#include "appconfig.h"
#include "helpersql.h"
#include "csvwriter.h"
#include "creditor.h"
#include "investment.h"
#include "letterTemplate.h"
#include "dkdbviews.h"
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
    if( not (vversion.isValid() and vversion.canConvert(QMetaType::Double)))
        return noVersion; // big problem: no db
    int d =vversion.toInt();
    qInfo() << "DB Version Comparison: expected / found: " << CURRENT_DB_VERSION << " / " << d;
    return d;
}

bool isExistingContractLabel( const QString& newLabel)
{
    QVariant existingLabel =executeSingleValueSql(contract::fnKennung, contract::tnContracts, qsl("Kennung = '") +newLabel +qsl("'"));
    return existingLabel.isValid();
}

bool isExisting_Ex_ContractLabel( const QString& newLabel)
{
    QVariant existingLabel =executeSingleValueSql(contract::fnKennung, contract::tnExContracts, qsl("Kennung = '") +newLabel +qsl("'"));
    return existingLabel.isValid();
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
    for(const auto & sql: qAsConst(updates)) {
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
    return createDbViews(views, db);
}// initial database tables and content

bool insertDKDB_Indices( const QSqlDatabase& db)
{   LOG_CALL;
    return createIndicesFromSQL( getIndexSql (), db);
}

bool fill_DkDbDefaultContent(const QSqlDatabase &db, bool includeViews /*=true*/, zinssusance sz /*=zs30360*/)
{
    LOG_CALL;
    switchForeignKeyHandling(db, true);
    bool ret = false;
    autoRollbackTransaction art(db.connectionName());
    do {
        if( includeViews) if ( not insertDKDB_Views(db)) break;
        if( includeViews) if (not insertDKDB_Indices(db)) break;
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

bool checkSchema_ConvertIfneeded(const QString &origDbFile)
{
    LOG_CALL;
    busycursor bc;
    int version_of_original_file = get_db_version(origDbFile);
    if (version_of_original_file < CURRENT_DB_VERSION)
    {
        qInfo() << "lower version -> converting";
        bc.finish (); // normal cursor during message box
        if (QMessageBox::Yes not_eq QMessageBox::question(getMainWindow(), qsl("Achtung"), qsl("Das Format der Datenbank \n%1\nist veraltet.\nSoll die Datenbank konvertiert werden?").arg(origDbFile))) {
            qInfo() << "conversion rejected by user";
            return false;
        }
        QString backup = convert_database_inplace(origDbFile);
        if (backup.isEmpty()) {
            bc.finish (); // normal cursor during message box
            QMessageBox::critical(getMainWindow(), qsl("Fehler"), qsl("Bei der Konvertierung ist ein Fehler aufgetreten. Die Ausführung muss beendet werden."));
            qCritical() << "db converstion of older DB failed";
            return false;
        }
        // actions which depend on the source version of the db
        if (not postDB_UpgradeActions(version_of_original_file, origDbFile)) {
            bc.finish (); // normal cursor during message box
            QMessageBox::critical(getMainWindow(), qsl("Fehler"), qsl("Bei der Konvertierung ist ein Fehler aufgetreten. Die Ausführung muss beendet werden."));
            qCritical() << "db converstion of older DB failed";
            return false;
        }
        bc.finish ();
        QMessageBox::information(nullptr, qsl("Erfolgsmeldung"), qsl("Die Konvertierung ware erfolgreich. Eine Kopie der ursprünglichen Datei liegt unter \n") + backup);
        return true;
    }
    else if (version_of_original_file == CURRENT_DB_VERSION)
    {
        return validateDbSchema(origDbFile, dkdbstructur);
    }
    else
    {
        qInfo() << "higher version ? there is no way back";
        return false;
    }
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
    if( not updateViewsAndIndices_if_needed (db)) {
        qCritical() << "update views on " << newDbFile << " failed";
        return false;
    }
    switchForeignKeyHandling(db, fkh_on);
    return true;
}

bool isValidNewContractLabel(const QString& label)
{
    return ( not isExistingContractLabel(label)) and ( not isExisting_Ex_ContractLabel(label));
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

int createNewInvestmentsFromContracts( bool fortlaufend)
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
    QStringList header;
    dbtable t(tempViewContractsCsv);
    t.append(dbfield(contract::fnId, QVariant::Type::Int));
    header.append(qsl("Vertragsnummer"));
    t.append(dbfield(creditor::fnId, QVariant::Type::Int));
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
//    t.append(dbfield(creditor::fnStrasse));
    t.append(dbfield(qsl("Zinssatz"), QVariant::Type::Double));
    header.append (qsl("Zinssatz"));
    t.append(dbfield(qsl("Wert"), QVariant::Type::Double));
    header.append (qsl("Vertragswert"));
    t.append(dbfield(qsl("Aktivierungsdatum"), QVariant::Type::Date));
    header.append (qsl("Aktivierungsdatum"));
    t.append(dbfield(qsl("Kuendigungsfrist"), QVariant::Type::Int));
    header.append (qsl("Kündigungsfrist"));
    t.append(dbfield(qsl("Vertragsende"), QVariant::Type::Date));
    header.append (qsl("Vertragsende"));
    t.append(dbfield(qsl("thesa"), QVariant::Type::Int));
    header.append (qsl("Zinsmodus"));

    QString sql = selectQueryFromFields(t.Fields (), QVector<dbForeignKey>());
    QSqlQuery q;
    if( not q.exec(sql)) {
        qCritical() << "sql faild to execute" << q.lastError() << "\nSQL: " << q.lastQuery();
        return false;
    }
    QVector<QStringList> data;
    QLocale locale;
    while(q.next()) {
        QStringList col;
        col.append (q.value(contract::fnId).toString());
        col.append (q.value(creditor::fnId).toString());
        col.append (q.value(creditor::fnVorname).toString());
        col.append (q.value(creditor::fnNachname).toString());
        col.append (q.value(creditor::fnStrasse).toString());
        col.append (q.value(creditor::fnPlz).toString());
        col.append (q.value(creditor::fnStadt).toString());
        col.append (q.value(creditor::fnEmail).toString());
        col.append (q.value(creditor::fnIBAN).toString());
        col.append (q.value(creditor::fnBIC).toString());
        col.append (d2percent_str(q.value(qsl("Zinssatz")).toDouble ()));
        col.append (locale.toCurrencyString (q.value(qsl("Wert")).toDouble ()));
        col.append (q.value(qsl("Aktivierungsdatum")).toDate().toString ("dd.MM.yyyy"));
        col.append (q.value(qsl("Kuendigungsfrist")).toString());
        col.append (q.value(qsl("Vertragsende")).toDate().toString ("dd.MM.yyyy"));
        col.append (interestModelDisplayString(interestModelFromInt(q.value(qsl("thesa")).toInt())));
        data.append(col);
    }

    QString filename(QDate::currentDate().toString(Qt::ISODate) + "-Aktive-Vertraege.csv");
//    bool res =table2csv( filename, t.Fields());
    bool res =StringLists2csv( filename, header, data);
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
        qCritical() << "calculation of runtime distribution failed: " << q.lastError() << "\n" << q.lastQuery();
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

namespace {
QString decorateHighValues(double d)
{
    QString ret =QLocale().toCurrencyString (d);
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
    QLocale l;
    QVector<QSqlRecord> rec;
    if( not executeSql (sql, rec)) {
        return QVector<QStringList>();
    }
    QVector<QStringList> result;
    for( int i=0; i< rec.size (); i++) {
        QStringList zeile;
        int col =1;
        // zeile.push_back (QString::number(rec[i].value(col++).toInt())); // AnlagenId
        QString anlage =qsl("%1 (%2%)").arg(rec[i].value(col++).toString());
        zeile.push_back (anlage.arg(QString::number(rec[i].value(col++).toInt ()/100.))); // Anlagenbez.
        zeile.push_back (rec[i].value(col++).toDate().toString ("dd.MM.yyyy")); // Buchungsdatum
        zeile.push_back (QString::number(rec[i].value(col++).toInt())); // Anzahl Buchungen
        zeile.push_back (l.toCurrencyString (rec[i].value(col++).toDouble ())); // buchungen zu diesem Buchungsdatum
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
    QLocale l;
    QVector<QStringList> result;
    for( int i=0; i< rec.size (); i++) {
        QStringList zeile;
        int col =0;
        zeile.push_back (rec[i].value(col++).toString ()), // Anlage
        zeile.push_back (rec[i].value(col++).toDate().toString("dd.MM.yyyy")); // Vertragsdatum
        zeile.push_back (rec[i].value(col++).toString());  //Kennung
        zeile.push_back (rec[i].value(col++).toString ()); // contract count
        zeile.push_back (l.toCurrencyString (rec[i].value(col++).toDouble ())); // new contract sum by day
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
