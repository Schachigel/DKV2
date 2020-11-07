#ifndef TABLEDATAINSERTER_H
#define TABLEDATAINSERTER_H

#include <QSqlDatabase>
#include <QSqlRecord>

#include "dbtable.h"

struct TableDataInserter
{
    // constr. destrc. & access fu
    //TableDataInserter(){}
    TableDataInserter(const dbtable& t);
    void init(const dbtable& t);
    bool setValue(const QString& field, const QVariant& v);
    bool setValueNULL(const QString& field);
    bool setValues(const QSqlRecord rec);
    QVariant getValue(const QString& f) const { return record.field(f).value();}
    QSqlRecord getRecord() const {return record;}
    // interface
    int InsertData(QSqlDatabase db = QSqlDatabase::database()) const;
    int InsertOrReplaceData(QSqlDatabase db = QSqlDatabase::database()) const;
    int InsertData_noAuto(QSqlDatabase db = QSqlDatabase::database()) const;
    int UpdateData() const;
    void reset() {tablename =QString(); record =QSqlRecord();}
private:
    // data
    QString tablename;
    QSqlRecord record;
    //int lastRecord;
    // helper
    QString getInsertRecordSQL() const;
    QString getInsertOrReplaceRecordSQL() const;
    QString getInsert_noAuto_RecordSQL() const;
    QString getUpdateRecordSQL(qlonglong& autovalue) const;
public:
    friend inline bool operator==(const TableDataInserter& lhs, const TableDataInserter& rhs)
    { /* do actual comparison */
        if( lhs.tablename != rhs.tablename){
            qInfo() << "table name missmatch " << lhs.tablename << " vs. " << rhs.tablename;
            return false;
        }
        if( lhs.record.count() != rhs.record.count()) {
            qInfo() << "Record count missmatch " << lhs.record << "vs. " << rhs.record;
            return false;
        }
        bool bRet =true;
        if( lhs.record != rhs.record) {
            qInfo() << "QSqlRecord mismatch: " << "\n" << lhs.record << "\nvs\n" << rhs.record;
            bRet =false;
        }
        for( int i =0; i< lhs.record.count(); i++) {
            if( lhs.record.field(i) != rhs.record.field(i)) {
                qInfo() << "QSqlField mismatch " << lhs.record.field(i) << rhs.record.field(i);
                bRet =false;
            }
        }
        return bRet;
    }

};



#endif // TABLEDATAINSERTER_H
