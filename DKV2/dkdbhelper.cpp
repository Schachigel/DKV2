#include <windows.h>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QtCore>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

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
    Kreditoren.append(dbfield("id",       QVariant::Int,    "PRIMARY KEY AUTOINCREMENT"));
    Kreditoren.append(dbfield("Vorname",  QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("Nachname", QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("Strasse",  QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("Plz",      QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("Stadt",    QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("IBAN",     QVariant::String));
    Kreditoren.append(dbfield("BIC",      QVariant::String));
    dkdbstructur.appendTable(Kreditoren);

    dbtable Zinssaetze("Zinssaetze");
    Zinssaetze.append(dbfield("id",       QVariant::Int,    "PRIMARY KEY AUTOINCREMENT"));
    Zinssaetze.append(dbfield("Zinssatz", QVariant::Double, "DEFAULT '0,0' UNIQUE NULL"));
    Zinssaetze.append(dbfield("Bemerkung"));
    dkdbstructur.appendTable(Zinssaetze);

    dbtable Vertraege("Vertraege");
    Vertraege.append((dbfield("id",         QVariant::Int, "PRIMARY KEY AUTOINCREMENT")));
    Vertraege.append((dbfield("KreditorId", QVariant::Int, "", Kreditoren["id"], dbfield::refIntOption::onDeleteCascade )));
    Vertraege.append((dbfield("Kennung")));
    Vertraege.append((dbfield("Betrag",     QVariant::Double, "DEFAULT '0,0' NOT NULL")));
    Vertraege.append((dbfield("Wert",       QVariant::Double, "DEFAULT '0,0' NULL")));
    Vertraege.append((dbfield("ZSatz",      QVariant::Int, "", Zinssaetze["id"], dbfield::refIntOption::non)));
    Vertraege.append((dbfield("tesaurierend",  QVariant::Bool, "DEFAULT '1' NOT NULL")));
    Vertraege.append((dbfield("Vertragsdatum", QVariant::Date, "DATE  NULL")));
    Vertraege.append((dbfield("aktiv",         QVariant::Bool, "DEFAULT '0' NOT NULL")));
    Vertraege.append((dbfield("LaufzeitEnde",  QVariant::Date, "DEFAULT '3000-12-31' NOT NULL")));
    Vertraege.append((dbfield("LetzteZinsberechnung", QVariant::Date, "NULL")));
    dkdbstructur.appendTable(Vertraege);

    dbtable Buchungsarten("Buchungsarten");
    Buchungsarten.append(dbfield("id",  QVariant::Int, "PRIMARY KEY AUTOINCREMENT"));
    Buchungsarten.append(dbfield("Art", QVariant::String, "NOT NULL"));
    dkdbstructur.appendTable(Buchungsarten);

    dbtable Buchungen("Buchungen");
    Buchungen.append(((dbfield("id",           QVariant::Int, "PRIMARY KEY AUTOINCREMENT"))));
    Buchungen.append(((dbfield("VertragId",    QVariant::Int, "", Vertraege["id"], dbfield::refIntOption::onDeleteNull))));
    Buchungen.append(((dbfield("Buchungsart",  QVariant::Int, "", Buchungsarten["id"], dbfield::refIntOption::non))));
    Buchungen.append(((dbfield("Betrag",       QVariant::Double, "DEFAULT '0' NULL"))));
    Buchungen.append(((dbfield("Datum",        QVariant::Date))));
    Buchungen.append(((dbfield("Bemerkung",    QVariant::String))));
    dkdbstructur.appendTable(Buchungen);

    dbtable meta("Meta");
    meta.append(dbfield("Name", QVariant::String, "NOT NULL"));
    meta.append(dbfield("Wert", QVariant::String, "NOT NULL"));
    dkdbstructur.appendTable(meta);
}

bool ZinssaetzeEinfuegen()
{
    TableDataInserter ti(dkdbstructur["Zinssaetze"]);
    ti.setValue("Zinssatz", 0.);
    ti.setValue("Bemerkung", "Unser Held");
    bool ret = ti.InsertData();

    double Zins = 0.1;
    for (Zins=0.1; Zins < .6; Zins+=0.1)
    {
        ti.setValue("Zinssatz", Zins); ti.setValue("Bemerkung", "Unser Freund");
        QSqlQuery sql;
        ret &= ti.InsertData();
    }
    for (; Zins < 1.1; Zins+=0.1){
        ti.setValue("Zinssatz", Zins); ti.setValue("Bemerkung", "Unser Förderer");
        QSqlQuery sql;
        ret &= ti.InsertData();
    }
    for (; Zins < 2.; Zins+=0.1)
    {
        ti.setValue("Zinssatz", Zins); ti.setValue("Bemerkung", "Unser Investor");
        QSqlQuery sql;
        ret &= ti.InsertData();
    }
    if( !ret)
        qCritical() << "There was an error creating intrest values";
    return ret;
}

bool BuchungsartenEinfuegen()
{
    QStringList arten{"Vertrag anlegen", "Vertrag aktivieren", "Passiven Vertrag löschen", "Vertrag beenden"};
    bool ret = true;
    for( auto art: arten)
    {
        TableDataInserter ti( dkdbstructur["Buchungsarten"]);
        ti.setValue("Art", QVariant(art));

        ret &= ti.InsertData();
    }
    return ret;
}

bool EigenschaftenEinfuegen()
{
    QSqlQuery sql;
    sql.exec("INSERT INTO Meta (Name, Wert) VALUES (\"Version\", \"1.0\"");
    return true;
}

bool DKDatenbankAnlegen(const QString& filename)
{    LOG_ENTRY_and_EXIT;

    DatenbankverbindungSchliessen();
    if( QFile(filename).exists())
    {
        backupFile(filename);
        QFile(filename).remove();
    }
    dbCloser closer;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filename);

    if( !db.open()) return false;
    bool ret = true;
    closer.set(&db);
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON");
//    db.transaction();
    ret &= dkdbstructur.createDb(db);
    ret &= ZinssaetzeEinfuegen();
    ret &= BuchungsartenEinfuegen();
    ret &= EigenschaftenEinfuegen();
//    if( ret) db.commit(); else db.rollback();

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
        sql.prepare(QString("SELECT * FROM ") + table.Name());
        if( !sql.exec())
        {
            qDebug() << "testing for table " << table.Name() << " failed\n" << sql.lastError() << "\n" << sql.lastQuery();
            return false;
        }
    }
    qDebug() << db.databaseName() << " has all tables expected";
    return true;
}

bool istValideDatenbank(const QString& filename)
{   LOG_ENTRY_and_EXIT;

    if( filename == "") return false;
    if( !QFile::exists(filename)) return false;

    dbCloser closer; // create before db
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "validate");
    db.setDatabaseName(filename);
    if( !db.open())
        return false;
    closer.set(&db);
    QSqlQuery enableRefInt(db);
    enableRefInt.exec("PRAGMA foreign_keys = ON");
    if( !hatAlleTabellen(db))
        return false;

    qDebug() << filename << " is a valid dk database";
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
        QDate vertragsdatum= QDate::currentDate().addDays(-1 * rand.bounded(365));
        c.Vertragsdatum = vertragsdatum;
        c.active = 0 != rand.bounded(3)%3; // random data, more true then false
        if( c.active.toBool() ) c.StartZinsberechnung =vertragsdatum.addDays(rand.bounded(15));
        c.verbucheVertrag();
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
        qDebug() << query.lastQuery() << "executed successfully w SQL statement:\n" << sql;
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

int BuchungsartIdFromArt(QString s)
{
    QSqlQuery query("SELECT * FROM Buchungsarten WHERE Art =\"" + s + "\"");
    query.next();
    int i = query.value(0).toInt();
    return i;
}

void baueDatensatzStrukturFuerBelegNeuerVertrag(QVector<dbfield>& fields)
{
    fields.append(dkdbstructur["Vertraege" ]["id"]);
    fields.append(dkdbstructur["Vertraege"]["Betrag"]);
    fields.append(dkdbstructur["Vertraege"]["Wert"]);
    fields.append(dkdbstructur["Zinssaetze"]["Zinssatz"]);
    fields.append(dkdbstructur["Vertraege"]["tesaurierend"]);
    fields.append(dkdbstructur["Vertraege"]["Vertragsdatum"]);
    fields.append(dkdbstructur["Vertraege"]["aktiv"]);
    fields.append(dkdbstructur["Vertraege"]["LetzteZinsberechnung"]);
    fields.append(dkdbstructur["Kreditoren"]["id"]);
    fields.append(dkdbstructur["Kreditoren"]["Vorname"]);
    fields.append(dkdbstructur["Kreditoren"]["Nachname"]);
    fields.append(dkdbstructur["Kreditoren"]["Strasse"]);
    fields.append(dkdbstructur["Kreditoren"]["Plz"]);
    fields.append(dkdbstructur["Kreditoren"]["Stadt"]);
    fields.append(dkdbstructur["Kreditoren"]["IBAN"]);
    fields.append(dkdbstructur["Kreditoren"]["BIC"]);
    fields.append(dkdbstructur["Zinssaetze"]["id"]);
    fields.append(dkdbstructur["Vertraege" ]["ZSatz"]);
}

QSqlRecord DatensatzFuerBelegNeuerVertrag(const int VertragId, const QVector<dbfield>& fields)
{
    QString sql("SELECT ");
    for( int i =0; i<fields.size(); i++)
    {
        if( i) sql += ", ";
        sql += fields[i].tableName() + "." + fields[i].name();
    }
    sql += " FROM Vertraege, Kreditoren, Zinssaetze "
           " WHERE Vertraege.id = " + QString::number(VertragId) +
           " AND Kreditoren.id = Vertraege.KreditorId AND Vertraege.ZSatz = Zinssaetze.id ";

    QSqlQuery all;
    if( !all.exec(sql))
    {
        qDebug() << "preparing buchungstext failed " << all.lastError() << "\n" << all.lastQuery();
        return QSqlRecord();
    }
    all.next();
    return all.record();
}

QJsonValue jsonValueFromVariant(QVariant v)
{
    switch(v.type())
    {
    case QVariant::Int:
        return QJsonValue(v.toInt());
    case QVariant::String:
        return QJsonValue(v.toString());
    case QVariant::Double:
        return QJsonValue(v.toDouble());
    case QVariant::Date:
        return QJsonValue(v.toDate().toString(Qt::ISODate));
    case QVariant::Bool:
        return QJsonValue(v.toBool());
    default:
        qDebug() << "jsonValueFromVariant: invalid data type: " << v;
    }
    return QJsonValue();
}

QString JasonAusDatensatz( QSqlRecord r)
{
    QJsonObject Vertrag;
    QJsonObject Kreditor;
    QJsonObject Zinssatz;
    for( int i =0; i<r.count(); i++)
    {
        QSqlField f(r.field(i));
        if( f.tableName() == "Vertraege")
            Vertrag.insert(f.name(), jsonValueFromVariant(f.value()));
        else if( f.tableName() == "Kreditoren")
            Kreditor.insert(f.name(), jsonValueFromVariant(f.value()));
        else if( f.tableName() == "Zinssaetze")
            Zinssatz.insert(f.name(), jsonValueFromVariant(f.value()));
        else qDebug() << "unknown table info in record: " << f;
    }
    QJsonArray Beleg;
    Beleg.push_back(Vertrag);
    Beleg.push_back(Kreditor);
    Beleg.push_back(Zinssatz);

    return QJsonDocument(Beleg).toJson();
}

bool VertragsDaten::BelegZuNeuemVertragSpeichern(const int VertragId)
{   LOG_ENTRY_and_EXIT;

    QVector<dbfield> fields;
    baueDatensatzStrukturFuerBelegNeuerVertrag(fields);
    QSqlRecord beleg = DatensatzFuerBelegNeuerVertrag(VertragId, fields);

    QString Buchungstext = JasonAusDatensatz(beleg);

    qDebug() << "Speichere Buchungsdaten:\n" << Buchungstext;
    QSqlQuery sqlBuchung;
    sqlBuchung.prepare("INSERT INTO Buchungen (VertragId, Buchungsart, Betrag, Datum, Bemerkung)"
                       " VALUES (:VertragsId, :Buchungsart, :Betrag, :Datum, :Bemerkung)");
    sqlBuchung.bindValue(":VertragsId", QVariant(VertragId));
    sqlBuchung.bindValue(":Buchungsart", QVariant(BuchungsartIdFromArt("Vertrag anlegen")));
    sqlBuchung.bindValue(":Betrag", QVariant(Betrag));
    sqlBuchung.bindValue(":Datum", QVariant(QDate::currentDate()));
    sqlBuchung.bindValue(":Bemerkung", QVariant(Buchungstext));
    if( !sqlBuchung.exec())
    {
        qCritical() << "Buchung wurde nicht gesp. Fehler: " << sqlBuchung.lastError();
        return false;
    }
    return true;
}

bool VertragsDaten::verbucheVertrag()
{   LOG_ENTRY_and_EXIT;

    QSqlDatabase::database().transaction();
    int vid = speichereVertrag();
    if( vid >0 )
        if( BelegZuNeuemVertragSpeichern(vid))
        {
            QSqlDatabase::database().commit();
            return true;
        }

    qCritical() << "ein neuer Vertrag konnte nicht gespeichert werden";
    QSqlDatabase::database().rollback();
    return false;
}

int VertragsDaten::speichereVertrag()
{   LOG_ENTRY_and_EXIT;
    TableDataInserter ti(dkdbstructur["Vertraege"]);
    ti.setValue(dkdbstructur["Vertraege"]["KreditorId"].name(), KreditorId);
    ti.setValue(dkdbstructur["Vertraege"]["Kennung"].name(), Kennung);
    ti.setValue(dkdbstructur["Vertraege"]["Betrag"].name(), Betrag);
    ti.setValue(dkdbstructur["Vertraege"]["Wert"].name(), Wert);
    ti.setValue(dkdbstructur["Vertraege"]["ZSatz"].name(), Zins);
    ti.setValue(dkdbstructur["Vertraege"]["tesaurierend"].name(), tesaurierend);
    ti.setValue(dkdbstructur["Vertraege"]["Vertragsdatum"].name(), Vertragsdatum);
    ti.setValue(dkdbstructur["Vertraege"]["aktiv"].name(), active);
    ti.setValue(dkdbstructur["Vertraege"]["LaufzeitEnde"].name(), LaufzeitEnde);
    ti.setValue(dkdbstructur["Vertraege"]["LetzteZinsberechnung"].name(), StartZinsberechnung);
    if( ti.InsertData(QSqlDatabase::database()))
    {
        int lastid = ti.getInsertedRecordId();
        qDebug() << "Neuer Vertrag wurde eingefügt mit id:" << lastid;
        return lastid;
    }
    return -1;
}

bool VertragAktivieren( int ContractId, const QDate& activationDate)
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

bool passivenVertragLoeschen(const QString& index)
{   LOG_ENTRY_and_EXIT;
    if( ExecuteSingleValueSql("SELECT [aktiv] FROM [Vertraege] WHERE id=" +index).toBool())
    {
        qWarning() << "will not delete active contract w id:" << index;
        return false;
    }
    QSqlQuery deleteQ;
    if( !deleteQ.exec("DELETE FROM Vertraege WHERE id=" + index))
    {
        qCritical() << "failed to delete Contract: " << deleteQ.lastError() << "\n" << deleteQ.lastQuery();
        return false;
    }
    return true;
}

bool aktivenVertragLoeschen(const int index, const QDate /*ende*/, double& /*neuerWert*/, double& /*davonZins*/)
{   LOG_ENTRY_and_EXIT;
    if( !ExecuteSingleValueSql("[aktiv]", "[Vertraege]", "id=" +QString::number(index)).toBool())
    {
        qWarning() << "will not delete passive contract w id:" << index;
        return false;
    }
    QString sql = "SELECT [Vertrage.Betrag], [Vertraege.Wert], [Vertraege.LetzteZinsberechnung], [Zinssaetze.Zinssatz] ";
        sql += "FROM [Vertrage], [Zinssatz] ";
        sql += "WHERE id=" + QString::number(index) + " AND [Zinssaetze].[id]=[Vertraege].[ZSatz]";

   QVector<dbfield> fields;
   fields.push_back(dkdbstructur["Vertraege"]["Betrag"]);
   fields.push_back(dkdbstructur["Vertraege"]["Wert"]);
   fields.push_back(dkdbstructur["Vertraege"]["ZSatz"]);
   fields.push_back(dkdbstructur["Zinssaetze"]["Zinssatz"]);

   QSqlRecord rec = ExecuteSingleRecordSql(fields, "[Vertraege].[id]="+QString::number(index));


    /* to do
     * - ende < letzteZinsberechnung -> ERROR
     * - calculate time difference ende-letzteZinsberechnung
     * - calculate davonZins = Wert*(ZinsProZeiteinheit * time differenz)
     * - calculate neuerWert = Wert+davonZins
     * return
     */
    return true;
}

QString ContractList_SELECT(const QVector<dbfield>& fields)
{   LOG_ENTRY_and_EXIT;
    QString sql("SELECT ");
    for( int i = 0; i < fields.size(); i++)
    {
        if( i) sql +=", ";
        sql += fields[i].tableName() +"." +fields[i].name();
    }
    return sql;
}

QString ContractList_FROM()
{
    return  "FROM Vertraege, Kreditoren, Zinssaetze";
}
QString ContractList_WHERE(const QString& Filter)
{
    QString s ("WHERE Kreditoren.id = Vertraege.KreditorId AND Vertraege.ZSatz = Zinssaetze.id");
    bool isNumber (false);
    int index = Filter.toInt(&isNumber);
    if (isNumber)
    {
        s += " AND Kreditoren.id = '" + QString::number(index) + "'";
        return s;
    }
    if( Filter.size()> 2)
    {
        s += " AND ( Vorname LIKE '%" + Filter + "%' OR Nachname LIKE '%" + Filter + "%' )";
    }
    return s;
}
QString ContractList_SQL(const QVector<dbfield>& fields, const QString& filter)
{
    QString sql = ContractList_SELECT(fields) + " "
           + ContractList_FROM() + " "
           + ContractList_WHERE(filter);
    qDebug() << "ContractList SQL: \n" << sql;
    return sql;
}

QString SelectQueryFromFields(const QVector<dbfield>& fields, const QString& where)
{
    QString Select ("SELECT ");
    QString From ("FROM ");
    QString Where("WHERE " + where);

    QStringList usedTables;
    for(int i=0; i < fields.count(); i++)
    {
        const dbfield& f = fields[i];
        if( i!=0)
            Select += ", ";
        Select += "[" + f.tableName() + "].[" + f.name() + "]";

        if( !usedTables.contains(f.tableName()))
        {
            if( usedTables.count()!= 0)
                From += ", ";
            usedTables.push_back(f.tableName());
            From += "[" + f.tableName() +"]";
        }

        refFieldInfo ref = f.getReferenzeInfo();
        if( !ref.tablename.isEmpty())
        {
            Where += " AND [" + ref.tablename + "].[" + ref.name + "]=[" + f.tableName() +"].[" + f.name() +"]";
        }
    }
    return Select + " " + From + " " + Where;
}

QSqlRecord ExecuteSingleRecordSql(const QVector<dbfield>& fields, const QString& where, const QString& con)
{
    QString sql = SelectQueryFromFields(fields, where);
    qDebug() << "ExecuteSingleRecordSql:\n" << sql;
    QSqlQuery q(QSqlDatabase::database(con));
    q.prepare(sql);
    if( !q.exec())
    {
        qCritical() << "SingleRecordSql failed " << q.lastError() << "\n" << q.lastQuery();
        return QSqlRecord();
    }
    q.last();
    if(q.at() != 0)
    {
        qCritical() << "SingleRecordSql returned more then one value\n" << q.lastQuery();
        return QSqlRecord();
    }
    return q.record();
}

QVariant ExecuteSingleValueSql(const QString& s, const QString& con)
{   LOG_ENTRY_and_EXIT;
    QSqlQuery q(QSqlDatabase::database(con));
    q.prepare(s);
    if( !q.exec())
    {
        qCritical() << "SingleValueSql failed to execute: " << q.lastError() << "\n" << q.lastQuery();
        return QVariant();
    }
    q.last();
    if(q.at() != 0)
    {
        qCritical() << "SingleValueSql returned more then one value\n" << q.lastQuery();
        return QVariant();
    }
    return q.value(0);
}

QVariant ExecuteSingleValueSql( const QString& field, const QString& table, const QString& where, const QString& con)
{
    QString sql = "SELECT " + field + " FROM " + table + " WHERE " + where;
    return ExecuteSingleValueSql(sql, con);
}

void berechneZusammenfassung(DbSummary& dbs, QString con)
{
    dbs.aktiveDk  = ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] = 1", con).toReal();
    dbs.passiveDk = ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] = 0", con).toReal();
    dbs.WertAktiveDk = ExecuteSingleValueSql("SUM([Wert])", "[Vertraege]", "[aktiv] = 1", con).toReal();
}
