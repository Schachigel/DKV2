#ifndef DKDBHELPER_H
#define DKDBHELPER_H
#include <qdebug.h>
#include <QSqlDatabase>
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qsqlrecord.h>
#include <qsqlfield.h>
#include <qvariant.h>
#include <qdatetime.h>
#include <qlist.h>
#include <qstring.h>

QString dbTypeFromVariant(QVariant::Type t);
struct dbtable;

struct dbfield
{
    dbtable* table;
    QString name;
    QVariant::Type VType;
    QString TypeInfo;
    dbfield(dbtable& parent, QString n, QVariant::Type t=QVariant::String, QString ti="") :
        table(&parent), name(n), VType(t), TypeInfo(ti)
    {}

    QString CreateFieldSQL()
    {
        return "[" + name + "] " + dbTypeFromVariant(VType) + " " +TypeInfo;
    }
    QSqlField getQSqlField()
    {
        return QSqlField(name, VType);
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
        for( int i = 0; i< Fields.count(); i++)
        {
            if( i>0) sql.append(", ");
            sql.append(Fields[i].CreateFieldSQL());
        }
        sql.append(")");
        return sql;
    }
    QSqlField getQSqlFieldByName(QString name)
    {
        for( auto field : Fields)
        {
            if( field.name == name)
            {
                QSqlField f(field.getQSqlField());
                f.setTableName(Name);
                return f;
            }
        }
        return QSqlField();
    }
};

struct dbstructure{
    QList<dbtable> Tables;
    QString checkTablesSql()
    {
        QString sql("SELECT * FROM ");
        for( int i=0; i<Tables.count(); i++)
        {
            if( i>0) sql.append(", ");
            sql.append(Tables[i].Name);
        }
        return sql;
    }
    dbtable getTable(QString name)
    {
        for( dbtable table : Tables)
        {
            if( table.Name == name)
                return table;
        }
        return dbtable("");
    }
};

extern dbstructure dkdbstructure;

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
void closeDbConnection();
void openAppDefaultDb( QString newDbFile="");

void createSampleDkDatabaseData();

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
int savePersonDataToDb(const PersonData& p);

typedef QPair<int, QString> PersonDispStringWithId;
void AllPersonsForSelection(QList<PersonDispStringWithId>& Entries);

typedef QPair<int, QString> ZinsDispStringWithId;
void AllInterestRatesForSelection(QList<ZinsDispStringWithId>& Entries);

struct VertragsDaten
{
    int id;
    int KreditorId;
    QString Kennung;
    float Betrag;
    float Wert;
    float Zins;
    bool tesaurierend;
    bool active;
    QDate Vertragsdatum;
    QDate LaufzeitEnde;
    QDate StartZinsberechnung;
    VertragsDaten();
};
bool verbucheVertrag(const VertragsDaten& c);
bool activateContract( int ContractId, QDate activationDate);

#endif // DKDBHELPER_H
