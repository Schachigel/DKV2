#ifndef DBSTRUCTURE_H
#define DBSTRUCTURE_H

#include "dbtable.h"

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
    // tests
    void clear() { Tables.clear();}
    QString toString();
private:
    QVector<dbtable> Tables;
};
////////////////////////////////
inline dbstructure dkdbstructur;
////////////////////////////////

// THE structure of our database the single source of truth
void init_DKDBStruct();
void reInit_DKDBStruct();
//
bool createFileWithDatabaseStructure (const QString& targetfn, const dbstructure& dbs =dkdbstructur);
//
enum zinssusance {
    zs_30360,
    zs_actact
};



bool createNewDatabaseFileWDefaultContent(const QString& filename, const zinssusance sz =zs_30360, const dbstructure& dbs =dkdbstructur);
// compare current structure to an actual database
bool hasAllTablesAndFields(const QSqlDatabase& db, const dbstructure& dbs =dkdbstructur);
// check schema
bool validateDbSchema(const QString& filename, const dbstructure& dbs =dkdbstructur);

#endif // DBSTRUCTURE_H
