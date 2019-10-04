#include <windows.h>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QtCore>

#include "helper.h"
#include "filehelper.h"
#include "dkdbhelper.h"
#include "dbstructure.h"

dbstructure dkdbstructur;

void initDKDBStruktur()
{
    LOG_ENTRY_and_EXIT;
    // DB date -> Variant String
    // DB bool -> Variant int
    dbtable Kreditoren("Kreditoren");
    Kreditoren.fields.append(dbfield(Kreditoren.name, "id",       QVariant::Int,    "PRIMARY KEY AUTOINCREMENT"));
    Kreditoren.fields.append(dbfield(Kreditoren.name, "Vorname",  QVariant::String, "NOT NULL"));
    Kreditoren.fields.append(dbfield(Kreditoren.name, "Nachname", QVariant::String, "NOT NULL"));
    Kreditoren.fields.append(dbfield(Kreditoren.name, "Strasse",  QVariant::String, "NOT NULL"));
    Kreditoren.fields.append(dbfield(Kreditoren.name, "Plz",      QVariant::String, "NOT NULL"));
    Kreditoren.fields.append(dbfield(Kreditoren.name, "Stadt",    QVariant::String, "NOT NULL"));
    Kreditoren.fields.append(dbfield(Kreditoren.name, "IBAN",     QVariant::String));
    Kreditoren.fields.append(dbfield(Kreditoren.name, "BIC",      QVariant::String));
    dkdbstructur.addTable(Kreditoren);

    dbtable Zinssaetze("Zinssaetze");
    Zinssaetze.fields.append(dbfield(Zinssaetze.name, "id",       QVariant::Int,    "PRIMARY KEY AUTOINCREMENT"));
    Zinssaetze.fields.append(dbfield(Zinssaetze.name, "Zinssatz", QVariant::Double, "DEFAULT '0,0' UNIQUE NULL"));
    Zinssaetze.fields.append(dbfield(Zinssaetze.name, "Bemerkung"));
    dkdbstructur.addTable(Zinssaetze);

    dbtable Vertraege("Vertraege");
    Vertraege.fields.append((dbfield(Vertraege.name, "id",         QVariant::Int, "PRIMARY KEY AUTOINCREMENT")));
    Vertraege.fields.append((dbfield(Vertraege.name, "KreditorId", QVariant::Int, "", Kreditoren["id"].getInfo(), dbfield::refIntOption::onDeleteCascade )));
    Vertraege.fields.append((dbfield(Vertraege.name, "Kennung")));
    Vertraege.fields.append((dbfield(Vertraege.name, "Betrag",     QVariant::Double, "DEFAULT '0,0' NOT NULL")));
    Vertraege.fields.append((dbfield(Vertraege.name, "Wert",       QVariant::Double, "DEFAULT '0,0' NULL")));
    Vertraege.fields.append((dbfield(Vertraege.name, "ZSatz",      QVariant::Int, "", Zinssaetze["id"].getInfo(), dbfield::refIntOption::non)));
    Vertraege.fields.append((dbfield(Vertraege.name, "tesaurierend",  QVariant::Bool, "DEFAULT '1' NOT NULL")));
    Vertraege.fields.append((dbfield(Vertraege.name, "Vertragsdatum", QVariant::Date, "DATE  NULL")));
    Vertraege.fields.append((dbfield(Vertraege.name, "aktiv",         QVariant::Bool, "DEFAULT '0' NOT NULL")));
    Vertraege.fields.append((dbfield(Vertraege.name, "LaufzeitEnde",  QVariant::Date, "DEFAULT '3000-12-31' NOT NULL")));
    Vertraege.fields.append((dbfield(Vertraege.name, "LetzteZinsberechnung", QVariant::Date, "NULL")));
    dkdbstructur.addTable(Vertraege);

    dbtable Buchungsarten("Buchungsarten");
    Buchungsarten.fields.append(dbfield(Buchungsarten.name, "id",  QVariant::Int, "PRIMARY KEY AUTOINCREMENT"));
    Buchungsarten.fields.append(dbfield(Buchungsarten.name, "Art", QVariant::String, "NOT NULL"));
    dkdbstructur.addTable(Buchungsarten);

    dbtable Buchungen("Buchungen");
    Buchungen.fields.append(((dbfield(Buchungen.name, "id",           QVariant::Int, "PRIMARY KEY AUTOINCREMENT"))));
    Buchungen.fields.append(((dbfield(Buchungen.name, "VertragId",    QVariant::Int, "", Vertraege["id"].getInfo(), dbfield::refIntOption::onDeleteNull))));
    Buchungen.fields.append(((dbfield(Buchungen.name, "Buchungsart",  QVariant::Int, "", Buchungsarten["id"].getInfo(), dbfield::refIntOption::non))));
    Buchungen.fields.append(((dbfield(Buchungen.name, "Betrag",       QVariant::Double, "DEFAULT '0' NULL"))));
    Buchungen.fields.append(((dbfield(Buchungen.name, "Datum",        QVariant::Date))));
    Buchungen.fields.append(((dbfield(Buchungen.name, "Bemerkung",    QVariant::String))));
    dkdbstructur.addTable(Buchungen);

    dbtable meta("Meta");
    meta.fields.append(dbfield(meta.name, "Name", QVariant::String, "NOT NULL"));
    meta.fields.append(dbfield(meta.name, "Wert", QVariant::String, "NOT NULL"));
    dkdbstructur.addTable(meta);
}

bool DKDB_TabellenAnlegen( const QSqlDatabase& db)
{   LOG_ENTRY_and_EXIT;

    QSqlQuery q(db);
    bool ret{true};
    for(dbtable table :dkdbstructur.getTables())
    {
        QString tableSql(table.CreateTableSQL());
        ret = q.exec(tableSql);
        if(!ret)
            qCritical() << "table.CreateTableSQL() failed: " << q.lastError() << "\n" << tableSql;
        else
            qDebug() << "New Database table creation. Tablename:" << table.name  << "\n" << tableSql;
    }
    return ret;
}

bool ZinssaetzeEinfuegen(const QSqlDatabase& db)
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

bool BuchungsartenEinfuegen(const QSqlDatabase& db)
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

bool EigenschaftenEinfuegen(const QSqlDatabase& db)
{
    QSqlQuery sql("INSERT INTO Meta (Name, Wert) VALUES (\"Version\", \"1.0\"", db);
    return true;
}

bool DKDatenbankAnlegen(const QString& filename)
{    LOG_ENTRY_and_EXIT;

    DatenbankverbindungSchliessen();
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
    ret &= DKDB_TabellenAnlegen(db);
    ret &= ZinssaetzeEinfuegen(db);
    ret &= BuchungsartenEinfuegen(db);
    ret &= EigenschaftenEinfuegen(db);
    if( ret) db.commit(); else db.rollback();

    if (istValideDatenbank(filename))
        return ret;
    else
    {
        qCritical() << "Newly created db is invalid. We should panic";
        return false;
    }
}

bool hatAlleTabellen(QSqlDatabase& db)
{   LOG_ENTRY_and_EXIT;

    for( auto table : dkdbstructur.getTables())
    {
        QSqlQuery sql(db);
        sql.prepare(QString("SELECT * FROM ") + table.name);
        if( !sql.exec())
        {
            qDebug() << "testing for table " << table.name << " failed\n" << sql.lastError() << "\n" << sql.lastQuery();
            return false;
        }
    }
    return true;
}

bool istValideDatenbank(const QString& filename)
{   LOG_ENTRY_and_EXIT;

    if( filename == "") return false;
    if( !QFile::exists(filename)) return false;

    dbCloser closer;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "validate");
    db.setDatabaseName(filename);
    if( !db.open())
        return false;
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON");
    closer.set(&db);
    if( !hatAlleTabellen(db))
        return false;

    return true;
}

void DatenbankverbindungSchliessen()
{   LOG_ENTRY_and_EXIT;

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

void DatenbankZurAnwendungOeffnen( QString newDbFile)
{   LOG_ENTRY_and_EXIT;

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
}

void BeispieldatenAnlegen( int AnzahlDatensaetze)
{   LOG_ENTRY_and_EXIT;

    QList<QString> Vornamen {"Holger", "Volker", "Peter", "Hans", "Susi", "Roland", "Claudia", "Emil", "Evelyn", "Ötzgür", "Thomas", "Elke", "Berta", "Malte", "Jori", "Paul", "Jonas", "Finn", "Leon", "Luca", "Emma", "Mia", "Lena", "Anna"};
    QList<QString> Nachnamen {"Maier", "Müller", "Schmit", "Kramp", "Adams", "Häcker", "Maresch", "Beutl", "Chauchev", "Chen", "Kirk", "Ohura", "Gorbatschov", "Merkel", "Karrenbauer", "Tritin", "Schmidt", "Rao", "Lassen", "Hurgedü"};
    QList<QString> Strassen {"Hauptstrasse", "Nebenstrasse", "Bahnhofstrasse", "Kirchstraße", "Dorfstrasse", "Süterlinweg", "Sorbenstrasse", "Kleines Gässchen", "Industriestrasse", "Sesamstrasse", "Lindenstrasse"};
    QList <QPair<QString, QString>> Cities {{"68305", "Mannheim"}, {"69123", "Heidelberg"}, {"69123", "Karlsruhe"}, {"90345", "Hamburg"}};
    QRandomGenerator rand(::GetTickCount());
    for( int i = 0; i<AnzahlDatensaetze; i++)
    {
        KreditorDaten p{ Vornamen [rand.bounded(Vornamen.count ())], Nachnamen[rand.bounded(Nachnamen.count())],
        Strassen[rand.bounded(Strassen.count())], Cities[rand.bounded(Cities.count())].first, Cities[rand.bounded(Cities.count())].second,
        "iban xxxxxxxxxxxxxxxxx", "BICxxxxxxxx"};
        int neueKreditorId =0;
        if( 0 > (neueKreditorId = KreditorDatenSpeichern(p)))
        {
            qCritical() << "No id from KreditorDatenSpeichern";
            Q_ASSERT(!bool("Verbuchung des neuen Vertrags gescheitert"));
        }
        // add a contract
        VertragsDaten c;
        c.KreditorId = neueKreditorId;
        c.Kennung = "id-" + QString::number(rand.bounded(13));
        c.Zins = rand.bounded(1,19); // cave ! this will fail if the values were deleted from the db
        c.Betrag = double(100) * rand.bounded(1,20);
        c.Wert = c.Betrag;
        c.tesaurierend = rand.bounded(100)%2 ? true : false;
        c.Vertragsdatum = QDate::currentDate().addDays(-1 * rand.bounded(365));
        c.active = 0 != rand.bounded(3)%3; // random data, more true then false
        if( c.active) c.StartZinsberechnung =c.Vertragsdatum.addDays(rand.bounded(15));
        VertragVerbuchen(c);
    }
}

int KreditorDatenSpeichern(const KreditorDaten& p)
{   LOG_ENTRY_and_EXIT;

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

bool KreditorLoeschen(QString index)
{   LOG_ENTRY_and_EXIT;

    // referential integrity will delete the contracts
    QSqlQuery deleteQ;
    if( !deleteQ.exec("DELETE FROM [Kreditoren] WHERE [Id]=" +index))
    {
        qCritical() << "Delete Kreditor failed "<< deleteQ.lastError() << "\n" << deleteQ.lastQuery();
        return false;
    }
    else
        return true;
}

void KreditorenFuerAuswahlliste(QList<KreditorAnzeigeMitId>& persons)
{   LOG_ENTRY_and_EXIT;

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
        KreditorAnzeigeMitId entry{ query.value("id").toInt(), Entry};
        persons.append(entry);
    }
}

void ZinssaetzeFuerAuswahlliste(QList<ZinsAnzeigeMitId>& Rates)
{   LOG_ENTRY_and_EXIT;

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

int speichereVertrag(const VertragsDaten& c)
{   LOG_ENTRY_and_EXIT;

    QSqlQuery VertragEinfuegen;
    QString sqlVertragEinfuegen ("INSERT INTO Vertraege (KreditorId, Kennung, Betrag, Wert, ZSatz, tesaurierend, Vertragsdatum, aktiv, LaufzeitEnde, LetzteZinsberechnung)");
    sqlVertragEinfuegen += " VALUES (\":dkgid\", \":kennung\", \":betrag\", \":wert\", \":zsatz\", \":tes\", \":vdatum\", \":akt\", \":lzende\", \":letzt\" )";
    sqlVertragEinfuegen.replace(QString(":dkgid"), QString::number(c.KreditorId));
    sqlVertragEinfuegen.replace(":kennung", c.Kennung);
    sqlVertragEinfuegen.replace(":betrag", QString::number(c.Betrag, 'f', 2)); // zweistellig
    sqlVertragEinfuegen.replace(":wert", QString::number(c.Wert, 'f', 2)); // zweistellig
    sqlVertragEinfuegen.replace(":zsatz", QString::number(c.Zins, 'f', 3));
    sqlVertragEinfuegen.replace(":tes", c.tesaurierend? QString::number(1) : QString::number(0));
    sqlVertragEinfuegen.replace(":vdatum", c.Vertragsdatum.toString(Qt::ISODate));
    sqlVertragEinfuegen.replace(":akt", c.active ? QString::number(1) : QString::number(0));
    sqlVertragEinfuegen.replace(":lzende", c.LaufzeitEnde.toString(Qt::ISODate));
    sqlVertragEinfuegen.replace(":letzt", c.StartZinsberechnung.toString(Qt::ISODate));

    VertragEinfuegen.prepare(sqlVertragEinfuegen);
    if( VertragEinfuegen.exec())
    {
        int lastid = VertragEinfuegen.lastInsertId().toInt();
        qDebug() << "Neuer Vertrag wurde eingefügt mit id:" << lastid;
        return lastid;
    }
    qCritical() << "Neuer Vertrag wurde nicht gespeichert: " << VertragEinfuegen.lastError() << "\n" << VertragEinfuegen.lastQuery();
    return -1;
}

int BuchungsartIdFromArt(QString s)
{
    QSqlQuery query("SELECT * FROM Buchungsarten WHERE Art =\"" + s + "\"");
    query.next();
    int i = query.value(0).toInt();
    return i;
}

bool BelegZuNeuemVertragSpeichern(const int VertragId, const VertragsDaten& c)
{   LOG_ENTRY_and_EXIT;

    QVector<dbfield> fields;
    fields.append(dkdbstructur["Vertraege"]["Betrag"]);
    fields.append(dkdbstructur["Vertraege"]["Wert"]);
    fields.append(dkdbstructur["Zinssaetze"]["Zinssatz"]);
    fields.append(dkdbstructur["Vertraege"]["tesaurierend"]);
    fields.append(dkdbstructur["Vertraege"]["Vertragsdatum"]);
    fields.append(dkdbstructur["Vertraege"]["aktiv"]);
    fields.append(dkdbstructur["Vertraege" ]["LetzteZinsberechnung"]);
    fields.append(dkdbstructur["Kreditoren"]["Vorname"]);
    fields.append(dkdbstructur["Kreditoren"]["Nachname"]);
    fields.append(dkdbstructur["Kreditoren"]["Strasse"]);
    fields.append(dkdbstructur["Kreditoren"]["Plz"]);
    fields.append(dkdbstructur["Kreditoren"]["Stadt"]);
    fields.append(dkdbstructur["Kreditoren"]["IBAN"]);
    fields.append(dkdbstructur["Kreditoren"]["BIC"]);

    fields.append(dkdbstructur["Vertraege" ]["id"]);
    fields.append(dkdbstructur["Kreditoren"]["id"]);
    fields.append(dkdbstructur["Zinssaetze"]["id"]);
    fields.append(dkdbstructur["Vertraege" ]["ZSatz"]);

    QString sql("SELECT ");
    for( int i =0; i<fields.size(); i++)
    {
        if( i) sql += ", ";
        sql += fields[i].tablename + "." + fields[i].name;
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
        qCritical() << "Buchung kann nicht gespeichert werden.\n" << sqlBuchung.lastError();
        return false;
    }
    return true;
}

bool VertragVerbuchen(const VertragsDaten& c)
{   LOG_ENTRY_and_EXIT;

    QSqlDatabase::database().transaction();
    int vid = speichereVertrag(c);
    if( vid >0 )
        if( BelegZuNeuemVertragSpeichern(vid, c))
        {
            QSqlDatabase::database().commit();
            return true;
        }

    qCritical() << "ein neuer Vertrag konnte nicht gespeichert werden";
    QSqlDatabase::database().rollback();
    return false;
}

bool VertragAktivieren( int ContractId, QDate activationDate)
{   LOG_ENTRY_and_EXIT;

    QSqlQuery updateQ;
    updateQ.prepare("UPDATE Vertraege SET LetzteZinsberechnung = :vdate, aktiv = :true WHERE id = :id");
    updateQ.bindValue(":vdate",QVariant(activationDate));
    updateQ.bindValue(":id", QVariant(ContractId));
    updateQ.bindValue(":true", QVariant(true));
    bool ret = updateQ.exec();
    qDebug() << updateQ.lastQuery() << updateQ.lastError();
    return ret;
}

bool VertragLoeschen(QString index)
{   LOG_ENTRY_and_EXIT;

    QSqlQuery deleteQ;
    if( !deleteQ.exec("DELETE FROM Vertraege WHERE id=" + index))
    {
        qCritical() << "failed to delete Contract: " << deleteQ.lastError() << "\n" << deleteQ.lastQuery();
        return false;
    }
    return true;
}
