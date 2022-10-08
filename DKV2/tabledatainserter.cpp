
#include "helper.h"
#include "helpersql.h"
#include "tabledatainserter.h"

TableDataInserter::TableDataInserter(const dbtable& t)
{
    tablename = t.Name();
    for( int i =0; i < t.Fields ().count (); i++) {
        QSqlField f {t.Fields ().at (i)};
        record.append (f);
    }
}

bool TableDataInserter::setValue(const QString& n, const QVariant& v, treatNull allowNull)
{//   LOG_CALL_W(n +qsl(", ") +v.toString());
    if( not record.contains(n)) {
        qCritical() << "wrong field name for insertion " << n;
        return false;
    }
    if( allowNull and (v.isNull () or not v.isValid ())) {
        setValueToDefault(n);
        return true;
    }
    if( dbAffinityType(record.field(n).type()) == dbAffinityType(v.type())) {
        record.setValue(n, v);
        return true;
    }
    qInfo() << "TDI::setValue: Wrong field type for insertion -> converting" << v.type() << " -> " << record.field(n).type();
    QVariant vf (v);
    if( vf.convert(record.field(n).type())){
        record.setValue(n, vf);
        return true;
    }
    qCritical() << "TDI::setValue: type conversion failed, no data inserted " << n;
    return false;
}

bool TableDataInserter::setValueToDefault(const QString &n)
{
    if( record.contains(n)) {
        record.field(n).value () = record.field(n).defaultValue ();
        // qDebug() << "field set to default: " << record.field(n).name() << record.field(n).value ();
        return true;
    }
    qCritical() << "TDI::setValueToDefault: Wrong field name for insertion of default value" << n;
    return false;
}

bool TableDataInserter::setValues(const QSqlRecord &input)
{
    if( input.count() not_eq record.count()) {
        qCritical() << "TableDataInserter setValues faild: wrong sqlRecord size (actual / expected): " << input.count() << " / " << record.count();
        return false;
    }
    qInfo() << "Tdi: setting Values from QSqlRecord";
    for( int i=0; i< input.count(); i++) {
        if( setValue(input.fieldName(i), input.value(i))){
            qInfo() << "TDI::setValues: Value set: " << input.fieldName(i) << " : " << input.value(i);
            continue;
        }
        else {
            qDebug() << "TDI::setValues: setValues failed in " << input.fieldName(i);
            return false;
        }
    }
    return true;
}

bool TableDataInserter::updateValue(const QString& n, const QVariant& v, qlonglong index)
{   LOG_CALL_W(n);
    if( not v.isValid() || v.isNull()) {
        setValueToDefault(n);
    } else {
        setValue(n, v);
    }
    QString sql{qsl("UPDATE %1 SET %2=%3 ").arg( tablename, n, DbInsertableString(record.field(n).value()))};

    QString where {qsl(" WHERE %4=%5")};
    bool whereDone =false;
    for( int i=0; i<record.count(); i++) {
        if( record.field(i).isAutoValue()) {
            where =where.arg(record.field(i).name(), QString::number(index));
            whereDone =true;
            break;
        }
    }
    if( not whereDone) {
        qWarning() << "could not update value w/o index; trying 'id'";
        where =where.arg(qsl("id"), index);
    }
    return executeSql_wNoRecords(sql + where);
}

QString TableDataInserter::getInsertRecordSQL() const
{//   LOG_CALL;
    if( record.isEmpty()) return QString();
    QString FieldList;
    QString ValueList;

    for( int i=0; i<record.count(); i++) {
        if (record.field(i).isNull() and not record.field(i).isAutoValue()) {
            // skip this value so that the defaults from table def will be used
            continue;
        }
        if( FieldList.size()) {
            FieldList +=qsl(", ");
            ValueList +=qsl(", ");
        }
        FieldList += record.field(i).name();
        if( record.field(i).isAutoValue())
            ValueList += qsl("NULL");
        else if(record.field(i).isNull())
            ValueList += qsl("NULL");
        else {
            ValueList += DbInsertableString(record.field(i).value());
        }
    }
    QString sql =qsl("INSERT INTO %1 (%2) VALUES (%3) ");
    sql = sql.arg(tablename, FieldList, ValueList);
    qInfo() << "getInsertRecordSQL: " << sql;
    return sql;
}
qlonglong TableDataInserter::InsertRecord(const QSqlDatabase &db) const
{   // LOG_CALL;
    if( record.isEmpty()) return -1;

    QSqlQuery q(db);
    bool ret = q.exec(getInsertRecordSQL());
    qlonglong lastRecord = q.lastInsertId().toLongLong();
    if( ret) {
        qInfo() << "TDI::WriteData: Successfully inserted Data into " << tablename << " at index " << q.lastInsertId().toLongLong()<< qsl(" (InsertData)")  << "\n" <<  q.lastQuery() << qsl("\n");
        return lastRecord;
    }
    qCritical() << "TDI::WriteData: Insert record failed: " << q.lastError() << "\n" << q.lastQuery() << qsl("\n");
    return -1;
}

QString TableDataInserter::getInsert_noAuto_RecordSQL() const
{//   LOG_CALL;
    if( record.isEmpty()) return QString();
    QString fieldnames, values;
    for( int i=0; i<record.count(); i++) {
        if( fieldnames.size()) {
            fieldnames += qsl(", ");
            values += qsl(", ");
        }
        fieldnames +=record.fieldName(i);
        if( record.field(i).isNull())
            values += qsl("NULL");
        else
            values += DbInsertableString(record.field(i).value());
    }
    QString sql(qsl("INSERT OR REPLACE INTO %1 (%2) VALUES (%3)"));
    sql = sql.arg(tablename, fieldnames, values);
    qInfo() << "getInsert_noAuto_RecordSQL: " << sql;
    return sql;
}
qlonglong TableDataInserter::InsertData_noAuto(const QSqlDatabase &db) const
{
    if( record.isEmpty()) return false;
    QSqlQuery q(db);
    bool ret = q.exec(getInsert_noAuto_RecordSQL());
    qlonglong lastRecord = q.lastInsertId().toLongLong();
    if( ret) {
        qDebug() << "InsertData_noAuto: successfully inserted Data at index " << q.lastInsertId().toLongLong() << qsl(" (InsertData_noAuto)")  << "\n" <<  q.lastQuery() <<  qsl("\n");
        return lastRecord;
    }
    qCritical() << "InsertData_noAuto: Insert/replace record failed: " << q.lastError() << "\n" << q.lastQuery() << qsl("\n");
    return -1;
}

QString TableDataInserter::getInsertOrReplaceRecordSQL() const
{//   LOG_CALL;
    if( record.isEmpty()) return QString();
    QString sql(qsl("INSERT OR REPLACE INTO ") + tablename +qsl(" (%1) VALUES (%2)"));
    QString fieldnames, values;

    for( int i=0; i<record.count(); i++) {
        if( fieldnames.size()) {
            fieldnames += qsl(", ");
            values += qsl(", ");
        }
        fieldnames +=record.fieldName(i);
        if( record.field(i).isAutoValue() or record.field(i).isNull())
            values += qsl("NULL");
        else
            values += DbInsertableString(record.field(i).value());
    }
    sql = sql.arg(fieldnames, values);
    qInfo() << "getInsertOrReplaceRecordSQL: " << sql;
    return sql;
}
qlonglong TableDataInserter::InsertOrReplaceData(const QSqlDatabase &db) const
{//   LOG_CALL;
    if( record.isEmpty()) return false;
    QSqlQuery q(db);
    bool ret = q.exec(getInsertOrReplaceRecordSQL());
    qlonglong lastRecord = q.lastInsertId().toLongLong();
    if( ret) {
        qDebug() << "InsertOrReplaceData: Successfully inserted Data at index " << q.lastInsertId().toLongLong() << "\n" <<  q.lastQuery() << qsl("\n");
        return lastRecord;
    }
    qCritical() << "InsertOrReplaceData: Insert/replace record failed: " << q.lastError() << "\n" << q.lastQuery() << qsl("\n");
    return -1;
}

QString TableDataInserter::getUpdateRecordSQL(qlonglong& OUT_index) const
{//    LOG_CALL;
    if( record.isEmpty()) return QString();
    QString sql(qsl("UPDATE %1 SET ").arg(tablename));
    QString where(qsl(" WHERE "));

    bool firstField = true;
    for( int i=0; i<record.count(); i++) {
        // WARN ! THIS will work with exactly 1 AutoValue. if it is missing ...
        if( record.field(i).isAutoValue()) {
            where += record.field(i).name() + qsl("=") + record.field(i).value().toString();
            OUT_index =record.field(i).value().toLongLong();
            continue;
        }
        if( firstField) firstField = false; else sql += qsl(", ");
        if( record.field(i).isNull()) {
            if( record.field(i).defaultValue().isNull ())
                sql +=  record.field(i).name() +qsl("=NULL");
            else
                sql += record.field(i).name() +qsl("= %1").arg(DbInsertableString (record.field (i).defaultValue ()));
        }
        else {
            sql += record.field(i).name() + qsl("=") + DbInsertableString(record.field(i).value());
        }
    }
    sql += where;
    qInfo() << "getUpdateRecordSQL: " << sql;
    return sql;
}
qlonglong TableDataInserter::UpdateRecord() const
{
    if( record.isEmpty()) return false;
    QSqlQuery q;
    qlonglong changedRecordId =0;
    bool ret = q.exec(getUpdateRecordSQL(changedRecordId));
    if( ret) {
        qDebug() << "TDI.UpdateData: successfull at index " << changedRecordId << "\n" <<  q.lastQuery() << qsl("\n");
        return changedRecordId;
    }
    qCritical() << "TDI.UpdateData: update record failed: " << q.lastError() << "\n" << q.lastQuery() << qsl("\n");
    return -1;
}
