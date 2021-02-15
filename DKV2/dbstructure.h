#ifndef DBSTRUCTURE_H
#define DBSTRUCTURE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVector>

#include "dbtable.h"

extern dbstructure dkdbstructur;

class dbstructure
{
public:
    // constr., destr. & access fu
    dbtable operator[](const QString& name) const;
    QVector<dbtable> getTables() const { return Tables;}
    // interface
    dbstructure appendTable(dbtable t);
    bool createDb(QString dbFileName) const;
    bool createDb(QSqlDatabase db= QSqlDatabase::database()) const;

private:
    QVector<dbtable> Tables;
};

// THE structure of our database the single source of truth
void init_DKDBStruct();
//
bool createFileWithDkDatabaseStructure (QString targetfn);
//
bool createNew_DKDatabaseFile(const QString& filename);
// compare current structure to an actual database
bool hasAllTablesAndFields(QSqlDatabase db);
// check schema
bool validDbSchema(QSqlDatabase db =QSqlDatabase::database());
bool validDbSchema(const QString& filename);




#endif // DBSTRUCTURE_H
