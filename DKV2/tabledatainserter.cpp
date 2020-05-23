
#include "helper.h"
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

void TableDataInserter::setValue(const QString& n, const QVariant& v)
{   // LOG_CALL_W(n);
    qInfo() << "tableDataInserter setValue: " << n << " -> " << v ;
    if( n.isEmpty()) return;
    if( record.contains(n)) {
        if( record.field(n).type() == v.type())
            record.setValue(n, v);
        else {
            qDebug() << "wrong field type for insertion -> converting" << v.type() << " -> " << record.field(n).type();
            QVariant vf (v); vf.convert(record.field(n).type());
            record.setValue(n, vf);
        }
    }
    else
        qCritical() << "wrong field name for insertion " << n;
}

void TableDataInserter::setValues(const QSqlRecord input)
{   LOG_CALL;
    if( input.count() != record.count()) {
        qCritical() << "TableDataInserter setValues faild: wrong sqlRecord size (actual / expected): " << input.count() << " / " << record.count();
        return;
    }
    // check compatibility

}

QString TableDataInserter::getInsertRecordSQL() const
{//   LOG_CALL;
    if( record.isEmpty()) return QString();
    QString FieldList;
    QString ValueList;

    for( int i=0; i<record.count(); i++) {
        if( i==0)
            FieldList = ValueList = "(";
        else {
            FieldList +=", ";
            ValueList +=", ";
        }
        FieldList += record.field(i).name();
        if( record.field(i).isAutoValue())
            ValueList += "NULL";
        else {
            ValueList += dbInsertableStringFromVariant(record.field(i).value());
        }
    }
    QString sql("INSERT INTO " + tablename + " ");
    sql += FieldList +  ") VALUES ";
    sql += ValueList + ")";
    qDebug() << "insertRecordSql: " << sql;
    return sql;
}

QString TableDataInserter::getInsertOrReplaceRecordSQL() const
{   LOG_CALL;
    if( record.isEmpty()) return QString();
    QString sql("INSERT OR REPLACE INTO " + tablename +" VALUES (");

    for( int i=0; i<record.count(); i++) {
        if( i>0) sql += ", ";
        if( record.field(i).isAutoValue())
            sql += "NULL";
        else
            sql += dbInsertableStringFromVariant(record.field(i).value());
    }
    sql +=")";
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
        // WARN ! THIS will not work with multiple AutoValues
        if( record.field(i).isAutoValue())
            where += record.field(i).name() + " = " + record.field(i).value().toString();
        else {
            sql += record.field(i).name() + " = " + dbInsertableStringFromVariant(record.field(i).value());
            firstField = false;
        }
    }
    sql += where;
    return sql;
}

int TableDataInserter::InsertData(QSqlDatabase db) const
{   // LOG_CALL;
    if( record.isEmpty()) return false;
    QString insertSql = getInsertRecordSQL();
    QSqlQuery q(db);
    bool ret = q.exec(insertSql);
    qlonglong lastRecord = q.lastInsertId().toLongLong();
    if( !ret) {
        qCritical() << "Insert record failed: " << q.lastError() << endl << q.lastQuery() << endl;
        return -1;
    }
    qInfo() << "Successfully inserted Data into " << tablename << " at index " << q.lastInsertId().toLongLong() << endl <<  q.lastQuery() << endl;
    return lastRecord;
}

int TableDataInserter::InsertOrReplaceData() const
{   LOG_CALL;
    if( record.isEmpty()) return false;
    QSqlQuery q;
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
{   LOG_CALL;
    if( record.isEmpty()) return false;
    QSqlQuery q;
    bool ret = q.exec(getUpdateRecordSQL());
    int lastRecord = q.lastInsertId().toInt();
    if( !ret) {
        qCritical() << "Update record failed: " << q.lastError() << endl << q.lastQuery() << endl;
        return -1;
    }
    qDebug() << "successfully updated Data at index " << q.lastInsertId().toInt() << endl <<  q.lastQuery() << endl;
    return lastRecord;
}

