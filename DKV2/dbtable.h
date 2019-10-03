#ifndef DBTABLE_H
#define DBTABLE_H

#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlField>
#include <QList>

#include "dbfield.h"

struct dbtable{
    QString name;
    QList<dbfield> fields;
    dbtable(QString n) : name(n)
    {}
    dbfield operator[](QString s);
    QString CreateTableSQL();
};


#endif // DBTABLE_H
