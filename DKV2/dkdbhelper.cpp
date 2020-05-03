#include <float.h>
#include <QtCore>
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
#include "vertrag.h"
#include "dkdbhelper.h"
#include "kreditor.h"
#include "dbstructure.h"

const double CURRENT_DB_VERSION {1.0};
const QString DB_VERSION {"Version"};

dbstructure dkdbstructur;
dbstructure dkdbAddtionalTables;

QList<QPair<qlonglong, QString>> Buchungsarten;

void init_DKDBStruct()
{   LOG_CALL_W("Setting up internal database structures");
    static bool done = false;
    if( done) return; // 4 tests
    init_bookingTypes();
    static bool init_done = false;
    if( init_done) return;
    init_done = true;
    // DB date -> Variant String
    // DB bool -> Variant int
    dbtable Kreditoren("Kreditoren");
    Kreditoren.append(dbfield("id",       QVariant::LongLong,    "PRIMARY KEY AUTOINCREMENT"));
    Kreditoren.append(dbfield("Vorname",  QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("Nachname", QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("Strasse",  QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("Plz",      QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("Stadt",    QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("Email"));
    Kreditoren.append(dbfield("Anmerkung"));
    Kreditoren.append(dbfield("IBAN"));
    Kreditoren.append(dbfield("BIC"));
    QVector<dbfield> unique;
    unique.append(Kreditoren["Vorname"]);
    unique.append(Kreditoren["Nachname"]);
    unique.append(Kreditoren["Strasse"]);
    unique.append(Kreditoren["Stadt"]);
    Kreditoren.setUnique(unique);
    dkdbstructur.appendTable(Kreditoren);

    dbtable Zinssaetze("Zinssaetze");
    Zinssaetze.append(dbfield("id",       QVariant::LongLong,    "PRIMARY KEY AUTOINCREMENT"));
    Zinssaetze.append(dbfield("Zinssatz", QVariant::Double, "DEFAULT '0,0' UNIQUE NULL"));
    Zinssaetze.append(dbfield("Bemerkung"));
    dkdbstructur.appendTable(Zinssaetze);

    dbtable Vertraege("Vertraege");
    Vertraege.append(dbfield("id",         QVariant::LongLong, "PRIMARY KEY AUTOINCREMENT"));
    Vertraege.append(dbfield("KreditorId", QVariant::LongLong, "", Kreditoren["id"], dbfield::refIntOption::onDeleteCascade ));
    Vertraege.append(dbfield("Kennung",    QVariant::String, "UNIQUE"));
    Vertraege.append(dbfield("Betrag",     QVariant::Double, "DEFAULT '0,0' NOT NULL"));
    Vertraege.append(dbfield("Wert",       QVariant::Double, "DEFAULT '0,0' NULL"));
    Vertraege.append(dbfield("ZSatz",      QVariant::LongLong, "", Zinssaetze["id"], dbfield::refIntOption::non));
    Vertraege.append(dbfield("thesaurierend", QVariant::Bool, "DEFAULT '1' NOT NULL"));
    Vertraege.append(dbfield("Vertragsdatum", QVariant::Date, "DATE  NULL"));
    Vertraege.append(dbfield("aktiv",         QVariant::Bool, "DEFAULT '0' NOT NULL"));
    Vertraege.append(dbfield("LaufzeitEnde",  QVariant::Date, "DEFAULT '3000-12-31' NOT NULL"));
    Vertraege.append(dbfield("LetzteZinsberechnung", QVariant::Date, "NULL"));
    Vertraege.append(dbfield("Kfrist" ,    QVariant::Int, "DEFAULT '6' NOT NULL"));
    dkdbstructur.appendTable(Vertraege);

    dbtable Buchungsarten("Buchungsarten");
    Buchungsarten.append(dbfield("id",  QVariant::LongLong, "PRIMARY KEY"));
    Buchungsarten.append(dbfield("Art", QVariant::String, "NOT NULL"));
    dkdbstructur.appendTable(Buchungsarten);

    dbtable Buchungen("Buchungen");
    Buchungen.append(dbfield("id",           QVariant::LongLong, "PRIMARY KEY AUTOINCREMENT"));
    Buchungen.append(dbfield("VertragId",    QVariant::LongLong, "", Vertraege["id"], dbfield::refIntOption::onDeleteNull));
    Buchungen.append(dbfield("Buchungsart",  QVariant::LongLong, "", Buchungsarten["id"], dbfield::refIntOption::non));
    Buchungen.append(dbfield("Betrag",       QVariant::Double, "DEFAULT '0' NULL"));
    Buchungen.append(dbfield("Datum",        QVariant::Date));
    Buchungen.append(dbfield("Bemerkung",    QVariant::String));
    Buchungen.append(dbfield("Buchungsdaten",    QVariant::String));
    dkdbstructur.appendTable(Buchungen);

    dbtable meta("Meta");
    meta.append(dbfield("Name", QVariant::String, "NOT NULL"));
    meta.append(dbfield("Wert", QVariant::String, "NOT NULL"));
    dkdbstructur.appendTable(meta);
    done = true;
}

void init_additionalTables()
{   LOG_CALL;
    static bool done = false;
    if( done) return; // 4 tests
    dbtable briefe("Briefvorlagen");
    briefe.append(dbfield("templateId", QVariant::Int));
    briefe.append(dbfield("EigenschaftId", QVariant::Int));
    briefe.append(dbfield("Wert"));
    dkdbAddtionalTables.appendTable(briefe);
    done =true;
}

void init_GmbHData( QSqlDatabase db = defaultDb())
{   LOG_CALL;
    initMetaInfo("gmbh.address1", "Esperanza Franklin GmbH", db);
    initMetaInfo("gmbh.address2", "", db);
    initMetaInfo("gmbh.plz", "68167", db);
    initMetaInfo("gmbh.stadt", "Mannheim", db);
    initMetaInfo("gmbh.strasse", "Turley-Platz 9", db);
    initMetaInfo("gmbh.email","info@esperanza-mannheim.de", db);
    initMetaInfo("gmbh.url", "www.esperanza-mannheim.de", db);
}

void init_bookingTypes()
{   LOG_CALL;

    Buchungsarten.push_back(QPair<qlonglong, QString>(Buchungsart_i::NOOP, ""));
    Buchungsarten.push_back(QPair<qlonglong, QString>(Buchungsart_i::VERTRAG_ANLEGEN, "Vertrag anlegen"));
    Buchungsarten.push_back(QPair<qlonglong, QString>(Buchungsart_i::VERTRAG_AKTIVIEREN, "Vertrag aktivieren"));
    Buchungsarten.push_back(QPair<qlonglong, QString>(Buchungsart_i::PASSIVEN_VERTRAG_LOESCHEN, "Passiven Vertrag löschen"));
    Buchungsarten.push_back(QPair<qlonglong, QString>(Buchungsart_i::VERTRAG_BEENDEN, "Vertrag beenden"));
    Buchungsarten.push_back(QPair<qlonglong, QString>(Buchungsart_i::ZINSGUTSCHRIFT, "Zinsgutschrift"));
    Buchungsarten.push_back(QPair<qlonglong, QString>(Buchungsart_i::KUENDIGUNG_FRIST, "Kuendigung mit Frist"));
}

bool insert_interestRates(QSqlDatabase db =defaultDb())
{   dbgTimer timer(__func__);
    double Zins = 0.;
    double ZinsIncrement = 0.01;
    TableDataInserter ti(dkdbstructur["Zinssaetze"]);
    ti.setValue("Zinssatz", 0.);
    ti.setValue("Bemerkung", "Unser Held");
    bool ret = 0<= ti.InsertData(db);

    QSqlQuery sql(db);
    sql.prepare("INSERT INTO Zinssaetze VALUES (NULL, :z, :b)");

    for (Zins+=ZinsIncrement; ret && Zins < .6; Zins+=ZinsIncrement)
    {
        dbgTimer t1("timer 1");
        sql.bindValue(":z", Zins);
        sql.bindValue(":b", "Unser Freund");
        if( sql.exec())
            ;//qInfo() << "inserted interest value " << Zins;
        else
        {
            qCritical() << "failed to insert interest value " << Zins;
            ret = false;
        }
    }
    for (; ret && Zins < 1.1; Zins+=ZinsIncrement)
    {
        dbgTimer t1("timer 2");
        sql.bindValue(":z", Zins);
        sql.bindValue(":b", "Unser Förderer");
        if( sql.exec())
            ;//qInfo() << "inserted interest value " << Zins;
        else
        {
            qCritical() << "failed to insert interest value " << Zins;
            ret = false;
        }
    }
    for (; ret && Zins < 2.; Zins+=ZinsIncrement)
    {
        dbgTimer t1("timer 3");
        sql.bindValue(":z", Zins);
        sql.bindValue(":b", "Unser Investor");
        if( sql.exec())
            ;//qInfo() << "inserted interest value " << Zins;
        else
        {
            qCritical() << "failed to insert interest value " << Zins;
            ret = false;
        }
    }
    if( !ret)
        qCritical() << "There was an error creating intrest values";
    return ret;
}

bool insert_bookingTypes(QSqlDatabase db =defaultDb())
{   LOG_CALL;
    dbgTimer timer (__func__);
    bool ret = true;
    for( auto art: Buchungsarten)
    {
        TableDataInserter ti( dkdbstructur["Buchungsarten"]);
        ti.setValue("id", QVariant(art.first));
        ti.setValue("Art", QVariant(art.second));

        ret &= 0<= ti.InsertOrReplaceData(db);
    }
    return ret;
}

bool insert_Properties(QSqlDatabase db =defaultDb())
{   LOG_CALL;
    bool ret =true;
    QSqlQuery sql(db);
    QRandomGenerator *rand = QRandomGenerator::system();
    initNumMetaInfo(DB_VERSION, CURRENT_DB_VERSION, db);
    initMetaInfo("IdOffset", QString::number(rand->bounded(10000,20000)), db);
    initMetaInfo("ProjektInitialen", "ESP", db);
    return ret;
}

void initMetaInfo( const QString& name, const QString& newValue, QSqlDatabase db)
{   LOG_CALL;
    QVariant value= ExecuteSingleValueSql("SELECT WERT FROM Meta WHERE Name='" + name +"'", db);
    if( value.type() == QVariant::Type::Invalid)
        setMetaInfo(name, newValue, db);
}
void initNumMetaInfo( const QString& name, const double& newValue, QSqlDatabase db)
{   LOG_CALL;
    QVariant value= ExecuteSingleValueSql("SELECT WERT FROM Meta WHERE Name='" + name +"'", db);
    if( value.type() == QVariant::Type::Invalid)
        setNumMetaInfo(name, newValue, db);
}
QString getMetaInfo(const QString& name, QSqlDatabase db)
{   LOG_CALL_W(name);
    QVariant value= ExecuteSingleValueSql("SELECT WERT FROM Meta WHERE Name='" + name +"'", db).toString();
    if( value.type() == QVariant::Type::Invalid)
    {
        qDebug() << "read empty property " << name << "; defaulted to empty string";
        return "";
    }
    qDebug() << "Property " << name << " : " << value;
    return value.toString();
}
double getNumMetaInfo(const QString& name, QSqlDatabase db)
{   LOG_CALL_W(name);

    QVariant value= ExecuteSingleValueSql("SELECT WERT FROM Meta WHERE Name='" + name +"'", db);
    if( value.type() == QVariant::Type::Invalid)
    {
        qDebug() << "getNumProperty read empty property " << name << " defaulted to 0.";
        return 0.;
    }
    qDebug() << "Property " << name << " : " << value.toDouble();
    return value.toDouble();
}
void setMetaInfo(const QString& name, const QString& Wert, QSqlDatabase db)
{   LOG_CALL_W(name);
    QSqlQuery q(db);
    q.prepare("INSERT OR REPLACE INTO Meta (Name, Wert) VALUES ('" + name + "', '" + Wert +"')");
    if( !q.exec())
        qCritical() << "Failed to insert Meta information " << q.lastError() << endl << q.lastQuery();
}
void setNumMetaInfo(const QString& name, const double Wert, QSqlDatabase db)
{   LOG_CALL_W(name);
    QSqlQuery q(db);
    q.prepare("INSERT OR REPLACE INTO Meta (Name, Wert) VALUES ('" + name + "', '" + QString::number(Wert) +"')");
    if( !q.exec())
        qCritical() << "Failed to insert Meta information " << q.lastError() << endl << q.lastQuery();
}

bool create_DK_databaseContent(QSqlDatabase db)
{   LOG_CALL_W(db.databaseName());
    bool ret = true;
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON", db);

    db.transaction();
    do{
        if(! (ret &= createTables( dkdbstructur, db))) break;
        if(! (ret &= insert_interestRates(db))) break;
        if(! (ret &= insert_bookingTypes(db))) break;
        if(! (ret &= insert_Properties(db))) break;
    }while(false);

    if(!ret)
    {
        db.rollback();
        qCritical() << "creating db structure in new database failed";
        return false;
    }

    db.commit();
    return isValidDatabase(db);
}
bool create_DK_databaseFile(const QString& filename) /*in the default connection*/
{   //LOG_CALL_W("filename: " + filename);
    Q_ASSERT(!filename.isEmpty());
    dbgTimer timer( QString(__func__) + QString(" (") + filename + QString(")"));
    if( QFile(filename).exists())
    {
        backupFile(filename, "db-bak");
        QFile(filename).remove();
        if( QFile(filename).exists())
        {
            qCritical() << "file to be replaced can not be deleted";
            return false;
        }
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filename);

    if( !db.open())
    {
        qDebug() << "DkDatenbankAnlegen failed in db.open";
        return false;
    }
    return create_DK_databaseContent(db);
}

bool has_allTablesAndFields(QSqlDatabase db)
{   LOG_CALL;
    for( auto table : dkdbstructur.getTables())
    {
        QSqlQuery sql(db);
        sql.prepare(QString("SELECT * FROM ") + table.Name() +" LIMIT 0");
        if( !sql.exec())
        {
            qDebug() << "testing for table " << table.Name() << " failed\n" << sql.lastError() << endl << sql.lastQuery();
            return false;
        }
        QSqlRecord r=sql.record();
        for(int i = 0; i< r.count(); i++ )
        {
            QString fieldname = r.fieldName(i);
            if( table[fieldname] == dbfield())
            {
                qDebug() << "testing for field" << fieldname << " failed\n" << sql.lastError() << endl << sql.lastQuery();
                return false;
            }
        }
        if( table.Fields().count() != sql.record().count())
        {
            qCritical() << "Tabelle " << table.Name() << " hat nicht die richtige Anzahl Felder";
            return false;
        }
    }
    qDebug() << db.databaseName() << " has all tables expected";
    return true;
}

bool isValidDatabase(const QString& filename)
{   LOG_CALL_W(filename);

    QString msg;
    if( filename == "") msg = "empty filename";
    else if( !QFile::exists(filename)) msg = "file not found";
    else
    {
        dbCloser closer;
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "validate");
        db.setDatabaseName(filename);
        if( !db.open()) msg = "open db failed";
        else
        {
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
    if( !check_db_version(db))
    {
        qCritical() << "database version check failed";
        return false;
    }
    qDebug() << db.databaseName() << " is a valid dk database";
    return true;
}

void closeDatabaseConnection(QString con)
{   LOG_CALL_W(con);

    QSqlDatabase::removeDatabase(con);
    QList<QString> cl = QSqlDatabase::connectionNames();
    if( cl.count() == 0)
        return;
    if( cl.count() > 0)
    {
        qDebug() << "Found " << cl.count() << "connections open, after closing  \"" + con +"\"";
        return;
    }

    qInfo() << "Database connection " << con << " removed";
}

bool check_db_version(QSqlDatabase db)
{   LOG_CALL;
    double d = getNumMetaInfo(DB_VERSION, db);
    if( d >= CURRENT_DB_VERSION)
        return true;
    qDebug() << "db version check failed: found version " << d << " needed version " << CURRENT_DB_VERSION;
    return false;
}

bool open_databaseForApplication( QString newDbFile)
{   LOG_CALL;

    closeDatabaseConnection();
    if( newDbFile == "")
    {
        newDbFile = appConfig::CurrentDb();
        qInfo() << "opening DbFile read from configuration: " << newDbFile;
    }
    backupFile(newDbFile, "db-bak");

    // setting the default database for the application
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(newDbFile);
    if( !db.open())
    {   qDebug() << "open database file " << newDbFile << " failed";
        return false;
    }
    if( !check_db_version(db))
        return false;

    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON");
    init_GmbHData();
    insert_bookingTypes();
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
    while(iban_q.next())
    {
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
    QSqlQuery q(defaultDb()); // default database connection -> active database
    q.prepare("SELECT * FROM " + table);
    q.exec();
    while( q.next())
    {
        QSqlRecord rec = q.record();
        qDebug() << "dePe Copy: working on Record " << rec;
        TableDataInserter tdi( dkdbstructur[table]);
        for( int iField = 0; iField < q.record().count(); iField++)
        {
            QString fieldname = rec.fieldName(iField);
            QVariant value = rec.value(iField);
            qDebug() << "Setting " << fieldname << " to " << value;
            tdi.setValue(fieldname, value);
        }
        if( tdi.InsertData(targetDB) == -1)
        {
            qDebug() << "Error inserting Data into deperso.Copy Table" << q.record();
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
    QSqlQuery q(defaultDb()); // default database connection -> active database
    q.prepare("SELECT * FROM Kreditoren");
    q.exec();
    while( q.next())
    {
        recCount++;
        QSqlRecord rec = q.record();
        qDebug() << "dePe Copy: working on Record " << rec;
        TableDataInserter tdi(dkdbstructur["Kreditoren"]);

        tdi.setValue("Vorname", QString("Vorname")+QString::number(recCount));
        tdi.setValue("Nachname", QString("Nachname")+QString::number(recCount));
        tdi.setValue("Strasse", QString("Strasse"));
        tdi.setValue("Plz", QString("D-xxxxx"));
        tdi.setValue("Stadt", QString("Stadt"));

        if( tdi.InsertData(targetDB) == -1)
        {
            qDebug() << "Error inserting Data into deperso.Copy Table" << q.record();
            success = false;
            break;
        }
    }
    return success;
}

bool create_DB_copy(QString targetfn, bool deper)
{   LOG_CALL_W(targetfn);
    if( QFile::exists(targetfn))
    {
        backupFile(targetfn);
        QFile::remove(targetfn);
        if( QFile::exists(targetfn))
        {
            qCritical() << "could not remove target file";
            return false;
        }
    }

    dbCloser closer;
    QSqlDatabase backupDB = QSqlDatabase::addDatabase("QSQLITE", "backup");
    backupDB.setDatabaseName(targetfn);

    if( !backupDB.open())
    {
        qDebug() << "faild to open backup database";
        return false;
    }
    else
        closer.set(&backupDB);

    if( !dkdbstructur.createDb(backupDB))
    {
        qDebug() << "faild to create db schema";
        return false;
    }
    bool success = true;
    QVector<dbtable> tables = dkdbstructur.getTables();
    for( auto table : tables)
    {
        if( deper && table.Name() == "Buchungen")
        {
            qDebug() << "de personalisation mode: skipping 'Buchungen' ";
            continue;
        }
        qDebug() << "dePe Copy: working on table " << table.Name();
        if( deper && table.Name() == "Kreditoren")
        {
            success = success && copy_mangledCreditors(backupDB);
        }
        else
        {
            success = success && copy_Table(table.Name(), backupDB);
        }

    }
    return success;
}

void create_sampleDataset( Contract& vertrag, int KId, int maxZinsIndex, QRandomGenerator* rand)
{   LOG_CALL;
    // add a contract
    double betragUWert = double(100) * rand->bounded(1,20);
    int zinsid = rand->bounded(1,maxZinsIndex);
    bool thesa = rand->bounded(100)%4 ? true : false;  // 75% thesaurierend
    bool active = rand->bounded(100)%6 ? true : false; // 85% inaktiv
    QDate vertragsdatum= QDate::currentDate().addDays(-1 * rand->bounded(365));
    QDate StartZinsberechnung = ( active) ? vertragsdatum.addDays(rand->bounded(15)) : EndOfTheFuckingWorld;
    QDate LaufzeitEnde = (rand->bounded(100)%8 == 1)
                             ? (QDate(9999, 12, 31)) // kein Ende vereinbart
                             : StartZinsberechnung.addDays( 500+ rand->bounded(0, 8000));
    int kFrist = -1;
    if( LaufzeitEnde == QDate(9999, 12, 31))
    {
        kFrist = rand->bounded(3, 25);
    }
    vertrag = Contract(KId, proposeKennung(),
                    betragUWert, betragUWert, zinsid,
                    vertragsdatum,
                    thesa, active, StartZinsberechnung, kFrist, LaufzeitEnde);
}

void create_sampleData( int AnzahlDatensaetze)
{   LOG_CALL;

    QList<QString> Vornamen {"Holger", "Volker", "Peter", "Hans", "Susi", "Roland", "Claudia", "Emil", "Evelyn", "Ötzgür", "Thomas", "Elke", "Berta", "Malte", "Jori", "Paul", "Jonas", "Finn", "Leon", "Luca", "Emma", "Mia", "Lena", "Anna", "Anne", "Martha", "Ruth", "Rosemie", "Rosemarie", "Verena", "Ursula", "Erika", "Adrian", "Avan", "Anton", "Benno", "Karl", "Merlin", "Noah", "Oliver", "Olaf", "Pepe", "Zeno"};
    QList<QString> Nachnamen {"Maier", "Müller", "Schmit", "Kramp", "Adams", "Häcker", "Maresch", "Beutl", "Chauchev", "Chen", "Kirk", "Ohura", "Gorbatschov", "Merkel", "Karrenbauer", "Tritin", "Schmidt", "Rao", "Lassen", "Hurgedü", "vom Dach", "Langstrumpf", "Lederstrumpf", "Potter", "Poppins", "Wisley", "Li", "Wang", "Ran"};
    QList<QString> Strassen {"Hauptstrasse", "Nebenstrasse", "Bahnhofstrasse", "Kirchstraße", "Dorfstrasse", "Süterlinweg", "Sorbenstrasse", "Kleines Gässchen", "Industriestrasse", "Sesamstrasse", "Lindenstrasse", "Theaterstrasse", "Museumsstrasse", "Opernplatz", "Schillerstrasse", "Lessingstrasse", "Rathausplatz", "Parkstrasse", "Turmstrasse" };
    QList<QString> emailprovider {"gmail.com", "googlemail.com", "mailbox.org", "t-online.de", "mail.de", "mail.com", "online.de", "yahoo.de", "yahoo.com", "telekom.de", "proivder.co.uk", "AOL.de", "outlook.com", "microsoft.com", "sap.com", "sap-ag.de", "abb.de"};
    QList<QString> ibans {"BG80BNBG96611020345678", "DE38531742365852502530", "DE63364408232964251731", "DE38737364268384258531", "DE69037950954001627624", "DE63377045386819730665", "DE18851420444163951769", "DE77921850720298609321", "DE70402696485599313572", "DE70455395581860402838", "DE94045704387963352767", "DE30724236236062816411", "DE62772043290447861437", "DE33387723124963875990", "DE15867719874951165967",
                          "DE96720348741083219766", "DE23152931057149592044", "DE13220161295670898833", "DE49737651031822324605", "DE38017168378078601588", "DE07717138875827514267"};
    QList <QPair<QString, QString>> Cities {{"68305", "Mannheim"}, {"69123", "Heidelberg"}, {"69123", "Karlsruhe"}, {"90345", "Hamburg"}};
    QRandomGenerator *rand = QRandomGenerator::system();
    int maxZinsIndex = ExecuteSingleValueSql("SELECT max(id) FROM Zinssaetze").toInt();
    int neueKreditorId =0;
    for( int i = 0; i<AnzahlDatensaetze; i++)
    {
        Kreditor k;
        QString vn (Vornamen [rand->bounded(Vornamen.count ())]);
        QString nn (Nachnamen [rand->bounded(Nachnamen.count ())]);
        k.setValue("Vorname", vn);
        k.setValue("Nachname", nn);
        k.setValue("Strasse", Strassen[rand->bounded(Strassen.count())]);
        k.setValue("Plz", Cities[rand->bounded(Cities.count())].first);
        k.setValue("Stadt", Cities[rand->bounded(Cities.count())].second);
        k.setValue("Email", vn+"."+nn+"@"+emailprovider[rand->bounded(emailprovider.count())]);
        k.setValue("IBAN", ibans[rand->bounded(ibans.count())]);
        k.setValue("BIC", "bic...");

        neueKreditorId =k.Speichern();
        if( -1 == neueKreditorId)
        {
            qCritical() << "No id from Kreditor.Speichern";
            Q_ASSERT(!bool("Verbuchung des neuen Vertrags gescheitert"));
        }
        Contract v;
        create_sampleDataset(v, neueKreditorId, maxZinsIndex, rand);
        v.bookNewContract();
    }
    for ( int i=0; i<AnzahlDatensaetze; i++)
    {   // more contracts for existing customers
        Contract v;
        create_sampleDataset(v, rand->bounded(1, neueKreditorId), maxZinsIndex, rand);
        v.bookNewContract();
    }
}

QString proposeKennung(QSqlDatabase db)
{   LOG_CALL;
    int idOffset = getMetaInfo("IdOffset", db).toInt();
    int iMaxid = idOffset + getHighestTableId("Vertraege");
    QString kennung;
    do
    {
        QString maxid = QString::number(iMaxid).rightJustified(6, '0');
        QString PI = "DK-" + getMetaInfo("ProjektInitialen", db);
        kennung = PI + "-" + QString::number(QDate::currentDate().year()) + "-" + maxid;
        QVariant v = ExecuteSingleValueSql("id", "Vertraege", "Kennung='" + kennung + "'");
        if( v.isValid())
            iMaxid++;
        else
            break;
    } while(1);
    return kennung;
}

void interestRates_for_dropdown(QList<ZinsAnzeigeMitId>& Rates)
{   LOG_CALL;

    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("SELECT id, Zinssatz, Bemerkung FROM Zinssaetze ORDER BY Zinssatz DESC");
    if( !query.exec())
    {
        qCritical() << "Error reading Interrest Rates while creating a contract: " << defaultDb().lastError().text();
    }
    while(query.next())
    {
        ZinsAnzeigeMitId entry{ query.value("id").toInt(), (query.value("Zinssatz").toString() + "  (" + query.value("Bemerkung").toString() + ")  ")};
        Rates.append(entry);
    }
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


void calculateSummary(DbSummary& dbs, QSqlDatabase db)
{   LOG_CALL;
    dbs.AnzahlDkGeber = ExecuteSingleValueSql("count(DISTINCT(KreditorId))", "[Kreditoren],[Vertraege]", "aktiv != 0 AND Kreditoren.id = Vertraege.KreditorId").toInt();

    dbs.AnzahlAuszahlende = ExecuteSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] = 0", db).toInt();
    dbs.BetragAuszahlende = ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] = 0", db).toReal();

    dbs.AnzahlThesaurierende= ExecuteSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] != 0", db).toInt();
    dbs.BetragThesaurierende= ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] != 0", db).toReal();
    dbs.WertThesaurierende  = ExecuteSingleValueSql("SUM([Wert])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] != 0", db).toReal();

    dbs.AnzahlAktive  = ExecuteSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] != 0", db).toInt();
    dbs.BetragAktive  = ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] != 0", db).toReal();
    dbs.WertAktive    = dbs.BetragAuszahlende+dbs.WertThesaurierende;

    dbs.DurchschnittZins = ExecuteSingleValueSql("SELECT SUM( w*z ) / SUM( W ) FROM  (SELECT MAX(Betrag, Wert) AS W, Zinssaetze.Zinssatz AS Z FROM Vertraege, Zinssaetze WHERE Zinssaetze.id = Vertraege.ZSatz AND Vertraege.aktiv)").toReal();
    dbs.MittlererZins = ExecuteSingleValueSql("SELECT AVG(Zinssaetze.Zinssatz) FROM Vertraege, Zinssaetze WHERE Zinssaetze.id = Vertraege.ZSatz AND Vertraege.aktiv").toDouble();
    dbs.AnzahlPassive = ExecuteSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] = 0", db).toInt();
    dbs.BetragPassive = ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] = 0", db).toReal();
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
void calc_contractEnd( QVector<ContractEnd>& ce, QSqlDatabase db)
{   LOG_CALL;

    QMap<int, int> m_count;
    QMap<int, double> m_sum;
    const int maxYear = QDate::currentDate().year() +99;

    QSqlQuery sql(db);
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
void calc_anualInterestDistribution( QVector<YZV>& yzv, QSqlDatabase db)
{   LOG_CALL;
    QString sql = "SELECT Substr([Vertragsdatum], 0, 5), [Zinssaetze].[Zinssatz], count(*), sum([Betrag]) "
                  "FROM [Vertraege], [Zinssaetze] "
                  "WHERE [ZSatz] = [Zinssaetze].[id] "
                  "GROUP BY Substr([Vertragsdatum], 0, 4), [ZSatz]";
    QSqlQuery query(db);
    query.exec(sql);
    while( query.next())
    {
        QSqlRecord r =query.record();
        yzv.push_back({r.value(0).toInt(), r.value(1).toReal(), r.value(2).toInt(), r.value(3).toReal() });
    }
    return;
}
QVector<rowData> contractRuntimeDistribution(QSqlDatabase db)
{
    int AnzahlBisEinJahr=0, AnzahlBisFuenfJahre=0, AnzahlLaenger=0, AnzahlUnbegrenzet = 0;
    double SummeBisEinJahr=0., SummeBisFuenfJahre=0., SummeLaenger=0., SummeUnbegrenzet = 0.;
    QString sql = "SELECT [Betrag], [Wert], [Vertragsdatum], [LaufzeitEnde] FROM [Vertraege]";
    QSqlQuery q (db);
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

