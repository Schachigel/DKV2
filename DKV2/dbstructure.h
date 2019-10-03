#ifndef DBSTRUCTURE_H
#define DBSTRUCTURE_H

#include <QString>
#include <QList>

#include "dbtable.h"

struct dbstructure
{
    void addTable(const dbtable& t);
    QString checkTablesSql();
    dbtable operator[](QString name);
    QList<dbtable> getTables(){
        return Tables;
    }
private:
    QList<dbtable> Tables;

};

#endif // DBSTRUCTURE_H
