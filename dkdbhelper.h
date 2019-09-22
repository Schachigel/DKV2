#ifndef DKDBHELPER_H
#define DKDBHELPER_H
#include <QSqlDatabase>
#include <qlist.h>
#include <qstring.h>

struct dbfield
{
    QString name;
    QString CreationSQL;
    dbfield(QString n, QString c)
    {
        name = n;
        CreationSQL = c;
    }
};

struct dbtable{
    QString Name;
    QList<dbfield> Fields;
    dbtable(QString n)
    {
        Name =n;
    }
    QString CreateTableSQL()
    {
        QString sql("CREATE TABLE [" + Name + "] (");
        for(dbfield Field : Fields)
        {
            sql.append("[" + Field.name + "] ");
            sql.append( Field.CreationSQL + ",");
        }
        sql.chop(1); // last comma
        sql.append(")");
        return sql;
    }
};

struct dbstructure{
    QList<dbtable> Tables;
    QString checkTablesSql()
    {
        QString sql("SELECT * FROM ");
        for( dbtable table : Tables)
        {
            sql.append(table.Name);
            sql.append(",");
        }
        sql.chop(1);
        return sql;
    }
};

class dbCloser
{   // for use on the stack only
public:
    dbCloser() : Db (nullptr){} // create closer before the database
    ~dbCloser(){if( !Db) return; Db->close(); Db->removeDatabase(Db->connectionName());}
    void set(QSqlDatabase* p){ Db=p;}
private:
    QSqlDatabase* Db;
};

void initDbHelper();

bool createDKDB(const QString& filename);
bool isValidDb(const QString& filename);

struct PersonData
{
    int id;
    QString Vorname;
    QString Nachname;
    QString Strasse;
    QString Plz;
    QString Stadt;
    QString Iban;
    QString Bic;
};

void closeDbConnection();
void openAppDefaultDb( QString newDbFile="");
bool savePersonDataToDatabase(const PersonData& p);

typedef QPair<int, QString> PersonDispStringWithId;
void AllPersonsForSelection(QList<PersonDispStringWithId>& Entries);

#endif // DKDBHELPER_H
