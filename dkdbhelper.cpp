#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QtCore>

#include "dkdbhelper.h"

static dkdbschema dbstructure;

void initDbHelper()
{
    dkdbtable DkGeber("DKGeber");
    DkGeber.Fields.append(dkdbfield("id", "INTEGER DEFAULT '1' NOT NULL PRIMARY KEY AUTOINCREMENT"));
    DkGeber.Fields.append(dkdbfield("Vorname", "TEXT  NOT NULL"));
    DkGeber.Fields.append(dkdbfield("Nachname", "TEXT  NOT NULL"));
    DkGeber.Fields.append(dkdbfield("Strasse", "TEXT  NOT NULL"));
    DkGeber.Fields.append(dkdbfield("Plz", "TEXT  NOT NULL"));
    DkGeber.Fields.append(dkdbfield("Stadt", "TEXT  NOT NULL"));
    DkGeber.Fields.append(dkdbfield("IBAN", "TEXT"));
    DkGeber.Fields.append(dkdbfield("BIC", "TEXT"));
    dbstructure.Tables.append(DkGeber);

    dkdbtable DkVertrag("DKVertrag");
    DkVertrag.Fields.append((dkdbfield("id", "INTEGER DEFAULT '0' NOT NULL PRIMARY KEY AUTOINCREMENT")));
    DkVertrag.Fields.append((dkdbfield("DKGeberId", "INTEGER FOREIGN_KEY REFERENCES [DKGeber](id)")));
    DkVertrag.Fields.append((dkdbfield("Kennung", "TEXT  NULL")));
    DkVertrag.Fields.append((dkdbfield("Betrag", "FLOAT DEFAULT '0,0' NOT NULL")));
    DkVertrag.Fields.append((dkdbfield("Wert", "FLOAT DEFAULT '0,0' NULL")));
    DkVertrag.Fields.append((dkdbfield("ZSatz", "FLOAT DEFAULT '0,0' NOT NULL")));
    DkVertrag.Fields.append((dkdbfield("tesaurierend", "BOOLEAN DEFAULT '1' NOT NULL")));
    DkVertrag.Fields.append((dkdbfield("Vertragsdatum", "DATE  NULL")));
    DkVertrag.Fields.append((dkdbfield("aktiv", "BOOLEAN DEFAULT 'false' NOT NULL")));
    DkVertrag.Fields.append((dkdbfield("LaufzeitEnde", "DATE DEFAULT '3000-12-31' NOT NULL")));
    DkVertrag.Fields.append((dkdbfield("LetzteZinsberechnung", "DATE  NULL")));
    dbstructure.Tables.append(DkVertrag);

    dkdbtable Buchung("Buchungen");
    Buchung.Fields.append(((dkdbfield("id", "INTEGER DEFAULT '0' NOT NULL PRIMARY KEY AUTOINCREMENT"))));
    Buchung.Fields.append(((dkdbfield("VertragId", "INTEGER FOREIGN_KEY REFERENCES [DKVertrag](id)"))));
    Buchung.Fields.append(((dkdbfield("Buchungsart", "INTEGER DEFAULT '0' NOT NULL"))));
    Buchung.Fields.append(((dkdbfield("Betrag", "FLOAT DEFAULT '0' NULL"))));
    Buchung.Fields.append(((dkdbfield("Datum", "DATE  NULL"))));
    Buchung.Fields.append(((dkdbfield("Bemerkung", "TEXT  NULL"))));
    dbstructure.Tables.append(Buchung);

    dkdbtable Zinse("DKZinssaetze");
    Zinse.Fields.append(dkdbfield("id","INTEGER DEFAULT '1' NOT NULL PRIMARY KEY AUTOINCREMENT"));
    Zinse.Fields.append(dkdbfield("Zinssatz","FLOAT DEFAULT '0,0' UNIQUE NULL"));
    Zinse.Fields.append(dkdbfield("Bemerkung","TEXT NULL"));
    dbstructure.Tables.append(Zinse);

}

bool createTables( const QSqlDatabase& db)
{
    QSqlQuery q(db);
    bool ret(true);
    for(dkdbtable table :dbstructure.Tables)
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
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filename);
    ret &= db.open();
    if( !ret) return ret;

    dbCloser c(&db);
    db.transaction();
    ret &= createTables(db);
    ret &= insertInterestRates(db);
    if( ret) db.commit(); else db.rollback();

    return ret;
}

bool hasAllTables(QSqlDatabase& db)
{
    QSqlQuery sqlp("SELECT * FROM DKPersonen", db);
    if( !sqlp.exec())
        return false;
    QSqlQuery sqlb("SELECT * FROM DKBuchungen", db);
    if( !sqlb.exec())
        return false;
    QSqlQuery sqlz("SELECT * FROM DKZinssaetze", db);
    if( !sqlz.exec())
        return false;
    return true;

}

bool isValidDb(const QString& filename)
{
    if( filename == "") return false;
    if( !QFile::exists(filename)) return false;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filename);
    if( !db.open())
        return false;

    dbCloser c(&db);
    if( !hasAllTables(db))
        return false;

    return true;
}
