#include <QtCore>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QVector>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "helper.h"
#include "filehelper.h"
#include "sqlhelper.h"
#include "csvwriter.h"
#include "finhelper.h"
#include "vertrag.h"
#include "dkdbhelper.h"
#include "kreditor.h"
#include "dbstructure.h"

dbstructure dkdbstructur;
dbstructure dkdbAddtionalTables;

QList<QPair<qlonglong, QString>> Buchungsarten;

void initDKDBStruktur()
{LOG_ENTRY_and_EXIT;
    static bool done = false;
    if( done) return; // 4 tests
    initBuchungsarten();
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

void initAdditionalTables()
{LOG_ENTRY_and_EXIT;
    dbtable briefe("Briefvorlagen");
    briefe.append(dbfield("templateId", QVariant::Int));
    briefe.append(dbfield("EigenschaftId", QVariant::Int));
    briefe.append(dbfield("Wert"));
    dkdbAddtionalTables.appendTable(briefe);
}

void initGmbHData()
{
    initProperty("gmbh.address1", "Esperanza Franklin GmbH");
    initProperty("gmbh.address2", "");
    initProperty("gmbh.plz", "68167");
    initProperty("gmbh.stadt", "Mannheim");
    initProperty("gmbh.strasse", "Turley-Platz 9");
    initProperty("gmbh.email","info@esperanza-mannheim.de");
    initProperty("gmbh.url", "www.esperanza-mannheim.de");
}

void initBuchungsarten()
{LOG_ENTRY_and_EXIT;

    Buchungsarten.push_back(QPair<qlonglong, QString>(Buchungsart_i::NOOP, ""));
    Buchungsarten.push_back(QPair<qlonglong, QString>(Buchungsart_i::VERTRAG_ANLEGEN, "Vertrag anlegen"));
    Buchungsarten.push_back(QPair<qlonglong, QString>(Buchungsart_i::VERTRAG_AKTIVIEREN, "Vertrag aktivieren"));
    Buchungsarten.push_back(QPair<qlonglong, QString>(Buchungsart_i::PASSIVEN_VERTRAG_LOESCHEN, "Passiven Vertrag löschen"));
    Buchungsarten.push_back(QPair<qlonglong, QString>(Buchungsart_i::VERTRAG_BEENDEN, "Vertrag beenden"));
    Buchungsarten.push_back(QPair<qlonglong, QString>(Buchungsart_i::ZINSGUTSCHRIFT, "Zinsgutschrift"));
    Buchungsarten.push_back(QPair<qlonglong, QString>(Buchungsart_i::KUENDIGUNG_FRIST, "Kuendigung mit Frist"));
}

bool ZinssaetzeEinfuegen(QSqlDatabase db)
{LOG_ENTRY_and_EXIT;
    double Zins = 0.;
    double ZinsIncrement = 0.01;
    TableDataInserter ti(dkdbstructur["Zinssaetze"]);
    ti.setValue("Zinssatz", 0.);
    ti.setValue("Bemerkung", "Unser Held");
    bool ret = 0<= ti.InsertData(db);

    for (Zins+=ZinsIncrement; ret && Zins < .6; Zins+=ZinsIncrement)
    {
        ti.setValue("Zinssatz", Zins); ti.setValue("Bemerkung", "Unser Freund");
        ret &= 0<=ti.InsertData(db);
    }
    for (; ret && Zins < 1.1; Zins+=ZinsIncrement){
        ti.setValue("Zinssatz", Zins); ti.setValue("Bemerkung", "Unser Förderer");
        ret &= 0<= ti.InsertData(db);
    }
    for (; ret && Zins < 2.; Zins+=ZinsIncrement)
    {
        ti.setValue("Zinssatz", Zins); ti.setValue("Bemerkung", "Unser Investor");
        ret &= 0<= ti.InsertData(db);
    }
    if( !ret)
        qCritical() << "There was an error creating intrest values";
    return ret;
}

bool BuchungsartenEinfuegen(QSqlDatabase db =QSqlDatabase::database())
{LOG_ENTRY_and_EXIT;
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

bool EigenschaftenEinfuegen(QSqlDatabase db)
{LOG_ENTRY_and_EXIT;
    bool ret =true;
    QSqlQuery sql(db);
    QRandomGenerator *rand = QRandomGenerator::system();
    initProperty("Version", "1.0");
    initProperty("IdOffset", QString::number(rand->bounded(10000,20000)));
    initProperty("ProjektInitialen", "ESP");

    return ret;
}

void initProperty( const QString& name, const QString& wert, const QString& connection)
{LOG_ENTRY_and_EXIT;
    if( getProperty(name, connection)== "")
        setProperty(name, wert, connection);
}

QString getProperty(const QString& name, const QString& connection)
{
    qDebug() << "Reading property... " << name;
    QString value= ExecuteSingleValueSql("SELECT WERT FROM Meta WHERE Name='" + name +"'", connection).toString();
    qDebug() << "... value " << value;
    return value;
}
void setProperty(const QString& name, const QString& Wert, const QString& connection)
{LOG_ENTRY_and_EXIT;
    QSqlQuery q(QSqlDatabase::database(connection));
    q.prepare("INSERT OR REPLACE INTO Meta (Name, Wert) VALUES ('" + name + "', '" + Wert +"')");
    if( !q.exec())
        qCritical() << "Failed to insert Meta information " << q.lastError() << endl << q.lastQuery();
}

bool DKDatenbankAnlegen(QSqlDatabase db)
{LOG_ENTRY_and_EXIT;
    bool ret = true;
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON");
    {
    db.transaction();
    ret &= dkdbstructur.createDb(db);
    ret &= ZinssaetzeEinfuegen(db);
    ret &= BuchungsartenEinfuegen(db);
    ret &= EigenschaftenEinfuegen(db);
    if( ret) db.commit(); else db.rollback();
    }
    if( !ret)
    {
        qCritical() << "creating db structure in new database failed";
        return false;
    }
    return istValideDatenbank(db);
}

bool DKDatenbankAnlegen(const QString& filename) /*in the default connection*/
{LOG_ENTRY_and_EXIT;
    if( filename.isEmpty())
    {
        qCritical() << "call to DKDatenbankAnlegen w/o filename";
        return false;
    }
    if( QFile(filename).exists())
    {
        backupFile(filename);
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
    return DKDatenbankAnlegen(db);
}

bool hatAlleTabellenUndFelder(QSqlDatabase& db)
{LOG_ENTRY_and_EXIT;
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

bool ensureTable( const dbtable& table, const QString& con)
{
    QSqlDatabase db = QSqlDatabase::database(con);
    return ensureTable(table, db);
}

bool ensureTable( const dbtable& table, QSqlDatabase& db)
{
    if( tableExists(table.Name(), db.connectionName()))
    {
        QVector<QString> fields = getFields(table.Name(), db.connectionName());
        for (int i=0; i < table.Fields().count(); i++)
        {
            QString expectedFieldName = table.Fields()[i].name();
            if( fields.indexOf(expectedFieldName) == -1 )
            {
                qDebug() << "ensureTable() failed: table exists with wrong field" << expectedFieldName;
                return false;
            }
        }
        return true;
    }
    // create table
    return table.create(db);
}

bool istValideDatenbank(const QString& filename)
{LOG_ENTRY_and_EXIT;

    if( filename == "") return false;
    if( !QFile::exists(filename)) return false;
    dbCloser closer;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "validate");
    db.setDatabaseName(filename);
    if( !db.open())
        return false;
    closer.set(&db);
    bool ret = istValideDatenbank(db);
    if( !ret)
        qDebug() << "failed to validate databse " << filename;
    return ret;
}

bool istValideDatenbank(QSqlDatabase db)
{LOG_ENTRY_and_EXIT;

    QSqlQuery enableRefInt(db);
    enableRefInt.exec("PRAGMA foreign_keys = ON");
    if( !hatAlleTabellenUndFelder(db))
        return false;

    qDebug() << db.databaseName() << " is a valid dk database";
    return true;
}

void DatenbankverbindungSchliessen(QString con)
{LOG_ENTRY_and_EXIT;

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

void DatenbankZurAnwendungOeffnen( QString newDbFile)
{LOG_ENTRY_and_EXIT;

    DatenbankverbindungSchliessen();
    QSettings config;
    if( newDbFile == "")
    {
        newDbFile = config.value("db/last").toString();
        qInfo() << "opening DbFile read from configuration: " << newDbFile;
    }
    else
        config.setValue("db/last", newDbFile);
    backupFile(newDbFile);

    // setting the default database for the application
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(newDbFile);
    db.open();
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON");
    initGmbHData();
    BuchungsartenEinfuegen();
}

void CheckDbConsistency( QStringList& msg)
{LOG_ENTRY_and_EXIT;
    // temporary; fix data corrupted by date edit control (1752 ...)
    QSqlDatabase db = QSqlDatabase::database();
    db.exec("UPDATE [Vertraege] SET [LaufzeitEnde]='9999-12-31' WHERE [LaufzeitEnde]<[Vertragsdatum]");
    db.exec("UPDATE [Vertraege] SET [LetzteZinsberechnung]='9999-12-31' WHERE NOT([aktiv])");
    db.exec("UPDATE [Vertraege] SET [LaufzeitEnde]='9999-12-31' WHERE [Kfrist] <> -1");
    db.exec("UPDATE [Vertraege] SET [Kfrist]='6' WHERE [LaufzeitEnde]=='9999-12-31' AND [Kfrist] < 1");

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

bool copyTable(QString table, QSqlDatabase targetDB)
{
    bool success = true;
    QSqlQuery q(QSqlDatabase::database(QLatin1String(QSqlDatabase::defaultConnection))); // default database connection -> active database
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

bool copyMangledKreditors(QSqlDatabase targetDB)
{
    bool success = true;
    int recCount = 0;
    QSqlQuery q(QSqlDatabase::database(QLatin1String(QSqlDatabase::defaultConnection))); // default database connection -> active database
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

bool createDbCopy(QString targetfn, bool deper)
{
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
            success = success && copyMangledKreditors(backupDB);
        }
        else
        {
            success = success && copyTable(table.Name(), backupDB);
        }

    }
    return success;
}

void BeispielVertragsdaten( Vertrag& vertrag, int KId, int maxZinsIndex, QRandomGenerator* rand)
{
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
    vertrag = Vertrag(KId, proposeKennung(),
                    betragUWert, betragUWert, zinsid,
                    vertragsdatum,
                    thesa, active, StartZinsberechnung, kFrist, LaufzeitEnde);
}

void BeispieldatenAnlegen( int AnzahlDatensaetze)
{LOG_ENTRY_and_EXIT;

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
        Vertrag v;
        BeispielVertragsdaten(v, neueKreditorId, maxZinsIndex, rand);
        v.verbucheNeuenVertrag();
    }
    for ( int i=0; i<AnzahlDatensaetze; i++)
    {   // more contracts for existing customers
        Vertrag v;
        BeispielVertragsdaten(v, rand->bounded(1, neueKreditorId), maxZinsIndex, rand);
        v.verbucheNeuenVertrag();

    }
}

QString proposeKennung()
{LOG_ENTRY_and_EXIT;
    int idOffset = getProperty("IdOffset").toInt();
    int iMaxid = idOffset + getHighestTableId("Vertraege");
    QString kennung;
    do
    {
        QString maxid = QString::number(iMaxid).rightJustified(6, '0');
        QString PI = "DK-" + getProperty("ProjektInitialen");
        kennung = PI + "-" + QString::number(QDate::currentDate().year()) + "-" + maxid;
        QVariant v = ExecuteSingleValueSql("id", "Vertraege", "Kennung='" + kennung + "'");
        if( v.isValid())
            iMaxid++;
        else
            break;
    } while(1);
    return kennung;
}

void ZinssaetzeFuerAuswahlliste(QList<ZinsAnzeigeMitId>& Rates)
{LOG_ENTRY_and_EXIT;

    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("SELECT id, Zinssatz, Bemerkung FROM Zinssaetze ORDER BY Zinssatz DESC");
    if( !query.exec())
    {
        qCritical() << "Error reading Interrest Rates while creating a contract: " << QSqlDatabase::database().lastError().text();
    }
    while(query.next())
    {
        ZinsAnzeigeMitId entry{ query.value("id").toInt(), (query.value("Zinssatz").toString() + "  (" + query.value("Bemerkung").toString() + ")  ")};
        Rates.append(entry);
    }
}

QString ContractList_SELECT(const QVector<dbfield>& fields)
{LOG_ENTRY_and_EXIT;
    QString sql("SELECT ");
    for( int i = 0; i < fields.size(); i++)
    {
        if( i) sql +=", ";
        sql += fields[i].tableName() +"." +fields[i].name();
    }
    return sql;
}

QString ContractList_FROM()
{LOG_ENTRY_and_EXIT;
    return  "FROM Vertraege, Kreditoren, Zinssaetze";
}
QString ContractList_WHERE(const QString& Filter)
{LOG_ENTRY_and_EXIT;
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
QString ContractList_SQL(const QVector<dbfield>& fields, const QString& filter)
{LOG_ENTRY_and_EXIT;
    QString sql = ContractList_SELECT(fields) + " "
            + ContractList_FROM() + " "
            + ContractList_WHERE(filter);
    qDebug() << "ContractList SQL: \n" << sql;
    return sql;
}


void berechneZusammenfassung(DbSummary& dbs, QString con)
{LOG_ENTRY_and_EXIT;
    dbs.AnzahlDkGeber = ExecuteSingleValueSql("COUNT(*)", "[Kreditoren]", "1=1").toInt();

    dbs.AnzahlAuszahlende = ExecuteSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] = 0", con).toInt();
    dbs.BetragAuszahlende = ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] = 0", con).toReal();

    dbs.AnzahlThesaurierende= ExecuteSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] != 0", con).toInt();
    dbs.BetragThesaurierende= ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] != 0", con).toReal();
    dbs.WertThesaurierende  = ExecuteSingleValueSql("SUM([Wert])", "[Vertraege]", "[aktiv] != 0 AND [thesaurierend] != 0", con).toReal();

    dbs.AnzahlAktive  = ExecuteSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] != 0", con).toInt();
    dbs.BetragAktive  = ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] != 0", con).toReal();
    dbs.WertAktive    = dbs.BetragAuszahlende+dbs.WertThesaurierende;

    dbs.DurchschnittZins = ExecuteSingleValueSql("SELECT SUM( w*z ) / SUM( W ) FROM  (SELECT MAX(Betrag, Wert) AS W, Zinssaetze.Zinssatz AS Z FROM Vertraege, Zinssaetze WHERE Zinssaetze.id = Vertraege.ZSatz AND Vertraege.aktiv)").toReal();
    dbs.MittlererZins = ExecuteSingleValueSql("SELECT AVG(Zinssaetze.Zinssatz) FROM Vertraege, Zinssaetze WHERE Zinssaetze.id = Vertraege.ZSatz AND Vertraege.aktiv").toDouble();
    dbs.AnzahlPassive = ExecuteSingleValueSql("COUNT([Betrag])", "[Vertraege]", "[aktiv] = 0", con).toInt();
    dbs.BetragPassive = ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] = 0", con).toReal();
}

void CsvActiveContracts()
{
    QDate today = QDate::currentDate();
    QString filename(today.toString(Qt::ISODate) + "-Aktive-Vertraege.csv");

    QVector<dbfield> fields;
    fields.append(dkdbstructur["Vertraege"]["id"]);
    fields.append(dkdbstructur["Vertraege"]["KreditorId"]);
    fields.append(dkdbstructur["Kreditoren"]["Vorname"]);
    fields.append(dkdbstructur["Kreditoren"]["Nachname"]);
    fields.append(dkdbstructur["Kreditoren"]["Strasse"]);
    fields.append(dkdbstructur["Kreditoren"]["Stadt"]);
    fields.append(dkdbstructur["Kreditoren"]["Nachname"]);
    fields.append(dkdbstructur["Vertraege"]["Betrag"]);
    fields.append(dkdbstructur["Vertraege"]["Wert"]);
    fields.append(dkdbstructur["Vertraege"]["Vertragsdatum"]);

    table2csv( fields, "[aktiv] = 1", filename);
    showFileInFolder(filename);
}

void berechneVertragsenden( QVector<ContractEnd>& ce, QString connection)
{LOG_ENTRY_and_EXIT;

    QMap<int, int> m_count;
    QMap<int, double> m_sum;
    const int maxYear = QDate::currentDate().year() +99;

    QSqlQuery sql(QSqlDatabase::database(connection));
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

void berechneJahrZinsVerteilung( QVector<YZV>& yzv, QString connection)
{
    QString sql = "SELECT Substr([Vertragsdatum], 0, 5), [Zinssaetze].[Zinssatz], count(*), sum([Betrag]) "
                  "FROM [Vertraege], [Zinssaetze] "
                  "WHERE [ZSatz] = [Zinssaetze].[id] "
                  "GROUP BY Substr([Vertragsdatum], 0, 4), [ZSatz]";
    QSqlQuery query(connection);
    query.exec(sql);
    while( query.next())
    {
        QSqlRecord r =query.record();
        yzv.push_back({r.value(0).toInt(), r.value(1).toReal(), r.value(2).toInt(), r.value(3).toReal() });
    }
    return;
}

QString LaufzeitenVerteilungHtml(QString con)
{
    int AnzahlBisEinJahr=0, AnzahlBisFuenfJahre=0, AnzahlLaenger=0, AnzahlUnbegrenzet = 0;
    double SummeBisEinJahr=0., SummeBisFuenfJahre=0., SummeLaenger=0., SummeUnbegrenzet = 0.;
    QString sql = "SELECT [Betrag], [Wert], [Vertragsdatum], [LaufzeitEnde] FROM [Vertraege]";
    QSqlQuery q (con);
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
    QString ret="<table><thead><tr><td>Zeitraum</td><td>Anzahl</td><td>Wert</td></tr></thead>";
    ret += "<tr><td align=left>Bis ein Jahr </td><td align=center>"+ QString::number(AnzahlBisEinJahr) + "</td><td align=right>";
    ret += locale.toCurrencyString(SummeBisEinJahr) + "</td></tr>";
    ret += "<tr><td align=left>Ein bis fünf Jahre </td><td align=center>"+ QString::number(AnzahlBisFuenfJahre) + "</td><td align=right>";
    ret += locale.toCurrencyString(SummeBisFuenfJahre) + "</td></tr>";
    ret += "<tr><td align=left>Länger als fünf Jahre </td><td align=center>"+ QString::number(AnzahlLaenger) + "</td><td align=right>";
    ret += locale.toCurrencyString(SummeLaenger) + "</td></tr>";
    ret += "<tr><td align=left>Unbegrenzte Verträge </td><td align=center>"+ QString::number(AnzahlUnbegrenzet) + "</td><td align=right>";
    ret += locale.toCurrencyString(SummeUnbegrenzet) + "</td></tr></table>";
    return ret;
}

