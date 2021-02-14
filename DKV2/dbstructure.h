#ifndef DBSTRUCTURE_H
#define DBSTRUCTURE_H

#include <QSqlDatabase>
#include <QString>
#include <QVector>

#include "dbtable.h"

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

#endif // DBSTRUCTURE_H
