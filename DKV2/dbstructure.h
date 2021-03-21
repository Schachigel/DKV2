#ifndef DBSTRUCTURE_H
#define DBSTRUCTURE_H

#include <QSqlDatabase>
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
    dbstructure appendTable(const dbtable& t);
    bool createDb(const QString& dbFileName) const;
    bool createDb(const QSqlDatabase& db= QSqlDatabase::database()) const;

private:
    QVector<dbtable> Tables;
};

// THE structure of our database the single source of truth
void init_DKDBStruct();
//
bool createFileWithDatabaseStructure (const QString& targetfn, const dbstructure& dbs =dkdbstructur);
//
bool createNewDatabaseFileWDefaultContent(const QString& filename, const dbstructure& dbs =dkdbstructur);
// compare current structure to an actual database
bool hasAllTablesAndFields(const QSqlDatabase& db, const dbstructure& dbs =dkdbstructur);
// check schema
bool validateDbSchema(const QString& filename, const dbstructure& dbs =dkdbstructur);

#endif // DBSTRUCTURE_H
