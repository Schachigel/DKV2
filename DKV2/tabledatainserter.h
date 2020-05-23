#ifndef TABLEDATAINSERTER_H
#define TABLEDATAINSERTER_H

#include <QSqlDatabase>

#include "dbtable.h"

class TableDataInserter
{
public:
    // constr. destrc. & access fu
    TableDataInserter(){}
    TableDataInserter(const dbtable& t);
    void init(const dbtable& t);
    void setValue(const QString& field, const QVariant& v);
    void setValues(const QSqlRecord rec);
    QVariant getValue(const QString& f) const { return record.field(f).value();}
    QSqlRecord getRecord() const {return record;}
    // interface
    int InsertData(QSqlDatabase db = QSqlDatabase::database()) const;
    int InsertOrReplaceData() const;
    int UpdateData() const;
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
#endif // TABLEDATAINSERTER_H
