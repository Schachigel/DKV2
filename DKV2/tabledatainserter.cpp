
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
    for (auto& dbfield : t.Fields()) {
        QSqlField sqlField(dbfield.name(), dbfield.type(), tablename);
        if( dbfield.isAutoValue())
            sqlField.setAutoValue(true);
        record.append(sqlField);
    }
}

bool TableDataInserter::setValue(const QString& n, const QVariant& v, treatNull allowNull)
{//   LOG_CALL_W(n +qsl(", ") +v.toString());
    if( not record.contains(n)) {
        qCritical() << "wrong field name for insertion " << n;
        return false;
    }
    if( allowNull and v.toString()==qsl("")) {
        setValueNULL(n);
        return true;
    }
    if( record.field(n).type() == v.type()) {
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

bool TableDataInserter::setValueNULL(const QString &n)
{
    if( record.contains(n)) {
        record.setNull(n);
        return true;
    }
    qCritical() << "wrong field name for insertion " << n;
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
            qInfo() << "value set: " << input.fieldName(i) << " : " << input.value(i);
            continue;
        }
        else {
            qDebug() << "setValues failed in " << input.fieldName(i);
            return false;
        }
    }
    return true;
}

bool TableDataInserter::updateValue(const QString& n, const QVariant& v, qlonglong index)
{   LOG_CALL_W(n);
    if( not v.isValid() || v.isNull()) {
        setValueNULL(n);
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
    qDebug() << "getInsertRecordSQL: " << sql;
    return sql;
}

QString TableDataInserter::getInsertOrReplaceRecordSQL() const
{   LOG_CALL;
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
    qDebug() << sql;
    return sql;
}

QString TableDataInserter::getInsert_noAuto_RecordSQL() const
{   LOG_CALL;
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
    qDebug() << sql;
    return sql;
}

QString TableDataInserter::getUpdateRecordSQL(qlonglong& autovalue) const
{   LOG_CALL;
    if( record.isEmpty()) return QString();
    QString sql(qsl("UPDATE ") + tablename +qsl(" SET "));
    QString where(qsl(" WHERE "));

    bool firstField = true;
    for( int i=0; i<record.count(); i++) {
        if( not firstField) sql += qsl(", ");
        if( record.field(i).isNull()) {
            sql += record.field(i).name() +qsl("=NULL");
            firstField = false;
        }
        // WARN ! THIS will work with exactly 1 AutoValue. if it is missing ...
        else if( record.field(i).isAutoValue()) {
            where += record.field(i).name() + qsl("=") + record.field(i).value().toString();
            autovalue =record.field(i).value().toLongLong();
        } else {
            sql += record.field(i).name() + qsl("=") + DbInsertableString(record.field(i).value());
            firstField = false;
        }
    }
    sql += where;
    return sql;
}

int TableDataInserter::WriteData(const QSqlDatabase &db) const
{   // LOG_CALL;
    if( record.isEmpty()) return false;

    QSqlQuery q(db);
    bool ret = q.exec(getInsertRecordSQL());
    qlonglong lastRecord = q.lastInsertId().toLongLong();
    if( ret) {
        qInfo() << "Successfully inserted Data into " << tablename << " at index " << q.lastInsertId().toLongLong()<< qsl(" (InsertData)")  << Qt::endl <<  q.lastQuery() << Qt::endl;
        return lastRecord;
    }
    qCritical() << "Insert record failed: " << q.lastError() << Qt::endl << q.lastQuery() << Qt::endl;
    return -1;
}

int TableDataInserter::InsertData_noAuto(const QSqlDatabase &db) const
{
    if( record.isEmpty()) return false;
    QSqlQuery q(db);
    bool ret = q.exec(getInsert_noAuto_RecordSQL());
    qlonglong lastRecord = q.lastInsertId().toLongLong();
    if( ret) {
        qDebug() << "successfully inserted Data at index " << q.lastInsertId().toLongLong() << qsl(" (InsertData_noAuto)")  << Qt::endl <<  q.lastQuery() <<  Qt::endl;
        return lastRecord;
    }
    qCritical() << "Insert/replace record failed: " << q.lastError() << Qt::endl << q.lastQuery() << Qt::endl;
    return -1;
}

int TableDataInserter::InsertOrReplaceData(const QSqlDatabase &db) const
{   LOG_CALL;
    if( record.isEmpty()) return false;
    QSqlQuery q(db);
    bool ret = q.exec(getInsertOrReplaceRecordSQL());
    qlonglong lastRecord = q.lastInsertId().toLongLong();
    if( ret) {
        qDebug() << "successfully inserted Data at index " << q.lastInsertId().toLongLong() << Qt::endl <<  q.lastQuery() << Qt::endl;
        return lastRecord;
    }
    qCritical() << "Insert/replace record failed: " << q.lastError() << Qt::endl << q.lastQuery() << Qt::endl;
    return -1;
}

int TableDataInserter::UpdateData() const
{
    if( record.isEmpty()) return false;
    QSqlQuery q;
    qlonglong changedRecordId =0;
    bool ret = q.exec(getUpdateRecordSQL(changedRecordId));
    if( ret) {
        qDebug() << "TDI.Update: successfull at index " << changedRecordId << Qt::endl <<  q.lastQuery() << Qt::endl;
        return changedRecordId;
    }
    qCritical() << "TDI.Update record failed: " << q.lastError() << Qt::endl << q.lastQuery() << Qt::endl;
    return -1;
}
