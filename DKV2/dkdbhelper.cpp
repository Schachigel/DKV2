#include <windows.h>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QtCore>

#include "filehelper.h"
#include "dkdbhelper.h"

dbstructure dkdbstructure;

void initDbHelper()
{
    // DB date -> Variant String
    // DB bool -> Variant int
    dbtable Kreditoren("Kreditoren");
    Kreditoren.Fields.append(dbfield(Kreditoren, "id",       QVariant::Int,    "'0' NOT NULL PRIMARY KEY AUTOINCREMENT"));
    Kreditoren.Fields.append(dbfield(Kreditoren, "Vorname",  QVariant::String, "NOT NULL"));
    Kreditoren.Fields.append(dbfield(Kreditoren, "Nachname", QVariant::String, "NOT NULL"));
    Kreditoren.Fields.append(dbfield(Kreditoren, "Strasse",  QVariant::String, "NOT NULL"));
    Kreditoren.Fields.append(dbfield(Kreditoren, "Plz",      QVariant::String, "NOT NULL"));
    Kreditoren.Fields.append(dbfield(Kreditoren, "Stadt",    QVariant::String, "NOT NULL"));
    Kreditoren.Fields.append(dbfield(Kreditoren, "IBAN",     QVariant::String));
    Kreditoren.Fields.append(dbfield(Kreditoren, "BIC",      QVariant::String));
    dkdbstructure.Tables.append(Kreditoren);

    dbtable Zinsen("Zinssaetze");
    Zinsen.Fields.append(dbfield(Zinsen, "id",       QVariant::Int,    "DEFAULT '0' NOT NULL PRIMARY KEY AUTOINCREMENT"));
    Zinsen.Fields.append(dbfield(Zinsen, "Zinssatz", QVariant::Double, "DEFAULT '0,0' UNIQUE NULL"));
    Zinsen.Fields.append(dbfield(Zinsen, "Bemerkung"));
    dkdbstructure.Tables.append(Zinsen);

    dbtable Vertraege("Vertraege");
    Vertraege.Fields.append((dbfield(Vertraege, "id",         QVariant::Int, "DEFAULT '0' NOT NULL PRIMARY KEY AUTOINCREMENT")));
    Vertraege.Fields.append((dbfield(Vertraege, "KreditorId", QVariant::Int, "FOREIGN_KEY REFERENCES [Kreditoren](id) ON DELETE CASCADE")));
    Vertraege.Fields.append((dbfield(Vertraege, "Kennung")));
    Vertraege.Fields.append((dbfield(Vertraege, "Betrag",     QVariant::Double, "DEFAULT '0,0' NOT NULL")));
    Vertraege.Fields.append((dbfield(Vertraege, "Wert",       QVariant::Double, "DEFAULT '0,0' NULL")));
    Vertraege.Fields.append((dbfield(Vertraege, "ZSatz",      QVariant::Int, "FOREIGN_KEY REFERENCES [Zinssaetze](id)")));
    Vertraege.Fields.append((dbfield(Vertraege, "tesaurierend",  QVariant::Bool, "DEFAULT '1' NOT NULL")));
    Vertraege.Fields.append((dbfield(Vertraege, "Vertragsdatum", QVariant::Date, "DATE  NULL")));
    Vertraege.Fields.append((dbfield(Vertraege, "aktiv",         QVariant::Bool, "DEFAULT '0' NOT NULL")));
    Vertraege.Fields.append((dbfield(Vertraege, "LaufzeitEnde",  QVariant::Date, "DEFAULT '3000-12-31' NOT NULL")));
    Vertraege.Fields.append((dbfield(Vertraege, "LetzteZinsberechnung", QVariant::Date, "NULL")));
    dkdbstructure.Tables.append(Vertraege);

    dbtable Buchungsarten("Buchungsarten");
    Buchungsarten.Fields.append(dbfield(Buchungsarten, "id",  QVariant::Int, "DEFAULT '0' NOT NULL PRIMARY KEY AUTOINCREMENT"));
    Buchungsarten.Fields.append(dbfield(Buchungsarten, "Art", QVariant::String, "NOT NULL"));
    dkdbstructure.Tables.append(Buchungsarten);

    dbtable Buchungen("Buchungen");
    Buchungen.Fields.append(((dbfield(Buchungen, "id",           QVariant::Int, "DEFAULT '0' NOT NULL PRIMARY KEY AUTOINCREMENT"))));
    Buchungen.Fields.append(((dbfield(Buchungen, "VertragId",    QVariant::Int, "FOREIGN_KEY REFERENCES [Vertraege](id) ON DELETE SET NULL"))));
    Buchungen.Fields.append(((dbfield(Buchungen, "Buchungsart",  QVariant::Int, "FOREIGN_KEY REFERENCES [Buchungsarten](id)"))));
    Buchungen.Fields.append(((dbfield(Buchungen, "Betrag",       QVariant::Double, "DEFAULT '0' NULL"))));
    Buchungen.Fields.append(((dbfield(Buchungen, "Datum",        QVariant::Date))));
    Buchungen.Fields.append(((dbfield(Buchungen, "Bemerkung",    QVariant::String))));
    dkdbstructure.Tables.append(Buchungen);

    dbtable meta("Meta");
    meta.Fields.append(dbfield(meta, "Name", QVariant::String, "NOT NULL"));
    meta.Fields.append(dbfield(meta, "Wert", QVariant::String, "NOT NULL"));
}

QString dbTypeFromVariant(QVariant::Type t)
{
    switch( t)
    {
    case QVariant::String:
        return "STRING";
    case QVariant::Int:
        return "INTEGER";
    case QVariant::Double:
        return "REAL";
    case QVariant::Date:
        return "STRING"; // sadly ...
    case QVariant::Bool:
        return "INTEGER";
    default:
        Q_ASSERT(!bool("invalid database type"));
    }
}

bool createTables( const QSqlDatabase& db)
{
    QSqlQuery q(db);
    bool ret{true};
    for(dbtable table :dkdbstructure.Tables)
    {
        QString tableSql(table.CreateTableSQL());
        ret = q.exec(tableSql);
        if(!ret)
            qCritical() << "table.CreateTableSQL() failed: " << q.lastError() << "\n" << tableSql;
        else
            qDebug() << "New Database table creation. Tablename:" << table.Name  << "\n" << tableSql;
    }
    return ret;
}

bool insertInterestRates(const QSqlDatabase& db)
{
    QString sqlZinssaetze ("INSERT INTO Zinssaetze (Zinssatz, Bemerkung) VALUES ");
    sqlZinssaetze += "(" + QString::number(0.) + ",'Unser Held')";
    double Zins = 0.1;
    for (Zins=0.1; Zins < .6; Zins+=0.1)
        sqlZinssaetze += ", (" + QString::number(Zins) + ",'Unser Freund')";
    for (; Zins < 1.1; Zins+=0.1)
        sqlZinssaetze += ", (" + QString::number(Zins) + ",'Unser Foerderer')";
    for (; Zins < 2.; Zins+=0.1)
        sqlZinssaetze += ", (" + QString::number(Zins) + ",'Unser Investor')";
    QSqlQuery q(db);
    return q.exec(sqlZinssaetze);
}

bool insertBuchungsarten(const QSqlDatabase& db)
{
    QSqlQuery sql(db);
    sql.prepare("INSERT INTO Buchungsarten (Art) VALUES (:art)");

    QStringList arten{"Vertrag anlegen", "Vertrag aktivieren", "Passiven Vertrag löschen", "Vertrag beenden"};
    for( auto art: arten)
    {
        sql.bindValue(":art", art);
        if( !sql.exec())
        {
            qCritical() << "Anlegen von Buchungsarten fehlgeschlagen\n" << sql.lastQuery() << "\n"<< sql.lastError();
            return false;
        }
    }
    return true;
}

bool insertProperties(const QSqlDatabase& db)
{
    QSqlQuery sql("INSERT INTO Meta (Name, Wert) VALUES (\"Version\", \"1.0\"", db);
    return true;
}

bool createDKDB(const QString& filename)
{
    closeDbConnection();
    if( QFile(filename).exists())
    {
        backupFile(filename);
    }
    dbCloser closer;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filename);

    if( !db.open()) return false;
    bool ret = true;
    closer.set(&db);
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON");
    db.transaction();
    ret &= createTables(db);
    ret &= insertInterestRates(db);
    ret &= insertBuchungsarten(db);
    ret &= insertProperties(db);
    if( ret) db.commit(); else db.rollback();

    if (isValidDb(filename))
        return ret;
    else
    {
        qCritical() << "Newly created db is invalid. We should panic";
        return false;
    }
}

bool hasAllTables(QSqlDatabase& db)
{
    for( auto table : dkdbstructure.Tables)
    {
        QSqlQuery sql(db);
        sql.prepare(QString("SELECT * FROM ") + table.Name);
        if( !sql.exec())
        {
            qDebug() << "testing for table " << table.Name << " failed\n" << sql.lastError() << "\n" << sql.lastQuery();
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
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON");
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
        config.setValue("db/last", newDbFile);
    backupFile(newDbFile);

    // setting the default database for the application
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(newDbFile);
    db.open();
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON");
}

void createSampleDkDatabaseData()
{
    QList<QString> Vornamen {"Holger", "Volker", "Peter", "Hans", "Susi", "Roland", "Claudia", "Emil", "Evelyn", "Ötzgür", "Thomas", "Elke", "Berta", "Malte", "Jori", "Paul", "Jonas", "Finn", "Leon", "Luca", "Emma", "Mia", "Lena", "Anna"};
    QList<QString> Nachnamen {"Maier", "Müller", "Schmit", "Kramp", "Adams", "Häcker", "Maresch", "Beutl", "Chauchev", "Chen", "Kirk", "Ohura", "Gorbatschov", "Merkel", "Karrenbauer", "Tritin", "Schmidt", "Rao", "Lassen", "Hurgedü"};
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
        int Id =0;
        if( 0 > (Id = savePersonDataToDb(p)))
        {
            qCritical() << "No id from savePersonenDataToDb";
        }
        // add a contract
        VertragsDaten c;
        c.KreditorId = Id;
        c.Kennung = "id-" + QString::number(rand.bounded(13));
        c.Zins = rand.bounded(1,19); // cave ! this will fail if the values were deleted from the db
        c.Betrag = float(100) * rand.bounded(1,20);
        c.Wert = c.Betrag;
        c.tesaurierend = rand.bounded(100)%2 ? true : false;
        c.Vertragsdatum = QDate::currentDate().addDays(-1 * rand.bounded(365));
        c.active = 0 != rand.bounded(3)%3; // random data, more true then false
        c.StartZinsberechnung = c.active ? c.Vertragsdatum.addDays(rand.bounded(15)) : QDate();
        verbucheVertrag(c);
    }
}

int savePersonDataToDb(const PersonData& p)
{
    QSqlQuery query; // assuming the app database is open
    QString sql ("INSERT INTO Kreditoren (Vorname, Nachname, Strasse, Plz, Stadt, IBAN, BIC)"\
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
        qWarning() << "Kreditor Daten konnten nicht gespeichert werden\n" << query.lastQuery() << endl << query.lastError().text();
        return -1;
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
    query.prepare("SELECT id, Vorname, Nachname, Plz, Strasse FROM Kreditoren ORDER BY Nachname ASC, Vorname ASC");
    if( !query.exec())
    {
        qCritical() << "Error reading DKGeber while creating a contract: " << QSqlDatabase::database().lastError().text();
    }

    while(query.next())
    {
        QString Entry = query.value("Nachname").toString() + QString(", ") + query.value("Vorname").toString() + QString(", ") + query.value("Plz").toString();
        Entry += QString(", ") + query.value("Strasse").toString();
        PersonDispStringWithId entry{ query.value("id").toInt(), Entry};
        persons.append(entry);
    }
}

void AllInterestRatesForSelection(QList<ZinsDispStringWithId>& Rates)
{
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("SELECT id, Zinssatz, Bemerkung FROM Zinssaetze ORDER BY Zinssatz DESC");
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

VertragsDaten::VertragsDaten() :
    KreditorId(-1),
    Betrag(0.), Wert(0.), Zins(0.),
    tesaurierend(true), active(true),
    Vertragsdatum(QDate::currentDate()),
    LaufzeitEnde(QDate(9999, 12, 31)),
    StartZinsberechnung(QDate::currentDate())
{

}

int speichereVertrag(const VertragsDaten& c)
{
    QSqlQuery VertragEinfuegen;
    QString sqlVertragEinfuegen ("INSERT INTO Vertraege (KreditorId, Kennung, Betrag, Wert, ZSatz, tesaurierend, Vertragsdatum, aktiv, LaufzeitEnde, LetzteZinsberechnung)");
    sqlVertragEinfuegen += " VALUES (:dkgid, :kennung, :betrag, :wert, :zsatz, :tes, :vdatum, :akt, :lzende, :letzt )";
    VertragEinfuegen.prepare(sqlVertragEinfuegen);
    VertragEinfuegen.bindValue(":dkgid", c.KreditorId);
    VertragEinfuegen.bindValue(":kennung", c.Kennung);
    VertragEinfuegen.bindValue(":betrag", c.Betrag); // zweistellig
    VertragEinfuegen.bindValue(":wert", c.Wert); // zweistellig
    VertragEinfuegen.bindValue(":zsatz", c.Zins);// ID !!
    VertragEinfuegen.bindValue(":tes", c.tesaurierend? "true" : "false");
    VertragEinfuegen.bindValue(":vdatum", c.Vertragsdatum.toString(Qt::ISODate));
    VertragEinfuegen.bindValue(":akt", c.active ? QVariant(true): QVariant(false));
    VertragEinfuegen.bindValue(":lzende", c.LaufzeitEnde.toString(Qt::ISODate));
    VertragEinfuegen.bindValue(":letzt", c.StartZinsberechnung.toString(Qt::ISODate));
    if( VertragEinfuegen.exec())
    {
        int lastid = VertragEinfuegen.lastInsertId().toInt();
        qDebug() << "Neuer Vertrag wurde eingefügt mit id:" << lastid;
        return lastid;
    }
    qCritical() << "Neuer Vertrag wurde nicht gespeichert" << VertragEinfuegen.lastError();
    return -1;
}

int BuchungsartIdFromArt(QString s)
{
    QSqlQuery query;
    query.exec("SELECT * FROM Buchungsarten WHERE Art =\"" + s + "\"");
    query.next();

    int i = query.value(0).toInt();
    return i;
}

bool speichereBeleg_neuerVertrag(const int VertragId, const VertragsDaten& c)
{
    QSqlRecord r;
    r.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("Betrag"));
    r.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("Wert"));
    r.append(dkdbstructure.getTable("Zinssaetze").getQSqlFieldByName("Zinssatz"));
    r.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("tesaurierend"));
    r.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("Vertragsdatum"));
    r.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("aktiv"));
    r.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("LetzteZinsberechnung"));
    r.append(dkdbstructure.getTable("Kreditoren").getQSqlFieldByName("Vorname"));
    r.append(dkdbstructure.getTable("Kreditoren").getQSqlFieldByName("Nachname"));
    r.append(dkdbstructure.getTable("Kreditoren").getQSqlFieldByName("Strasse"));
    r.append(dkdbstructure.getTable("Kreditoren").getQSqlFieldByName("Plz"));
    r.append(dkdbstructure.getTable("Kreditoren").getQSqlFieldByName("Stadt"));
    r.append(dkdbstructure.getTable("Kreditoren").getQSqlFieldByName("IBAN"));
    r.append(dkdbstructure.getTable("Kreditoren").getQSqlFieldByName("BIC"));

    r.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("id"));
    r.append(dkdbstructure.getTable("Kreditoren").getQSqlFieldByName("id"));
    r.append(dkdbstructure.getTable("Zinssaetze").getQSqlFieldByName("id"));
    r.append(dkdbstructure.getTable("Vertraege").getQSqlFieldByName("ZSatz"));

    QString sql("SELECT ");
    for( int i =0; i<r.count(); i++)
    {
        if( i) sql += ", ";
        sql += r.field(i).tableName() + "." + r.field(i).name();
    }
    sql += " FROM Vertraege, Kreditoren, Zinssaetze "
           "WHERE Vertraege.id = " + QString::number(VertragId) +
           " AND Kreditoren.id = Vertraege.KreditorId AND Vertraege.ZSatz = Zinssaetze.id ";

    QSqlQuery all(sql); all.first();
    QString Buchungstext;
    for( int i =0; i<all.record().count(); i++)
    {
        Buchungstext += all.record().field(i).tableName()+ "." + all.record().fieldName(i) + ":\"" + all.record().field(i).value().toString() + "\";";
    }
    qDebug() << "Speichere Buchungsdaten:\n" << Buchungstext;
    QSqlQuery sqlBuchung;
    sqlBuchung.prepare("INSERT INTO Buchungen (VertragId, Buchungsart, Betrag, Datum, Bemerkung)"
                       " VALUES (:VertragsId, :Buchungsart, :Betrag, :Datum, :Bemerkung)");
    sqlBuchung.bindValue(":VertragsId", QVariant(VertragId));
    sqlBuchung.bindValue(":Buchungsart", QVariant(BuchungsartIdFromArt("Vertrag anlegen")));
    sqlBuchung.bindValue(":Betrag", QVariant(c.Betrag));
    sqlBuchung.bindValue(":Datum", QVariant(QDate::currentDate()));
    sqlBuchung.bindValue(":Bemerkung", QVariant(Buchungstext));
    if( !sqlBuchung.exec())
    {
        qDebug() << "Buchung kann nicht gespeichert werden.\n" << sqlBuchung.lastError();
        return false;
    }
    return true;
}

bool verbucheVertrag(const VertragsDaten& c)
{
    QSqlDatabase::database().transaction();
    int vid = speichereVertrag(c);
    if( vid >0 )
        if( speichereBeleg_neuerVertrag(vid, c))
        {
            QSqlDatabase::database().commit();
            return true;
        }
    qCritical() << "ein neuer Vertrag konnte nicht gespeichert werden";
    QSqlDatabase::database().rollback();
    return false;
}

bool activateContract( int ContractId, QDate activationDate)
{
    QSqlQuery updateQ;
    updateQ.prepare("UPDATE Vertraege SET LetzteZinsberechnung = :vdate, aktiv = :true WHERE id = :id");
    updateQ.bindValue(":vdate",QVariant(activationDate));
    updateQ.bindValue(":id", QVariant(ContractId));
    updateQ.bindValue(":true", QVariant(true));
    bool ret = updateQ.exec();
    qDebug() << updateQ.lastQuery() << updateQ.lastError();
    return ret;
}
