#if defined(__APPLE__)
#include <mach/mach_time.h>
uint32_t GetTickCount() {
    uint64_t mat = mach_absolute_time();
    uint32_t mul = 0x80d9594e;
    return ((((0xffffffff & mat) * mul) >> 32) + (mat >> 32) * mul) >> 23;
}
#else
#include <windows.h>
#endif

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
    Vertraege.append((dbfield("tesaurierend",  QVariant::Bool, "DEFAULT '1' NOT NULL")));
    Vertraege.append((dbfield("Vertragsdatum", QVariant::Date, "DATE  NULL")));
    Vertraege.append((dbfield("aktiv",         QVariant::Bool, "DEFAULT '0' NOT NULL")));
    Vertraege.append((dbfield("LaufzeitEnde",  QVariant::Date, "DEFAULT '3000-12-31' NOT NULL")));
    Vertraege.append((dbfield("LetzteZinsberechnung", QVariant::Date, "NULL")));
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
    ret &= sql.exec("INSERT INTO Meta (Name, Wert) VALUES ('Version', '1.0')");
    QRandomGenerator rand(::GetTickCount());
    ret &= sql.exec("INSERT INTO Meta (Name, Wert) VALUES ('IdOffset', '" + QString::number(rand.bounded(10000,20000)) + "')");
    ret &= sql.exec("INSERT INTO Meta (Name, Wert) VALUES ('ProjektInitialen', 'ESP')");
    ret &= sql.exec("INSERT INTO Meta (Name, Wert) VALUES ('Lettertemplate', '" +htmlbrief::getTemplate() + "')");
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
        sql.prepare(QString("SELECT * FROM ") + table.Name());
        if( !sql.exec())
        {
            qDebug() << "testing for table " << table.Name() << " failed\n" << sql.lastError() << endl << sql.lastQuery();
            return false;
        }
        if( table.Fields().count() != sql.record().count())
        {
            qCritical() << "Tabelle " << table.Name() << " hat nicht genug Felder";
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

QString ProposeKennung()
{LOG_ENTRY_and_EXIT;
    int idOffset = Eigenschaft("IdOffset").toInt();
    QString maxid = QString::number(idOffset + getHighestTableId("Vertraege")).rightJustified(6, '0');
    QString PI = "DK-" + Eigenschaft("ProjektInitialen").toString();
    return PI + "-" + QString::number(QDate::currentDate().year()) + "-" + maxid;
}

void BeispieldatenAnlegen( int AnzahlDatensaetze)
{LOG_ENTRY_and_EXIT;

    QList<QString> Vornamen {"Holger", "Volker", "Peter", "Hans", "Susi", "Roland", "Claudia", "Emil", "Evelyn", "Ötzgür", "Thomas", "Elke", "Berta", "Malte", "Jori", "Paul", "Jonas", "Finn", "Leon", "Luca", "Emma", "Mia", "Lena", "Anna"};
    QList<QString> Nachnamen {"Maier", "Müller", "Schmit", "Kramp", "Adams", "Häcker", "Maresch", "Beutl", "Chauchev", "Chen", "Kirk", "Ohura", "Gorbatschov", "Merkel", "Karrenbauer", "Tritin", "Schmidt", "Rao", "Lassen", "Hurgedü"};
    QList<QString> Strassen {"Hauptstrasse", "Nebenstrasse", "Bahnhofstrasse", "Kirchstraße", "Dorfstrasse", "Süterlinweg", "Sorbenstrasse", "Kleines Gässchen", "Industriestrasse", "Sesamstrasse", "Lindenstrasse"};
    QList<QString> emailprovider {"gmail.com", "googlemail.com", "mailbox.org", "t-online.de", "mail.de", "mail.com", "online.de", "yahoo.de", "yahoo.com", "telekom.de", "proivder.co.uk"};
    QList <QPair<QString, QString>> Cities {{"68305", "Mannheim"}, {"69123", "Heidelberg"}, {"69123", "Karlsruhe"}, {"90345", "Hamburg"}};
    QRandomGenerator rand(::GetTickCount());
    int maxZinsIndex = ExecuteSingleValueSql("SELECT max(id) FROM Zinssaetze").toInt();
    for( int i = 0; i<AnzahlDatensaetze; i++)
    {
        Kreditor k;
        QString vn (Vornamen [rand.bounded(Vornamen.count ())]);
        QString nn (Nachnamen [rand.bounded(Nachnamen.count ())]);
        k.setValue("Vorname", vn);
        k.setValue("Nachname", nn);
        k.setValue("Strasse", Strassen[rand.bounded(Strassen.count())]);
        k.setValue("Plz", Cities[rand.bounded(Cities.count())].first);
        k.setValue("Stadt", Cities[rand.bounded(Cities.count())].second);
        k.setValue("Email", vn+"."+nn+"@"+emailprovider[rand.bounded(emailprovider.count())]);
        k.setValue("IBAN", "DExx-xxxxx");
        k.setValue("BIC", "bic...");

        int neueKreditorId =k.Speichern();
        if( -1 == neueKreditorId)
        {
            qCritical() << "No id from Kreditor.Speichern";
            Q_ASSERT(!bool("Verbuchung des neuen Vertrags gescheitert"));
        }
        // add a contract
        double betragUWert = double(100) * rand.bounded(1,20);
        int zinsid = rand.bounded(1,maxZinsIndex);
        bool tesa = rand.bounded(100)%2 ? true : false;
        bool active = 0 != rand.bounded(3)%3;
        QDate vertragsdatum= QDate::currentDate().addDays(-1 * rand.bounded(365));
        QDate StartZinsberechnung = ( active) ? vertragsdatum.addDays(rand.bounded(15)) : QDate(9999, 12, 31);

        Vertrag vertrag (neueKreditorId, ProposeKennung(),
                         betragUWert, betragUWert, zinsid,
                         vertragsdatum,
                         tesa, active, StartZinsberechnung);
        vertrag.verbucheNeuenVertrag();
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
{
    dbs.aktiveDk  = ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] = 1", con).toReal();
    dbs.passiveDk = ExecuteSingleValueSql("SUM([Betrag])", "[Vertraege]", "[aktiv] = 0", con).toReal();
    dbs.WertAktiveDk = ExecuteSingleValueSql("SUM([Wert])", "[Vertraege]", "[aktiv] = 1", con).toReal();
}
