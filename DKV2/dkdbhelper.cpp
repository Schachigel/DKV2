#include <QRandomGenerator64>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QVector>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "helper.h"
#include "filehelper.h"
#include "appconfig.h"
#include "sqlhelper.h"
#include "csvwriter.h"
#include "finhelper.h"
#include "dkdbhelper.h"
#include "creditor.h"
#include "contract.h"
#include "booking.h"
#include "dbstructure.h"

const double CURRENT_DB_VERSION {2.0};
const QString DB_VERSION {"Version"};

dbstructure dkdbstructur;

QList<QPair<qlonglong, QString>> Buchungsarten;

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
    meta.append(dbfield("Name", QVariant::String, "NOT NULL"));
    meta.append(dbfield("Wert", QVariant::String, "NOT NULL"));
    dkdbstructur.appendTable(meta);

    dbtable letters("Briefvorlagen");
    letters.append(dbfield("templateId", QVariant::Int));
    letters.append(dbfield("EigenschaftId", QVariant::Int));
    letters.append(dbfield("Wert"));
    dkdbstructur.appendTable(letters);

    done = true;
}

void init_GmbHData( )
{   LOG_CALL;
    initMetaInfo("gmbh.address1", "Esperanza Franklin GmbH");
    initMetaInfo("gmbh.address2", "");
    initMetaInfo("gmbh.plz", "68167");
    initMetaInfo("gmbh.stadt", "Mannheim");
    initMetaInfo("gmbh.strasse", "Turley-Platz 9");
    initMetaInfo("gmbh.email","info@esperanza-mannheim.de");
    initMetaInfo("gmbh.url", "www.esperanza-mannheim.de");
}

bool insert_Properties()
{   LOG_CALL;
    bool ret =true;
    QSqlQuery sql;
//    QRandomGenerator *rand = QRandomGenerator::system();
    initNumMetaInfo(DB_VERSION, CURRENT_DB_VERSION);
//    initMetaInfo("IdOffset", QString::number(rand->bounded(10000,20000)), db);
    initMetaInfo("ProjektInitialen", "ESP");
    return ret;
}

void initMetaInfo( const QString& name, const QString& newValue)
{   LOG_CALL;
    QVariant value= ExecuteSingleValueSql("SELECT WERT FROM Meta WHERE Name='" + name +"'");
    if( value.type() == QVariant::Type::Invalid)
        setMetaInfo(name, newValue);
}
void initNumMetaInfo( const QString& name, const double& newValue)
{   LOG_CALL;
    QVariant value= ExecuteSingleValueSql("SELECT WERT FROM Meta WHERE Name='" + name +"'");
    if( value.type() == QVariant::Type::Invalid)
        setNumMetaInfo(name, newValue);
}
QString getMetaInfo(const QString& name)
{   LOG_CALL_W(name);
    QVariant value= ExecuteSingleValueSql("SELECT WERT FROM Meta WHERE Name='" + name +"'").toString();
    if( value.type() == QVariant::Type::Invalid) {
        qInfo() << "read empty property " << name << "; defaulted to empty string";
        return "";
    }
    qInfo() << "Property " << name << " : " << value;
    return value.toString();
}
double getNumMetaInfo(const QString& name, QSqlDatabase db)
{   LOG_CALL_W(name);

    QVariant value= ExecuteSingleValueSql("SELECT WERT FROM Meta WHERE Name='" + name +"'", db);
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
    q.prepare("INSERT OR REPLACE INTO Meta (Name, Wert) VALUES ('" + name + "', '" + QString::number(Wert) +"')");
    if( !q.exec())
        qCritical() << "Failed to insert Meta information " << q.lastError() << endl << q.lastQuery();
}

bool create_DK_databaseContent(QSqlDatabase db)
{   LOG_CALL;
    bool ret = true;
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON", db);

    db.transaction();
    do {
        if(! (ret &= createTables(dkdbstructur, db))) break;
        if(! (ret &= insert_Properties())) break;
    } while(false);

    if(!ret) {
        db.rollback();
        qCritical() << "creating db structure in new database failed";
        return false;
    }

    db.commit();
    return isValidDatabase();
}
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
    return create_DK_databaseContent(db);
}

bool has_allTablesAndFields(QSqlDatabase db)
{   LOG_CALL;
    for( auto table : dkdbstructur.getTables()) {
        QSqlQuery sql(db);
        sql.prepare(QString("SELECT * FROM ") + table.Name() +" LIMIT 0");
        if( !sql.exec()) {
            qCritical() << "Testing for table " << table.Name() << " failed\n" << sql.lastError() << endl << sql.lastQuery();
            return false;
        }
        if( table.Fields().count() != sql.record().count()) {
            qCritical() << "Table " << table.Name() << " has the wrong number of fields ";
            return false;
        }
        QSqlRecord r=sql.record();
        for(int i = 0; i< r.count(); i++ ) {
            QString fieldname = r.fieldName(i);
            if( table[fieldname] == dbfield()) {
                qCritical() << "testing for field" << fieldname << " failed\n" << sql.lastError() << endl << sql.lastQuery();
                return false;
            }
        }
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

bool check_db_version(QSqlDatabase db)
{   LOG_CALL;
    double d = getNumMetaInfo(DB_VERSION, db);
    if( d >= CURRENT_DB_VERSION)
        return true;
    qCritical() << "db version check failed: found version " << d << " needed version " << CURRENT_DB_VERSION;
    return false;
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
        return false;

    init_GmbHData();
    return true;
}

void check_KfristLaufzeitende(QStringList& msgs)
{
    // temporary; fix data corrupted by date edit control (1752 ...)
    // inaktive Verträge können kein Datum zur Zinsberechnung haben
    int i = ExecuteUpdateSql("[Vertraege]", "[LetzteZinsberechnung]", "9999-12-31",  "NOT([aktiv]) AND [LetzteZinsberechnung] <> '9999-12-31'");
    if( i > 0) msgs.append("Inaktive Verträge mit Zinsberechnungsdatum wurden korrigiert");

    // wenn Laufzeitende gültig gesetzt wurde muss Kfrist -1 sein
    i = ExecuteUpdateSql("[Vertraege]", "[Kfrist]", "-1", "[LaufzeitEnde] <> '9999-12-31' AND [Kfrist] <> -1");
    if( i > 0) msgs.append("Die Kündigungsfrist von Verträgen mit festem Laufzeitende wurde korrigiert ");

    // inkonsistente Datensätze: ende und KFrist sind nicht gesetzt
    i = ExecuteUpdateSql("[Vertraege]", "[Kfrist]", "6", "[LaufzeitEnde] == '9999-12-31' AND [Kfrist] < 3");
    if( i > 0) msgs.append("Die Kündigungsfrist von Verträgen ohne gültigem Laufzeitende wurde angepasst");
}

void check_Ibans(QStringList& msg)
{
    IbanValidator iv;
    QSqlQuery iban_q;
    iban_q.exec("SELECT [id],[Vorname],[Nachname],[IBAN] FROM [Kreditoren] WHERE [IBAN] <> ''");
    while(iban_q.next()) {
        QString iban = iban_q.value("IBAN").toString();
        int pos = 0;
        if( iv.validate(iban, pos) == IbanValidator::State::Acceptable)
            continue;
        msg.append( QString("IBAN Prüfung fehlgeschlagen bei Kreditor ") +iban_q.value("id").toString() +": "
                       +iban_q.value("Vorname").toString() +iban_q.value("Nachname").toString()
                   +"\n: " +iban_q.value("IBAN").toString());
    }
}

QStringList check_DbConsistency( )
{   LOG_CALL;
    QStringList msgs;
    check_KfristLaufzeitende(msgs);
    check_Ibans(msgs);
    if( msgs.size() > 0)
        msgs.push_front("Prüfen Sie die LOG Datei! ");

    return msgs;
}

bool copy_Table(QString table, QSqlDatabase targetDB)
{   LOG_CALL_W(table);
    bool success = true;
    QSqlQuery q; // default database connection -> active database
    q.prepare("SELECT * FROM " + table);
    q.exec();
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
    q.prepare("SELECT * FROM Kreditoren");
    q.exec();
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
        if( deper && table.Name() == "Buchungen") {
            qDebug() << "de personalisation mode: skipping 'Buchungen' ";
            continue;
        }
        qDebug() << "dePe Copy: working on table " << table.Name();
        if( deper && table.Name() == "Kreditoren")
            success = success && copy_mangledCreditors(backupDB);
        else
            success = success && copy_Table(table.Name(), backupDB);
    }
    return success;
}

//void create_sampleDataset( contract& vertrag, int KId, int maxZinsIndex, QRandomGenerator* rand)
//{   LOG_CALL;
//    // add a contract
//    double betragUWert = double(100) * rand->bounded(1,20);
//    int zinsid = rand->bounded(1,maxZinsIndex);
//    bool thesa = rand->bounded(100)%4 ? true : false;  // 75% thesaurierend
//    bool active = rand->bounded(100)%6 ? true : false; // 85% inaktiv
//    QDate vertragsdatum= QDate::currentDate().addDays(-1 * rand->bounded(365));
//    QDate StartZinsberechnung = ( active) ? vertragsdatum.addDays(rand->bounded(15)) : EndOfTheFuckingWorld;
//    QDate LaufzeitEnde = (rand->bounded(100)%8 == 1)
//                             ? (QDate(9999, 12, 31)) // kein Ende vereinbart
//                             : StartZinsberechnung.addDays( 500+ rand->bounded(0, 8000));
//    int kFrist = -1;
//    if( LaufzeitEnde == QDate(9999, 12, 31))
//    {
//        kFrist = rand->bounded(3, 25);
//    }
//    vertrag = Contract(KId, proposeKennung(),
//                    betragUWert, betragUWert, zinsid,
//                    vertragsdatum,
//                    thesa, active, StartZinsberechnung, kFrist, LaufzeitEnde);
//}

//void create_sampleData( int AnzahlDatensaetze)
//{   LOG_CALL;
//Q_ASSERT(!"change");
//    QList<QString> Vornamen {"Holger", "Volker", "Peter", "Hans", "Susi", "Roland", "Claudia", "Emil", "Evelyn", "Ötzgür", "Thomas", "Elke", "Berta", "Malte", "Jori", "Paul", "Jonas", "Finn", "Leon", "Luca", "Emma", "Mia", "Lena", "Anna", "Anne", "Martha", "Ruth", "Rosemie", "Rosemarie", "Verena", "Ursula", "Erika", "Adrian", "Avan", "Anton", "Benno", "Karl", "Merlin", "Noah", "Oliver", "Olaf", "Pepe", "Zeno"};
//    QList<QString> Nachnamen {"Maier", "Müller", "Schmit", "Kramp", "Adams", "Häcker", "Maresch", "Beutl", "Chauchev", "Chen", "Kirk", "Ohura", "Gorbatschov", "Merkel", "Karrenbauer", "Tritin", "Schmidt", "Rao", "Lassen", "Hurgedü", "vom Dach", "Langstrumpf", "Lederstrumpf", "Potter", "Poppins", "Wisley", "Li", "Wang", "Ran"};
//    QList<QString> Strassen {"Hauptstrasse", "Nebenstrasse", "Bahnhofstrasse", "Kirchstraße", "Dorfstrasse", "Süterlinweg", "Sorbenstrasse", "Kleines Gässchen", "Industriestrasse", "Sesamstrasse", "Lindenstrasse", "Theaterstrasse", "Museumsstrasse", "Opernplatz", "Schillerstrasse", "Lessingstrasse", "Rathausplatz", "Parkstrasse", "Turmstrasse" };
//    QList<QString> emailprovider {"gmail.com", "googlemail.com", "mailbox.org", "t-online.de", "mail.de", "mail.com", "online.de", "yahoo.de", "yahoo.com", "telekom.de", "proivder.co.uk", "AOL.de", "outlook.com", "microsoft.com", "sap.com", "sap-ag.de", "abb.de"};
//    QList<QString> ibans {"BG80BNBG96611020345678", "DE38531742365852502530", "DE63364408232964251731", "DE38737364268384258531", "DE69037950954001627624", "DE63377045386819730665", "DE18851420444163951769", "DE77921850720298609321", "DE70402696485599313572", "DE70455395581860402838", "DE94045704387963352767", "DE30724236236062816411", "DE62772043290447861437", "DE33387723124963875990", "DE15867719874951165967",
//                          "DE96720348741083219766", "DE23152931057149592044", "DE13220161295670898833", "DE49737651031822324605", "DE38017168378078601588", "DE07717138875827514267"};
//    QList <QPair<QString, QString>> Cities {{"68305", "Mannheim"}, {"69123", "Heidelberg"}, {"69123", "Karlsruhe"}, {"90345", "Hamburg"}};
//    QRandomGenerator *rand = QRandomGenerator::system();
//    int maxZinsIndex = ExecuteSingleValueSql("SELECT max(id) FROM Zinssaetze").toInt();
//    int neueKreditorId =0;
//    for( int i = 0; i<AnzahlDatensaetze; i++)
//    {
//        creditor k;
//        QString vn (Vornamen [rand->bounded(Vornamen.count ())]);
//        QString nn (Nachnamen [rand->bounded(Nachnamen.count ())]);
//        k.setVorname(vn);
//        k.setNachname(nn);
//        k.setStrasse(Strassen[rand->bounded(Strassen.count())]);
//        k.setPlz(Cities[rand->bounded(Cities.count())].first);
//        k.setStadt(Cities[rand->bounded(Cities.count())].second);
//        k.setEmail(vn+"."+nn+"@"+emailprovider[rand->bounded(emailprovider.count())]);
//        k.setIban(ibans[rand->bounded(ibans.count())]);
//        k.setBic("bic...");

//        neueKreditorId =k.save();
//        if( -1 == neueKreditorId)
//        {
//            qCritical() << "No id from Kreditor.Speichern";
//            Q_ASSERT(!bool("Verbuchung des neuen Vertrags gescheitert"));
//        }
//        contract v;
//        create_sampleDataset(v, neueKreditorId, maxZinsIndex, rand);
////        v.bookNewContract();
//    }
//    for ( int i=0; i<AnzahlDatensaetze; i++)
//    {   // more contracts for existing customers
//        contract v;
//        create_sampleDataset(v, rand->bounded(1, neueKreditorId), maxZinsIndex, rand);
////        v.bookNewContract();
//    }
//}

QString proposeKennung()
{   LOG_CALL;
    int idOffset = getMetaInfo("IdOffset").toInt();
    int iMaxid = idOffset + getHighestTableId("Vertraege");
    QString kennung;
    do
    {
        QString maxid = QString::number(iMaxid).rightJustified(6, '0');
        QString PI = "DK-" + getMetaInfo("ProjektInitialen");
        kennung = PI + "-" + QString::number(QDate::currentDate().year()) + "-" + maxid;
        QVariant v = ExecuteSingleValueSql("id", "Vertraege", "Kennung='" + kennung + "'");
        if( v.isValid())
            iMaxid++;
        else
            break;
    } while(1);
    return kennung;
}

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


void calculateSummary(DbSummary& dbs)
{   LOG_CALL;
    dbs.AnzahlDkGeber = ExecuteSingleValueSql("count(DISTINCT(KreditorId))", "[Kreditoren],[Vertraege]", "aktiv != 0 AND Kreditoren.id = Vertraege.KreditorId").toInt();

    dbs.AnzahlAuszahlende = ExecuteSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] = 0").toInt();
    dbs.BetragAuszahlende = ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] = 0").toReal();

    dbs.AnzahlThesaurierende= ExecuteSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] != 0").toInt();
    dbs.BetragThesaurierende= ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] != 0").toReal();
    dbs.WertThesaurierende  = ExecuteSingleValueSql("SUM([Wert])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] != 0").toReal();

    dbs.AnzahlAktive  = ExecuteSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] != 0").toInt();
    dbs.BetragAktive  = ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] != 0").toReal();
    dbs.WertAktive    = dbs.BetragAuszahlende+dbs.WertThesaurierende;

    dbs.DurchschnittZins = ExecuteSingleValueSql("SELECT SUM( w*z ) / SUM( W ) FROM  (SELECT MAX(Betrag, Wert) AS W, Zinssaetze.Zinssatz AS Z FROM Vertraege, Zinssaetze WHERE Zinssaetze.id = Vertraege.ZSatz AND Vertraege.aktiv)").toReal();
    dbs.MittlererZins = ExecuteSingleValueSql("SELECT AVG(Zinssaetze.Zinssatz) FROM Vertraege, Zinssaetze WHERE Zinssaetze.id = Vertraege.ZSatz AND Vertraege.aktiv").toDouble();
    dbs.AnzahlPassive = ExecuteSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] = 0").toInt();
    dbs.BetragPassive = ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] = 0").toReal();
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

