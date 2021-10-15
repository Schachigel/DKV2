#ifndef TABLEDATAINSERTER_H
#define TABLEDATAINSERTER_H
#include <iso646.h>

#include <QSqlDatabase>
#include <QSqlRecord>

#include "dbtable.h"

struct TableDataInserter
{
    // constr. destrc. & access fu
    //TableDataInserter(){}
    TableDataInserter(const dbtable& t);
    TableDataInserter(const QString& tname, const QSqlRecord& rec) : tablename(tname), record(rec){};
    void init(const dbtable& t);
    enum treatNull {allowNullValues, notAllowNullValues};
    bool setValue(const QString& field, const QVariant& v, treatNull allowNull =treatNull::notAllowNullValues);
    bool setValueNULL(const QString& field);
    bool setValues(const QSqlRecord &rec);
    bool updateValue(const QString& n, const QVariant& v, qlonglong index);
    QVariant getValue(const QString& f) const { return record.field(f).value();}
    QSqlRecord getRecord() const {return record;}
    // interface
    int WriteData(const QSqlDatabase &db = QSqlDatabase::database()) const;
    int InsertOrReplaceData(const QSqlDatabase &db = QSqlDatabase::database()) const;
    int InsertData_noAuto(const QSqlDatabase &db = QSqlDatabase::database()) const;
    int UpdateData() const;
    void reset() {tablename =QString(); record =QSqlRecord();}
    void overrideTablename(const QString& tn) {tablename =tn;};
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
//    friend inline bool operator==(const TableDataInserter& lhs, const TableDataInserter& rhs)
//    { /* do actual comparison */
//        if( lhs.tablename not_eq rhs.tablename){
//            qInfo() << "table name missmatch " << lhs.tablename << " vs. " << rhs.tablename;
//            return false;
//        }
//        if( lhs.record.count() not_eq rhs.record.count()) {
//            qInfo() << "Record count missmatch " << lhs.record << "vs. " << rhs.record;
//            return false;
//        }
//        bool bRet =true;
//        if( lhs.record not_eq rhs.record) {
//            qInfo() << "QSqlRecord mismatch: " << "\n" << lhs.record << "\nvs\n" << rhs.record;
//            bRet =false;
//        }
//        for( int i =0; i< lhs.record.count(); i++) {
//            if( lhs.record.field(i) not_eq rhs.record.field(i)) {
//                qInfo() << "QSqlField mismatch " << lhs.record.field(i) << rhs.record.field(i);
//                bRet =false;
//            }
//        }
//        return bRet;
//    }

};



#endif // TABLEDATAINSERTER_H
