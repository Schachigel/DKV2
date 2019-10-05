#ifndef DBSTRUCTURE_H
#define DBSTRUCTURE_H

#include <QString>
#include <QList>

#include "dbtable.h"

struct dbstructure
{
    dbstructure appendTable(dbtable t);
    bool createDb(QSqlDatabase db);
    bool checkDb();

    QString checkTablesSql();
    dbtable operator[](const QString& name);
    QList<dbtable> getTables(){
        return Tables;
    }
private:
    QList<dbtable> Tables;

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

#endif // DBSTRUCTURE_H
