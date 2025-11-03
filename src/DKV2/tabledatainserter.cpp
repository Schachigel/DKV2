
#include "helper.h"
#include "helperfin.h"
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
{
    if( not record.contains(n))
        RETURN_ERR(false, qsl("wrong field name for insertion %1").arg(n));
    if( allowNull and (v.isNull () or not v.isValid ())) {
        setValueToDefault(n);
        return true;
    }
    if( dbAffinityType(record.field(n).metaType()) == dbAffinityType(v.metaType())) {
        record.setValue(n, v);
        return true;
    }
    qInfo() << "TDI::setValue: Wrong field type for insertion -> converting" << v.metaType().name() << " -> " << record.field(n).metaType().name();
    QVariant vf (v);
    if( vf.convert(record.field(n).metaType())) {
        record.setValue(n, vf);
        return true;
    }
    RETURN_ERR( false, qsl("TDI::setValue: type conversion failed, no data inserted ").arg(n));
}

bool TableDataInserter::setValueToDefault(const QString &n)
{
    if( record.contains(n)) {
        QVariant defaultvalue =record.field(n).defaultValue ();
        record.field(n).value () = defaultvalue;
        if( defaultvalue.isNull ())
            RETURN_OK(true, qsl("field value set to default (null)"), n, qsl("/"), defaultvalue.toString ());
        else
            RETURN_OK(true, qsl("field value set to default:"), n, qsl("/"), defaultvalue.toString ());
    }
    RETURN_ERR(false, qsl("TDI::setValueToDefault: Wrong field name for insertion of default value"), n);
}

bool TableDataInserter::setValues(const QSqlRecord &input)
{
    if( input.count() not_eq record.count())
        RETURN_ERR( false, qsl("TableDataInserter setValues faild: wrong sqlRecord size (actual / expected): %1 / %2").arg(i2s(input.count ()),i2s(record.count())) );
    qInfo() << "Tdi: setting Values from QSqlRecord";

    for( int i=0; i< input.count(); i++) {
        if( setValue(input.fieldName(i), input.value(i))) {
            qInfo() << "TDI::setValues: Value set: " << input.fieldName(i) << " : " << input.value(i);
            continue;
        }
        else
            RETURN_ERR(false, qsl("TDI::setValues: setValues failed in "), input.fieldName(i));
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

    QString where { qsl(" WHERE %4=%5")};
    bool whereDone =false;
    for( int i=0; i<record.count(); i++) {
        if( record.field(i).isAutoValue()) {
            where =where.arg(record.field(i).name(), i2s(index));
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
        else
            ValueList += DbInsertableString(record.field(i).value());
    }
    QString sql =qsl("INSERT INTO %1 (%2) VALUES (%3) ");
    sql = sql.arg(tablename, FieldList, ValueList);
    RETURN_OK( sql, qsl("getInsertRecordSQL: "), sql);
}

tableindex_t TableDataInserter::InsertRecord(const QSqlDatabase &db) const
{   // LOG_CALL;
    if( record.isEmpty())
        RETURN_ERR( SQLITE_invalidRowId, qsl("tdi has an empty record"));

    QSqlQuery q(db);
    bool ret = q.exec(getInsertRecordSQL());
    tableindex_t lastRecord = q.lastInsertId().toLongLong();
    if( ret)
        RETURN_OK( lastRecord, qsl("TDI::WriteData: Successfully inserted Data into %1 at rowid %2").arg( tablename, i2s(q.lastInsertId().toLongLong())));

    RETURN_ERR(SQLITE_invalidRowId, qsl("TDI::WriteData: Insert record failed: "), q.lastError ().text (), q.lastQuery ());
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
    RETURN_OK( sql, qsl("getInsert_noAuto_RecordSQL: "), sql);
}
tableindex_t TableDataInserter::InsertData_noAuto(const QSqlDatabase &db) const
{
    if( record.isEmpty()) return false;
    QSqlQuery q(db);
    bool ret = q.exec(getInsert_noAuto_RecordSQL());
    tableindex_t lastRecord = q.lastInsertId().toLongLong();
    if( ret)
        RETURN_OK( lastRecord, qsl("InsertData_noAuto: successfully inserted Data at index %1").arg(q.lastInsertId().toLongLong()));
    RETURN_ERR( SQLITE_invalidRowId, qsl("InsertData_noAuto: Insert/replace record failed: "), q.lastError ().text (), qsl("\n"), q.lastQuery ());
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
    RETURN_OK( sql, qsl("getInsertOrReplaceRecordSQL: "), sql);
}

tableindex_t TableDataInserter::InsertOrReplaceData(const QSqlDatabase &db) const
{
    if( record.isEmpty()) return false;
    QSqlQuery q(db);
    bool ret = q.exec(getInsertOrReplaceRecordSQL());
    tableindex_t lastRecord = q.lastInsertId().toLongLong();
    if( ret)
        RETURN_OK( lastRecord, qsl("InsertOrReplaceData: Successfully inserted Data at index %1").arg( q.lastInsertId().toLongLong()));

    RETURN_ERR( SQLITE_invalidRowId, qsl("InsertOrReplaceData: Insert/replace record failed: "), q.lastError ().text ());
}

QString TableDataInserter::getUpdateRecordSQL(tableindex_t& OUT_index) const
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
        else
            sql += record.field(i).name() + qsl("=") + DbInsertableString(record.field(i).value());
    }
    sql += where;
    RETURN_OK( sql, qsl("getUpdateRecordSQL: "), sql);
}

tableindex_t TableDataInserter::UpdateRecord() const
{
    if( record.isEmpty())
        return false;
    tableindex_t changedRecordId =0;
    bool ret = executeSql_wNoRecords (getUpdateRecordSQL(changedRecordId));
    if( ret)
        RETURN_OK( changedRecordId, qsl("TDI.UpdateData: successfull at index %1").arg(changedRecordId));
    RETURN_ERR( SQLITE_invalidRowId, qsl("TDI.UpdateData: update record failed"));
}
