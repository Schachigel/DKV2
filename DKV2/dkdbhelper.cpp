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
#include "htmlbrief.h"

dbstructure dkdbstructur;

QList<QPair<int, QString>> Buchungsarten;

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
    Kreditoren.append(dbfield("id",       QVariant::Int,    "PRIMARY KEY AUTOINCREMENT"));
    Kreditoren.append(dbfield("Vorname",  QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("Nachname", QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("Strasse",  QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("Plz",      QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("Stadt",    QVariant::String, "NOT NULL"));
    Kreditoren.append(dbfield("Email",    QVariant::String, "TYPE UNIQUE"));
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
    Zinssaetze.append(dbfield("id",       QVariant::Int,    "PRIMARY KEY AUTOINCREMENT"));
    Zinssaetze.append(dbfield("Zinssatz", QVariant::Double, "DEFAULT '0,0' UNIQUE NULL"));
    Zinssaetze.append(dbfield("Bemerkung"));
    dkdbstructur.appendTable(Zinssaetze);

    dbtable Vertraege("Vertraege");
    Vertraege.append((dbfield("id",         QVariant::Int, "PRIMARY KEY AUTOINCREMENT")));
    Vertraege.append((dbfield("KreditorId", QVariant::Int, "", Kreditoren["id"], dbfield::refIntOption::onDeleteCascade )));
    Vertraege.append((dbfield("Kennung",    QVariant::String, "UNIQUE")));
    Vertraege.append((dbfield("Betrag",     QVariant::Double, "DEFAULT '0,0' NOT NULL")));
    Vertraege.append((dbfield("Wert",       QVariant::Double, "DEFAULT '0,0' NULL")));
    Vertraege.append((dbfield("ZSatz",      QVariant::Int, "", Zinssaetze["id"], dbfield::refIntOption::non)));
    Vertraege.append((dbfield("thesaurierend", QVariant::Bool, "DEFAULT '1' NOT NULL")));
    Vertraege.append((dbfield("Vertragsdatum", QVariant::Date, "DATE  NULL")));
    Vertraege.append((dbfield("aktiv",         QVariant::Bool, "DEFAULT '0' NOT NULL")));
    Vertraege.append((dbfield("LaufzeitEnde",  QVariant::Date, "DEFAULT '3000-12-31' NOT NULL")));
    Vertraege.append((dbfield("LetzteZinsberechnung", QVariant::Date, "NULL")));
    Vertraege.append((dbfield("Kfrist" ,    QVariant::Int, "DEFAULT '6' NOT NULL")));
    dkdbstructur.appendTable(Vertraege);

    dbtable Buchungsarten("Buchungsarten");
    Buchungsarten.append(dbfield("id",  QVariant::Int, "PRIMARY KEY"));
    Buchungsarten.append(dbfield("Art", QVariant::String, "NOT NULL"));
    dkdbstructur.appendTable(Buchungsarten);

    dbtable Buchungen("Buchungen");
    Buchungen.append(((dbfield("id",           QVariant::Int, "PRIMARY KEY AUTOINCREMENT"))));
    Buchungen.append(((dbfield("VertragId",    QVariant::Int, "", Vertraege["id"], dbfield::refIntOption::onDeleteNull))));
    Buchungen.append(((dbfield("Buchungsart",  QVariant::Int, "", Buchungsarten["id"], dbfield::refIntOption::non))));
    Buchungen.append(((dbfield("Betrag",       QVariant::Double, "DEFAULT '0' NULL"))));
    Buchungen.append(((dbfield("Datum",        QVariant::Date))));
    Buchungen.append(((dbfield("Bemerkung",    QVariant::String))));
    Buchungen.append(((dbfield("Buchungsdaten",    QVariant::String))));
    dkdbstructur.appendTable(Buchungen);

    dbtable meta("Meta");
    meta.append(dbfield("Name", QVariant::String, "NOT NULL"));
    meta.append(dbfield("Wert", QVariant::String, "NOT NULL"));
    dkdbstructur.appendTable(meta);
    done = true;
}

void initBuchungsarten()
{LOG_ENTRY_and_EXIT;
    //    "NOOP",
    //    "Vertrag anlegen",
    //    "Vertrag aktivieren",
    //    "Passiven Vertrag löschen",
    //    "Vertrag beenden",
    //    "Zinsgutschrift"
    Buchungsarten.push_back(QPair<int, QString>(Buchungsart_i::NOOP, ""));
    Buchungsarten.push_back(QPair<int, QString>(Buchungsart_i::VERTRAG_ANLEGEN, "Vertrag anlegen"));
    Buchungsarten.push_back(QPair<int, QString>(Buchungsart_i::VERTRAG_AKTIVIEREN, "Vertrag aktivieren"));
    Buchungsarten.push_back(QPair<int, QString>(Buchungsart_i::PASSIVEN_VERTRAG_LOESCHEN, "Passiven Vertrag löschen"));
    Buchungsarten.push_back(QPair<int, QString>(Buchungsart_i::VERTRAG_BEENDEN, "Vertrag beenden"));
    Buchungsarten.push_back(QPair<int, QString>(Buchungsart_i::ZINSGUTSCHRIFT, "Zinsgutschrift"));
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

bool BuchungsartenEinfuegen(QSqlDatabase db)
{LOG_ENTRY_and_EXIT;
    bool ret = true;
    for( auto art: Buchungsarten)
    {
        TableDataInserter ti( dkdbstructur["Buchungsarten"]);
        ti.setValue("id", QVariant(art.first));
        ti.setValue("Art", QVariant(art.second));

        ret &= 0<= ti.InsertData(db);
    }
    return ret;
}

bool EigenschaftenEinfuegen(QSqlDatabase db)
{LOG_ENTRY_and_EXIT;
    bool ret =true;
    QSqlQuery sql(db);
    QRandomGenerator *rand = QRandomGenerator::system();
    ret &= sql.exec("INSERT INTO Meta (Name, Wert) VALUES ('Version', '1.0')");
    ret &= sql.exec("INSERT INTO Meta (Name, Wert) VALUES ('IdOffset', '" + QString::number(rand->bounded(10000,20000)) + "')");
    ret &= sql.exec("INSERT INTO Meta (Name, Wert) VALUES ('ProjektInitialen', 'ESP')");
    // ret &= sql.exec("INSERT INTO Meta (Name, Wert) VALUES ('Lettertemplate', '" +htmlbrief::getTemplate() + "')");
    return ret;
}

QVariant Eigenschaft(const QString& name)
{LOG_ENTRY_and_EXIT;
    return ExecuteSingleValueSql("SELECT WERT FROM Meta WHERE Name='" + name +"'");
}

bool DKDatenbankAnlegen(const QString& filename, QSqlDatabase db)
{LOG_ENTRY_and_EXIT;
    if( (filename.length()>0) == db.isValid())
        // use this function with db xor with filename
        return false;
    dbCloser closer;
    if( !filename.isEmpty())
    {
        DatenbankverbindungSchliessen();
        if( QFile(filename).exists())
        {
            backupFile(filename);
            QFile(filename).remove();
        }
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(filename);

        if( !db.open()) return false;
        closer.set(&db);
    }
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
    if (istValideDatenbank(filename))
        return ret;
    else
    {
        qCritical() << "Newly created db is invalid. We should panic";
        return false;
    }
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

bool istValideDatenbank(const QString& filename)
{LOG_ENTRY_and_EXIT;

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
    if( !hatAlleTabellenUndFelder(db))
        return false;

    qDebug() << filename << " is a valid dk database";
    return true;
}

void DatenbankverbindungSchliessen()
{LOG_ENTRY_and_EXIT;

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

QString ProposeKennung()
{LOG_ENTRY_and_EXIT;
    int idOffset = Eigenschaft("IdOffset").toInt();
    QString maxid = QString::number(idOffset + getHighestTableId("Vertraege")).rightJustified(6, '0');
    QString PI = "DK-" + Eigenschaft("ProjektInitialen").toString();
    return PI + "-" + QString::number(QDate::currentDate().year()) + "-" + maxid;
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
    vertrag = Vertrag(KId, ProposeKennung(),
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
        k.setValue("IBAN", "DExx-xxxxx");
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

void berechneVertragsenden( QVector<ContractEnd>& ce, QString con)
{LOG_ENTRY_and_EXIT;

    QMap<int, int> m_count;
    QMap<int, double> m_sum;
    const int maxYear = QDate::currentDate().year() +99;

    QSqlQuery sql(QSqlDatabase::database(con));
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

void berechneJahrZinsVerteilung( QVector<YZV>& yzv, QString con)
{
    QString sql = "SELECT count(*), Substr([Vertragsdatum], 0, 5), [Zinssaetze].[Zinssatz]"
                  "FROM [Vertraege], [Zinssaetze] "
                  "WHERE [ZSatz] = [Zinssaetze].[id] "
                  "GROUP BY Substr([Vertragsdatum], 0, 4), [ZSatz]";
    QSqlQuery query(con);
    query.exec(sql);
    while( query.next())
    {
        QSqlRecord r =query.record();
        yzv.push_back({r.value(1).toInt(), r.value(2).toReal(), r.value(0).toInt() });
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
