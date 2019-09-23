#include <windows.h>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QtCore>

#include "dkdbhelper.h"

static dbstructure dkdbstructure;

void initDbHelper()
{
    dbtable DkGeber("DKGeber");
    DkGeber.Fields.append(dbfield("id", "INTEGER DEFAULT '1' NOT NULL PRIMARY KEY"));
    DkGeber.Fields.append(dbfield("Vorname", "TEXT  NOT NULL"));
    DkGeber.Fields.append(dbfield("Nachname", "TEXT  NOT NULL"));
    DkGeber.Fields.append(dbfield("Strasse", "TEXT  NOT NULL"));
    DkGeber.Fields.append(dbfield("Plz", "TEXT  NOT NULL"));
    DkGeber.Fields.append(dbfield("Stadt", "TEXT  NOT NULL"));
    DkGeber.Fields.append(dbfield("IBAN", "TEXT"));
    DkGeber.Fields.append(dbfield("BIC", "TEXT"));
    dkdbstructure.Tables.append(DkGeber);

    dbtable DkVertrag("DKVertrag");
    DkVertrag.Fields.append((dbfield("id", "INTEGER DEFAULT '0' NOT NULL PRIMARY KEY AUTOINCREMENT")));
    DkVertrag.Fields.append((dbfield("DKGeberId", "INTEGER FOREIGN_KEY REFERENCES [DKGeber](id)")));
    DkVertrag.Fields.append((dbfield("Kennung", "TEXT  NULL")));
    DkVertrag.Fields.append((dbfield("Betrag", "FLOAT DEFAULT '0,0' NOT NULL")));
    DkVertrag.Fields.append((dbfield("Wert", "FLOAT DEFAULT '0,0' NULL")));
    DkVertrag.Fields.append((dbfield("ZSatz", "INTEGER FOREIGN_KEY REFERENCES [DKZinssaetze](id)")));
    DkVertrag.Fields.append((dbfield("tesaurierend", "BOOLEAN DEFAULT '1' NOT NULL")));
    DkVertrag.Fields.append((dbfield("Vertragsdatum", "DATE  NULL")));
    DkVertrag.Fields.append((dbfield("aktiv", "BOOLEAN DEFAULT 'false' NOT NULL")));
    DkVertrag.Fields.append((dbfield("LaufzeitEnde", "DATE DEFAULT '3000-12-31' NOT NULL")));
    DkVertrag.Fields.append((dbfield("LetzteZinsberechnung", "DATE  NULL")));
    dkdbstructure.Tables.append(DkVertrag);

    dbtable Buchung("Buchungen");
    Buchung.Fields.append(((dbfield("id", "INTEGER DEFAULT '0' NOT NULL PRIMARY KEY AUTOINCREMENT"))));
    Buchung.Fields.append(((dbfield("VertragId", "INTEGER FOREIGN_KEY REFERENCES [DKVertrag](id)"))));
    Buchung.Fields.append(((dbfield("Buchungsart", "INTEGER DEFAULT '0' NOT NULL"))));
    Buchung.Fields.append(((dbfield("Betrag", "FLOAT DEFAULT '0' NULL"))));
    Buchung.Fields.append(((dbfield("Datum", "DATE  NULL"))));
    Buchung.Fields.append(((dbfield("Bemerkung", "TEXT  NULL"))));
    dkdbstructure.Tables.append(Buchung);

    dbtable Zinse("DKZinssaetze");
    Zinse.Fields.append(dbfield("id","INTEGER DEFAULT '1' NOT NULL PRIMARY KEY AUTOINCREMENT"));
    Zinse.Fields.append(dbfield("Zinssatz","FLOAT DEFAULT '0,0' UNIQUE NULL"));
    Zinse.Fields.append(dbfield("Bemerkung","TEXT NULL"));
    dkdbstructure.Tables.append(Zinse);

}

bool createTables( const QSqlDatabase& db)
{
    QSqlQuery q(db);
    bool ret(true);
    for(dbtable table :dkdbstructure.Tables)
        ret &= q.exec(table.CreateTableSQL());
    return ret;
}

bool insertInterestRates(const QSqlDatabase& db)
{
    QString sqlZinssaetze ("INSERT INTO DKZinssaetze (Zinssatz, Bemerkung) VALUES ");

    sqlZinssaetze += "(" + QString::number(0.) + ",'Unsere Helden')";
    double Zins = 0.1;
    for (Zins=0.1; Zins < .6; Zins+=0.1)
        sqlZinssaetze += ", (" + QString::number(Zins) + ",'Unsere Freunde')";
    for (; Zins < 1.1; Zins+=0.1)
        sqlZinssaetze += ", (" + QString::number(Zins) + ",'Unsere Foerderer')";
    for (; Zins < 2.; Zins+=0.1)
        sqlZinssaetze += ", (" + QString::number(Zins) + ",'Unsere Investoren')";
    QSqlQuery q(db);
    return q.exec(sqlZinssaetze);
}

bool createDKDB(const QString& filename)
{
    if( QFile(filename).exists())
    {
        QFile::remove(filename +".old");
        if( !QFile::rename(filename, filename +".old"))
            return false;
    }
    bool ret = true;
    dbCloser closer;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filename);
    ret &= db.open();
    if( !ret) return ret;

    closer.set(&db);
    db.transaction();
    ret &= createTables(db);
    ret &= insertInterestRates(db);
    if( ret) db.commit(); else db.rollback();

    return ret;
}

bool hasAllTables(QSqlDatabase& db)
{
    for( auto table : dkdbstructure.Tables)
    {
        QSqlQuery sql(QString("SELECT * FROM ") + table.Name, db);
        if( !sql.exec())
        {
            qDebug() << "testing for table " << table.Name << " failed";
            return false;
        }
    }
    return true;
}

bool isValidDb(const QString& filename)
{
    if( filename == "") return false;
    if( !QFile::exists(filename)) return false;

    dbCloser closer;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "validate");
    db.setDatabaseName(filename);
    if( !db.open())
        return false;
    closer.set(&db);
    if( !hasAllTables(db))
        return false;

    return true;
}

void closeDbConnection()
{
    QList<QString> cl = QSqlDatabase::connectionNames();
    if( cl.count() == 0)
        return;
    if( cl.count() > 1)
    {
        qWarning() << "Found " << cl.count() << "connections open, when there should be 1 or 0";
        return;
    }
    QSqlDatabase::removeDatabase(cl[0]);
    qInfo() << "Database connection " << cl[0] << " removed";
}

void openAppDefaultDb( QString newDbFile)
{
    closeDbConnection();
    QSettings config;
    if( newDbFile == "")
    {
        newDbFile = config.value("db/last").toString();
        qInfo() << "opening DbFile read from configuration: " << newDbFile;
    }
    else
    {
        config.setValue("db/last", newDbFile);
    }

    // setting the default database for the application
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(newDbFile);
    db.open();
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON");
}

void createSampleDkDatabaseData()
{
    QList<QString> Vornamen {"Holger", "Volker", "Peter", "Hans", "Susi", "Roland", "Claudia", "Emil", "Evelyn", "Ötzgür", "Thomas", "Elke", "Berta", "Malte", "Jori"};
    QList<QString> Nachnamen {"Maier", "Müller", "Schmit", "Kramp", "Adams", "Häcker", "Maresch", "Beutl", "Chauchev", "Chen", "Kirk", "Ohura", "Gorbatschov", "Merkel", "Karrenbauer", "Tritin", "Schmidt"};
    QList<QString> Strassen {"Hauptstrasse", "Nebenstrasse", "Bahnhofstrasse", "Kirchstraße", "Dorfstrasse", "Süterlinweg", "Sorbenstrasse", "Kleines Gässchen", "Industriestrasse", "Sesamstrasse", "Lindenstrasse"};
    QList <QPair<QString, QString>> Cities {{"68305", "Mannheim"}, {"69123", "Heidelberg"}, {"69123", "Karlsruhe"}, {"90345", "Hamburg"}};
    QRandomGenerator rand(::GetTickCount());
    for( int i = 0; i<30; i++)
    {
        PersonData p;
        p.Vorname  =  Vornamen [rand.bounded(Vornamen.count ())];
        p.Nachname = Nachnamen[rand.bounded(Nachnamen.count())];
        p.Strasse =  Strassen[rand.bounded(Strassen.count())];
        p.Plz = Cities[rand.bounded(Cities.count())].first;
        p.Stadt = Cities[rand.bounded(Cities.count())].second;
        p.Iban = "iban xxxxxxxxxxxxxxxxx";
        p.Bic = "BICxxxxxxxx";
        int Id = savePersonDataToDb(p);
        // add a contract
        ContractData c;
        c.DKGeberId = Id;
        c.Kennung = "id-" + QString::number(rand.bounded(13));
        c.Zins = rand.bounded(1,19); // cave ! this will fail if the values were deleted from the db
        c.Betrag = float(100) * rand.bounded(1,20);
        c.Wert = c.Betrag;
        c.tesaurierend = rand.bounded(100)%2 ? true : false;
        c.Vertragsdatum = QDate::currentDate().addDays(-1 * rand.bounded(365));
        c.StartZinsberechnung = c.Vertragsdatum.addDays(rand.bounded(15));
        saveContractDataToDb(c);
    }
}

int savePersonDataToDb(const PersonData& p)
{
    QSqlQuery query("", QSqlDatabase::database()); // assuming the app database is open
    QString sql ("INSERT INTO DKGeber (Vorname, Nachname, Strasse, Plz, Stadt, IBAN, BIC)"\
        " VALUES ( :vorn, :nachn, :strasse, :plz, :stadt, :iban, :bic)");
    query.prepare(sql);
    query.bindValue(":vorn", p.Vorname);
    query.bindValue(":nachn", p.Nachname);
    query.bindValue(":strasse", p.Strasse);
    query.bindValue(":plz",  p.Plz);
    query.bindValue(":stadt", p.Stadt);
    query.bindValue(":iban", p.Iban);
    query.bindValue(":bic", p.Bic);
    if( !query.exec())
    {
        qWarning() << "Creating demo data failed\n" << query.lastQuery() << endl << query.lastError().text();
        return false;
    }
    else
    {
        qDebug() << query.lastQuery() << "executed successfully\n" << sql;
        return query.lastInsertId().toInt();
    }
}

void AllPersonsForSelection(QList<PersonDispStringWithId>& persons)
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("SELECT id, Vorname, Nachname, Plz, Strasse FROM DKGeber ORDER BY Nachname ASC, Vorname ASC");
    if( !query.exec())
    {
        qCritical() << "Error reading DKGeber while creating a contract: " << QSqlDatabase::database().lastError().text();
    }

    while(query.next())
    {
        QString Entry = query.value("Nachname").toString() + QString(", ") + QString(", ") + query.value("Vorname").toString() + QString(", ") + query.value("Plz").toString();
        Entry += QString(", ") + query.value("Strasse").toString();
        PersonDispStringWithId entry{ query.value("id").toInt(), Entry};
        persons.append(entry);
    }
}

void AllInterestRatesForSelection(QList<ZinsDispStringWithId>& Rates)
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("SELECT id, Zinssatz, Bemerkung FROM DKZinssaetze ORDER BY Zinssatz DESC");
    if( !query.exec())
    {
        qCritical() << "Error reading Interrest Rates while creating a contract: " << QSqlDatabase::database().lastError().text();
    }
    while(query.next())
    {
        ZinsDispStringWithId entry{ query.value("id").toInt(), (query.value("Zinssatz").toString() + "  (" + query.value("Bemerkung").toString() + ")  ")};
        Rates.append(entry);
    }
}

ContractData::ContractData() :
    DKGeberId(-1),
    Betrag(0.), Wert(0.), Zins(0.),
    tesaurierend(true), active(true),
    Vertragsdatum(QDate::currentDate()),
    LaufzeitEnde(QDate(9999, 12, 31)),
    StartZinsberechnung(QDate::currentDate())
{

}

bool saveContractDataToDb(const ContractData& c)
{
    QSqlQuery query("", QSqlDatabase::database());
    QString sql ("INSERT INTO DKVertrag (DKGeberId, Kennung, Betrag, Wert, ZSatz, tesaurierend, Vertragsdatum, aktiv, LaufzeitEnde, LetzteZinsberechnung)");
    sql += " VALUES (:dkgid, :kennung, :betrag, :wert, :zsatz, :tes, :vdatum, :akt, :lzende, :letzt )";
    query.prepare(sql);
    query.bindValue(":dkgid", c.DKGeberId);
    query.bindValue(":kennung", c.Kennung);
    query.bindValue(":betrag", c.Betrag); // zweistellig
    query.bindValue(":wert", c.Wert); // zweistellig
    query.bindValue(":zsatz", c.Zins);// ID !!
    query.bindValue(":tes", c.tesaurierend? "true" : "false");
    query.bindValue(":vdatum", c.Vertragsdatum.toString(Qt::ISODate));
    query.bindValue(":akt", c.active ? "true": "false");
    query.bindValue(":lzende", c.LaufzeitEnde.toString(Qt::ISODate));
    query.bindValue(":letzt", c.StartZinsberechnung.toString(Qt::ISODate));
    if( !query.exec())
    {
        qWarning() << "Creating contract db entry failed\n" << query.lastQuery() << endl << query.lastError().text();
        return false;
    }
    else
    {
        qDebug() << query.lastQuery() << "executed successfully\n" << sql;
        return true;
    }
}
