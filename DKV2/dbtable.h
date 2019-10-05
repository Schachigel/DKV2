#ifndef DBTABLE_H
#define DBTABLE_H

#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlField>
#include <QList>

#include "dbfield.h"

struct dbtable{
    QString name;
    QList<dbfield> fields;
    dbtable append(dbfield);
    dbtable(QString n) : name(n)
    {}
    dbfield operator[](QString s);
    QString CreateTableSQL();
};

struct TableDataInserter
{
    TableDataInserter(const dbtable& t);
    QString InsertRecordSQL();
    void setValue(const QString& field, const QVariant& v);
    bool InsertData(QSqlDatabase db);
private:
    QString tablename;
    QSqlRecord record;
};

#endif // DBTABLE_H
