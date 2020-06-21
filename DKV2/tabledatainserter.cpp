
#include "helper.h"
#include "helpersql.h"
#include "tabledatainserter.h"

TableDataInserter::TableDataInserter(const dbtable& t)
{
    init(t);
}

void TableDataInserter::init(const dbtable& t)
{   //LOG_CALL;
    tablename = t.Name();
    for (auto dbfield : t.Fields()) {
        QSqlField sqlField(dbfield.name(), dbfield.type(), tablename);
        if( dbfield.isAutoValue())
            sqlField.setAutoValue(true);
        record.append(sqlField);
    }
}

bool TableDataInserter::setValue(const QString& n, const QVariant& v)
{
    if( ! record.contains(n)) {
        qCritical() << "wrong field name for insertion " << n;
        return false;
    }
    if( record.field(n).type() == v.type()) {
//        qInfo() << "tableDataInserter setValue: " << n << " -> " << v ;
        record.setValue(n, v);
        return true;
    }
    qDebug() << "wrong field type for insertion -> converting" << v.type() << " -> " << record.field(n).type();
    QVariant vf (v);
    if( vf.convert(record.field(n).type())){
        record.setValue(n, vf);
        return true;
    }
    qCritical() << "type conversion failed, no data inserted " << n;
    return false;
}

bool TableDataInserter::setValues(const QSqlRecord input)
{   LOG_CALL;
    if( input.count() != record.count()) {
        qCritical() << "TableDataInserter setValues faild: wrong sqlRecord size (actual / expected): " << input.count() << " / " << record.count();
        return false;
    }
    for( int i=0; i< input.count(); i++) {
        if( ! setValue(input.fieldName(i), input.value(i))) {
            qDebug() << "setValues failed in " << input.fieldName(i);
            return false;
        }
    }
    return true;
}

QString TableDataInserter::getInsertRecordSQL() const
{//   LOG_CALL;
    if( record.isEmpty()) return QString();
    QString FieldList;
    QString ValueList;

    for( int i=0; i<record.count(); i++) {
        if( i!=0) {
            FieldList +=", ";
            ValueList +=", ";
        }
        FieldList += record.field(i).name();
        if( record.field(i).isAutoValue())
            ValueList += "NULL";
        else {
            ValueList += dbInsertableString(record.field(i).value());
        }
    }
    QString sql="INSERT INTO %1 (%2) VALUES (%3) ";
    sql = sql.arg(tablename).arg(FieldList).arg(ValueList);
    qDebug() << "insertRecordSql: " << sql;
    return sql;
}

QString TableDataInserter::getInsertOrReplaceRecordSQL() const
{   LOG_CALL;
    if( record.isEmpty()) return QString();
    QString sql("INSERT OR REPLACE INTO " + tablename +" (%1) VALUES (%2)");
    QString fieldnames, values;

    for( int i=0; i<record.count(); i++) {
        if( i>0) {
            fieldnames += ", ";
            values += ", ";
        }
        fieldnames +=record.fieldName(i);
        if( record.field(i).isAutoValue())
            values += "NULL";
        else
            values += dbInsertableString(record.field(i).value());
    }
    sql = sql.arg(fieldnames).arg(values);
    qDebug() << sql;
    return sql;
}

QString TableDataInserter::getUpdateRecordSQL() const
{   LOG_CALL;
    if( record.isEmpty()) return QString();
    QString sql("UPDATE " + tablename +" SET ");
    QString where(" WHERE ");

    bool firstField = true;
    for( int i=0; i<record.count(); i++) {
        if( ! firstField) sql += ", ";
        // WARN ! THIS will work with exactly 1 AutoValue
        if( record.field(i).isAutoValue())
            where += record.field(i).name() + "=" + record.field(i).value().toString();
        else {
            sql += record.field(i).name() + "=" + dbInsertableString(record.field(i).value());
            firstField = false;
        }
    }
    sql += where;
    return sql;
}

int TableDataInserter::InsertData(QSqlDatabase db) const
{   // LOG_CALL;
    if( record.isEmpty()) return false;

    QSqlQuery q(db);
    bool ret = q.exec(getInsertRecordSQL());
    qlonglong lastRecord = q.lastInsertId().toLongLong();
    if( !ret) {
        qCritical() << "Insert record failed: " << q.lastError() << endl << q.lastQuery() << endl;
        return -1;
    }
    qInfo() << "Successfully inserted Data into " << tablename << " at index " << q.lastInsertId().toLongLong() << endl <<  q.lastQuery() << endl;
    return lastRecord;
}

int TableDataInserter::InsertOrReplaceData(QSqlDatabase db) const
{   LOG_CALL;
    if( record.isEmpty()) return false;
    QSqlQuery q(db);
    bool ret = q.exec(getInsertOrReplaceRecordSQL());
    qlonglong lastRecord = q.lastInsertId().toLongLong();
    if( !ret) {
        qCritical() << "Insert/replace record failed: " << q.lastError() << endl << q.lastQuery() << endl;
        return -1;
    }
    qDebug() << "successfully inserted Data at index " << q.lastInsertId().toLongLong() << endl <<  q.lastQuery() << endl;
    return lastRecord;
}

int TableDataInserter::UpdateData() const
{
    if( record.isEmpty()) return false;
    QSqlQuery q;
    bool ret = q.exec(getUpdateRecordSQL());
    int lastRecord = q.lastInsertId().toInt();
    if( !ret) {
        qCritical() << "TDI.Update record failed: " << q.lastError() << endl << q.lastQuery() << endl;
        return -1;
    }
    qDebug() << "TDI.Update: successfull at index " << q.lastInsertId().toInt() << endl <<  q.lastQuery() << endl;
    return lastRecord;
}

