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
    QVector<dbfield> Fields() const { return fields;}
    dbfield operator[](QString s) const;
    // interface
    dbtable append(const dbfield&);
    void setUnique(const QVector<dbfield>& fs);
    bool create(QSqlDatabase& db) const;
private:
    QString name;
    QString unique;
    QVector<dbfield> fields;
    // helper
    QString createSQL() const;
};

class TableDataInserter
{
public:
    // constr. destrc. & access fu
    TableDataInserter(){}
    TableDataInserter(const dbtable& t);
    void init(const dbtable& t);
    void setValue(const QString& field, const QVariant& v);
    QVariant getValue(const QString& f) const { return record.field(f).value();}
    QSqlRecord getRecord() const {return record;}
    // interface
    int InsertData(     QSqlDatabase db =QSqlDatabase::database()) const;
    int InsertOrReplaceData(QSqlDatabase db =QSqlDatabase::database()) const;
    int UpdateData(     QSqlDatabase db =QSqlDatabase::database()) const;
    //int getInsertedRecordId() const {return lastRecord;}
private:
    // data
    QString tablename;
    QSqlRecord record;
    //int lastRecord;
    // helper
    QString getInsertRecordSQL() const;
    QString getInsertOrReplaceRecordSQL() const;
    QString getUpdateRecordSQL() const;
};

#endif // DBTABLE_H
