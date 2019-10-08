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
class dbstructure;

class dbtable
{
    friend class dbstructure;
public:
    // constr. destr. & access fu
    dbtable(QString n="") : name(n) {}
    QString Name() const {return name;}
    void setName(QString n){name=n;}
    QList<dbfield> Fields() const { return fields;}
    dbfield operator[](QString s) const;
    // interface
    dbtable append(const dbfield&);
private:
    QString name;
    QList<dbfield> fields;
    // helper
    QString CreateTableSQL() const;
};

class TableDataInserter
{
public:
    // constr. destrc. & access fu
    TableDataInserter(const dbtable& t);
    void setValue(const QString& field, const QVariant& v);
    // interface
    bool InsertData(QSqlDatabase db =QSqlDatabase::database());
    int getInsertedRecordId() const {return lastRecord;}
private:
    // data
    QString tablename;
    QSqlRecord record;
    int lastRecord;
    // helper
    QString InsertRecordSQL() const;
};

#endif // DBTABLE_H
