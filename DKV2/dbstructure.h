#ifndef DBSTRUCTURE_H
#define DBSTRUCTURE_H

#include <QString>
#include <QVector>
#include <QSqlDatabase>

#include "dbtable.h"

class dbstructure
{
public:
    // constr., destr. & access fu
    dbtable operator[](const QString& name) const;
    QVector<dbtable> getTables() const { return Tables;}
    // interface
    dbstructure appendTable(dbtable t);
    bool createDb(QSqlDatabase db= QSqlDatabase::database()) const;

private:
    QVector<dbtable> Tables;
};

class dbCloser
{   // for use on the stack only
public:
    dbCloser() : Db (nullptr){} // create 'closer' before the database
    ~dbCloser(){if( !Db) return; Db->close(); Db->removeDatabase(Db->connectionName());}
    void set(QSqlDatabase* p){ Db=p;}
private:
    QSqlDatabase* Db;
};

#endif // DBSTRUCTURE_H
