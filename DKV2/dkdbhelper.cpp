#include <QRandomGenerator>
//#include <QTemporaryFile>
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

bool insert_views( QSqlDatabase db)
{   LOG_CALL;
    return createViews(getViews(), db);
}// initial database tables and content

void insert_DbProperties(QSqlDatabase db = QSqlDatabase::database())
{
    LOG_CALL;
    dbConfig::writeDefaults(db);
}

bool fill_DkDbDefaultContent(QSqlDatabase db, bool includeViews /*=true*/)
{
    LOG_CALL;
    switchForeignKeyHandling(db, true);
    bool ret = false;
    autoRollbackTransaction art(db.connectionName());
    do {
        if( includeViews) if ( not insert_views(db)) break;
        insert_DbProperties(db);
        if ( not letterTemplate::insert_letterTypes(db)) break;
        if ( not letterTemplate::insert_elementTypes(db)) break;
        if ( not letterTemplate::insert_letterElements(db)) break;
        ret = true;
    } while (false);
    if (ret)
        art.commit();
    return ret;
}

version_check_result check_db_version(QString file)
{
    LOG_CALL_W(file);
    dbCloser closer(qsl("db_version_check"));
    QSqlDatabase db =QSqlDatabase::addDatabase(dbTypeName, closer.conName);
    db.setDatabaseName(file);
    if( not db.open()) {
        qCritical() << "could not open db " << file << " for version check ";
        return noVersion;
    } else {
        return check_db_version(db);
    }
}

version_check_result check_db_version(QSqlDatabase db)
{   /*LOG_CALL;*/
    QVariant vversion =dbConfig::read_DBVersion(db);
    if( not (vversion.isValid() and vversion.canConvert(QMetaType::Double)))
        return noVersion; // big problem: no db
    double d =vversion.toDouble();
    qInfo() << "DB Version Comparison: expected / found: " << CURRENT_DB_VERSION << " / " << d;
    if( d  < CURRENT_DB_VERSION)
        return lowVersion; // the database is old -> conversion?
    if( d == CURRENT_DB_VERSION)
        return sameVersion; // all good
    if( d >  CURRENT_DB_VERSION)
        return higherVersion; // the database is too young -> don't touch!
    Q_ASSERT( not "one should never come here");
    return noVersion;
}

bool updateViews(QSqlDatabase db =QSqlDatabase::database())
{   LOG_CALL;
    QString lastProgramVersion = dbConfig::readValue(DKV2_VERSION).toString();
    QString thisProgramVersion = QCoreApplication::applicationVersion();
    if( lastProgramVersion not_eq thisProgramVersion) {
        qInfo() << "Program versions used differ -> views will be updated";
        qInfo() << qsl("last exe: ") << lastProgramVersion << qsl(" / this exe: ") << thisProgramVersion;
        if( not insert_views(db)) {
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
    Q_ASSERT( not newDbFile.isEmpty());

    closeAllDatabaseConnections();
    backupFile(newDbFile, "db-bak");

    // setting the default database for the application
    QSqlDatabase db = QSqlDatabase::addDatabase(dbTypeName);
    db.setDatabaseName(newDbFile);
    if( not db.open()) {
        qCritical() << "open database file " << newDbFile << " failed";
        return false;
    }

    switchForeignKeyHandling(db, fkh_on);
    if( not updateViews())
        return false;
    return true;
}

// general stuff
bool isExistingContractLabel( const QString& newLabel)
{
    QVariant existingLabel =executeSingleValueSql(qsl("Kennung"), qsl("Vertraege"), qsl("Kennung = '") +newLabel +qsl("'"));
    return existingLabel.isValid();
}

bool isExistingExContractLabel( const QString& newLabel)
{
    QVariant existingLabel =executeSingleValueSql(qsl("Kennung"), qsl("exVertraege"), qsl("Kennung = '") +newLabel +qsl("'"));
    return existingLabel.isValid();
}

bool isValidNewContractLabel(const QString& label)
{
    return ( not isExistingContractLabel(label)) and ( not isExistingExContractLabel(label));
}

QString proposeContractLabel()
{   LOG_CALL;
    static int iMaxid = dbConfig::readValue(STARTINDEX).toInt() + getHighestRowId("Vertraege");
    QString kennung;
    do {
        QString maxid = QString::number(iMaxid).rightJustified(6, '0');
        QString PI = "DK-" + dbConfig::readValue(GMBH_INITIALS).toString();
        kennung = PI + "-" + QString::number(QDate::currentDate().year()) + "-" + maxid;
        if( isValidNewContractLabel(kennung))
            break;
        else
            iMaxid++;
    } while(1);
    iMaxid++; // prepare for next contract
    return kennung;
}

int createNewInvestmentsFromContracts()
{
    QString sql{qsl("SELECT ZSatz, Vertragsdatum FROM Vertraege ORDER BY Vertragsdatum ASC")};
    QSqlQuery q; q.setForwardOnly(true);
    if( not q.exec(sql)) {
        qCritical() << "query execute faild in " << __func__;
        return 0;
    }
    int ret =0;
    while(q.next()) {
        int ZSatz =q.record().value(qsl("ZSatz")).toInt();
        QDate vDate =q.record().value(qsl("Vertragsdatum")).toDate();
        if( createInvestmentFromContractIfNeeded(ZSatz, vDate))
            ret++;
    }
    return ret;
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
    dbtable t(sqlContractsActiveDetailsView);
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

    if( not table2csv( filename, t.Fields())) {
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
    QString sql = qsl("SELECT Wert, Aktivierungsdatum, Vertragsende "
                  "FROM (%1)").arg(sqlContractsActiveView);
    QSqlQuery q; q.setForwardOnly(true);
    if( not q.exec(sql)) {
        qCritical() << "calculation of runtime distribution failed: " << q.lastError() << Qt::endl << q.lastQuery();
        return QVector<rowData>();
    }

    while( q.next()) {
        double wert =   q.value("Wert").toReal();
        QDate von = q.value("Datum").toDate();
        QDate bis = q.value("Vertragsende").toDate();
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
    QVector<rowData> ret;
    ret.push_back({"Zeitraum", "Anzahl", "Wert"});
    ret.push_back({"Bis ein Jahr ", QString::number(AnzahlBisEinJahr), locale.toCurrencyString(SummeBisEinJahr)});
    ret.push_back({"Ein bis fünf Jahre ", QString::number(AnzahlBisFuenfJahre), locale.toCurrencyString(SummeBisFuenfJahre)});
    ret.push_back({"Länger als fünf Jahre ", QString::number(AnzahlLaenger), locale.toCurrencyString(SummeLaenger) });
    ret.push_back({"Unbegrenzte Verträge ", QString::number(AnzahlUnbegrenzet), locale.toCurrencyString(SummeUnbegrenzet) });
    return ret;
}

void calc_contractEnd( QVector<ContractEnd>& ces)
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
void calc_annualInterestDistribution( QVector<YZV>& yzv)
{   LOG_CALL;
    QString sql =qsl("SELECT * FROM (%1) ORDER BY Year").arg(sqlContractsByYearByInterest);

    QSqlQuery query; query.setForwardOnly(true);
    if( not query.exec(sql)) {
        qCritical() << "execute query failed";
        return;
    }
    while( query.next()) {
        QSqlRecord r =query.record();
        yzv.push_back({r.value("Year").toInt(), r.value("Zinssatz").toReal(),
                       r.value("Anzahl").toInt(), r.value("Summe").toReal()});
    }
    return;
}

void getDatesBySql(QString sql, QVector<QDate>& dates)
{
    QVector<QSqlRecord> records;
    if( not executeSql(sql, QVector<QVariant>(), records)) {
        qInfo() << "getDatesBySql: no dates to found";
        return;
    }
    for (auto rec : records) {
        dates.push_back(rec.value(0).toDate());
    }
    qInfo() << "getDatesBySql added " << dates.size() << " dates to the vector";
}

// get dates for statistic page
void getActiveContracsDates( QVector<QDate>& dates)
{
    QString sql {qsl(R"str(
SELECT DISTINCT Datum FROM
(
    SELECT Datum
    FROM Buchungen
       UNION
    SELECT Datum
    FROM exBuchungen
)
ORDER BY Datum DESC
)str")};
    return getDatesBySql(sql, dates);
}

void getInactiveContractDates( QVector<QDate>& dates)
{
    QString sql {qsl(R"str(
SELECT DISTINCT Vertragsdatum AS Datum
FROM Vertraege
ORDER BY Datum DESC
)str")};
    return getDatesBySql(sql, dates);
}

void getAllContractDates( QVector<QDate>& dates)
{
QString sql {qsl(R"str(
SELECT DISTINCT Datum FROM
(
  SELECT Datum
  FROM Buchungen
    UNION
  SELECT Datum
  FROM exBuchungen
    UNION
  SELECT Vertragsdatum AS Datum
  FROM Vertraege
)
ORDER BY Datum DESC
)str")};
    return getDatesBySql(sql, dates);
}


