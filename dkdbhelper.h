#ifndef DKDBHELPER_H
#define DKDBHELPER_H
#include <QSqlDatabase>
#include <qlist.h>
#include <qstring.h>

struct dkdbfield
{
    QString name;
    QString CreationSQL;
    dkdbfield(QString n, QString c)
    {
        name = n;
        CreationSQL = c;
    }
};

struct dkdbtable{
    QString Name;
    QList<dkdbfield> Fields;
    dkdbtable(QString n)
    {
        Name =n;
    }
    QString CreateTableSQL()
    {
        QString sql("CREATE TABLE [" + Name + "] (");
        for(dkdbfield Field : Fields)
        {
            sql.append("[" + Field.name + "] ");
            sql.append( Field.CreationSQL + ",");
        }
        sql.chop(1); // last comma
        sql.append(")");
        return sql;
    }
};

struct dkdbschema{
    QList<dkdbtable> Tables;
    QString checkTablesSql()
    {
        QString sql("SELECT * FROM ");
        for( dkdbtable table : Tables)
        {
            sql.append(table.Name);
            sql.append(",");
        }
        sql.chop(1);
        return sql;
    }
};


const QString sqls_createDkGeber("CREATE TABLE [DKGeber] ("
         "[id] INTEGER DEFAULT '1' NOT NULL PRIMARY KEY AUTOINCREMENT,"
         "[Vorname] TEXT  NOT NULL,"
         "[Nachname] TEXT  NOT NULL,"
         "[Strasse] TEXT  NOT NULL,"
         "[Plz] TEXT  NOT NULL,"
         "[Stadt] TEXT  NOT NULL,"
         "[IBAN] TEXT,"
         "[BIC] TEXT)");

const QString sqls_createDkVertrag("CREATE TABLE [DKVertrag] ("
        "[id] INTEGER DEFAULT '0' NOT NULL PRIMARY KEY AUTOINCREMENT,"
        "[DKGeberId] INTEGER FOREIGN_KEY REFERENCES [DKGeber](id),"
        "[Kennung] TEXT  NULL,"
        "[Betrag] FLOAT DEFAULT '0,0' NOT NULL,"
        "[Wert] FLOAT DEFAULT '0,0' NULL,"
        "[ZSatz] FLOAT DEFAULT '0,0' NOT NULL,"
        "[tesaurierend] BOOLEAN DEFAULT '1' NOT NULL,"
        "[Vertragsdatum] DATE  NULL,"
        "[aktiv] BOOLEAN DEFAULT 'false' NOT NULL,"
        "[LaufzeitEnde] DATE DEFAULT '3000-12-31' NOT NULL,"
        "[LetzteZinsberechnung] DATE  NULL)");

const QString sqls_createBuchungen("CREATE TABLE [Buchungen] ("
        "[id] INTEGER DEFAULT '0' NOT NULL PRIMARY KEY AUTOINCREMENT,"
        "[VertragId] INTEGER FOREIGN_KEY REFERENCES [DKVertrag](id),"
        "[Buchungsart] INTEGER DEFAULT '0' NOT NULL,"
        "[Betrag] FLOAT DEFAULT '0' NULL,"
        "[Datum] DATE  NULL,"
        "[Bemerkung] TEXT  NULL)");

const QString sqls_createZinssaetze("CREATE TABLE [DKZinssaetze] ("
        "[id] INTEGER DEFAULT '1' NOT NULL PRIMARY KEY AUTOINCREMENT,"
        "[Zinssatz] FLOAT DEFAULT '0,0' UNIQUE NULL,"
        "[Bemerkung] TEXT NULL)");

class dbCloser
{   // for use on the stack only
public:
    dbCloser (QSqlDatabase* d){Db=d;}
    dbCloser() = delete;
    ~dbCloser(){Db->close();}
private:
    QSqlDatabase* Db;
};

void initDbHelper();

bool createDKDB(const QString& filename);
bool isValidDb(const QString& filename);



#endif // DKDBHELPER_H
