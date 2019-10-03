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
    QString Name;
    QList<dbfield> Fields;
    dbtable(QString n) : Name(n)
    {}
    dbfield operator[](QString s);
    QString CreateTableSQL();
    QSqlField getQSqlFieldByName(QString);
};


#endif // DBTABLE_H
