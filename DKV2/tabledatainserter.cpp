
#include "helper.h"
#include "sqlhelper.h"
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
        qInfo() << "tableDataInserter setValue: " << n << " -> " << v ;
        record.setValue(n, v);
        return true;
    }
    qDebug() << "wrong field type for insertion -> converting" << v.type() << " -> " << record.field(n).type();
    QVariant vf (v);
    if( vf.convert(record.field(n).type())){
        record.setValue(n, vf);
        return true;
    }
    qCritical() << "type conversion failed" << n;
    return false;
}

bool TableDataInserter::setValues(const QSqlRecord input)
{   LOG_CALL;
    if( input.count() != record.count()) {
        qCritical() << "TableDataInserter setValues faild: wrong sqlRecord size (actual / expected): " << input.count() << " / " << record.count();
        return false;
    }
    // check compatibility
    for( int i=0; i< input.count(); i++) {
        QSqlField inputf = input.field(i);
        QSqlField recordf = record.field(inputf.name());
        if( inputf.type() != recordf.type())
            return false;
    }

    for( int i=0; i< input.count(); i++) {
        setValue(input.fieldName(i), input.value(i));
    }
    return true;
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
            ValueList += dbInsertableString(record.field(i).value());
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
            sql += dbInsertableString(record.field(i).value());
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

