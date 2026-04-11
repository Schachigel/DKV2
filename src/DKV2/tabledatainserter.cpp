#include "tabledatainserter.h"

#include "helper_core.h"
#include "helperfin.h"
#include "helpersql.h"

#include <optional>

namespace {
enum class InsertMode {
    insert,
    insertOrReplace
};

struct PreparedStatement
{
    QString sql;
    QVector<QVariant> params;
};

QString quotedIdentifier(const QString& name)
{
    const QStringList parts = name.split(qsl("."));
    QStringList quotedParts;
    quotedParts.reserve(parts.size());
    for( const QString& part : parts)
        quotedParts.append(qsl("[%1]").arg(part));
    return quotedParts.join(qsl("."));
}

QVariant typedNull(const QSqlField& field)
{
    return QVariant(field.metaType());
}

std::optional<QSqlQuery> prepareWriteQuery(const QString& sql, const QSqlDatabase& db)
{
    QSqlQuery q(db);
    q.setForwardOnly(true);
    if( q.prepare(sql))
        return q;

    qCritical() << qsl("TableDataInserter prepare failed:") << q.lastError().text() << q.lastQuery();
    return std::nullopt;
}

bool bindWriteParams(QSqlQuery& q, const QVector<QVariant>& params)
{
    for( const QVariant& param : std::as_const(params))
        q.addBindValue(param);

    if( q.boundValues().size() not_eq params.size())
        RETURN_ERR(false, qsl("TableDataInserter bind failed: not all parameters were consumed"));

    if( not params.isEmpty())
        qInfo() << "bound values: " << q.boundValues();
    return true;
}

QVariant defaultedFieldValue(const QSqlField& field)
{
    if( field.value().isValid() and not field.value().isNull())
        return field.value();

    if( field.defaultValue().isValid() and not field.defaultValue().isNull())
        return field.defaultValue();

    return typedNull(field);
}

std::optional<PreparedStatement> buildInsertStatement(const QString& tableName, const QSqlRecord& record,
                                                      InsertMode mode, bool skipNullNonAutoFields,
                                                      bool forceAutoFieldsToNull)
{
    QStringList fieldNames;
    QStringList placeholders;
    QVector<QVariant> params;

    for( int i =0; i < record.count(); i++) {
        const QSqlField field = record.field(i);
        const bool isNullField = (not field.value().isValid()) or field.value().isNull();

        if( skipNullNonAutoFields and isNullField and not field.isAutoValue())
            continue;

        fieldNames.append(quotedIdentifier(field.name()));
        placeholders.append(qsl("?"));

        if( forceAutoFieldsToNull and field.isAutoValue())
            params.push_back(typedNull(field));
        else if( isNullField)
            params.push_back(typedNull(field));
        else
            params.push_back(field.value());
    }

    if( fieldNames.isEmpty())
        return std::nullopt;

    PreparedStatement statement;
    statement.sql = qsl("%1 INTO %2 (%3) VALUES (%4)")
                        .arg(mode == InsertMode::insert ? qsl("INSERT") : qsl("INSERT OR REPLACE"),
                             quotedIdentifier(tableName),
                             fieldNames.join(qsl(", ")),
                             placeholders.join(qsl(", ")));
    statement.params = params;
    return statement;
}

std::optional<QPair<QString, QVariant>> updateKeyField(const QSqlRecord& record)
{
    for( int i =0; i < record.count(); i++) {
        const QSqlField field =record.field(i);
        if( field.isAutoValue())
            return QPair<QString, QVariant>{field.name(), field.value()};
    }
    if( record.contains(qsl("id")))
        return QPair<QString, QVariant>{qsl("id"), record.value(qsl("id"))};
    return std::nullopt;
}
}

TableDataInserter::TableDataInserter(const dbtable& t)
{
    tablename = t.Name();
    for( int i =0; i < t.Fields ().count (); i++) {
        QSqlField f {t.Fields ().at (i)};
        record.append (f);
    }
}

bool TableDataInserter::setValue(const QString& n, const QVariant& v, NullPolicy nullPolicy)
{
    if( not record.contains(n))
        RETURN_ERR(false, qsl("wrong field name for insertion %1").arg(n));
    if( v.isNull () or not v.isValid ()) {
        if( nullPolicy == NullPolicy::useDefaultForNull)
            setValueToDefault(n);
        else
            record.setValue(n, QVariant());
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
        record.setValue(n, defaultvalue);
        if( defaultvalue.isNull ())
            RETURN_OK(true, qsl("field value set to default (null)"), n, qsl("/"), defaultvalue.toString ());
        else
            RETURN_OK(true, qsl("field value set to default:"), n, qsl("/"), defaultvalue.toString ());
    }
    RETURN_ERR(false, qsl("TDI::setValueToDefault: Wrong field name for insertion of default value"), n);
}

bool TableDataInserter::setValues(const QSqlRecord &input)
{
    qInfo() << "Tdi: setting Values from QSqlRecord";

    // Reset missing fields to their schema defaults so partial/legacy records remain usable.
    for( int i =0; i < record.count(); i++) {
        setValueToDefault(record.fieldName(i));
    }

    for( int i=0; i< input.count(); i++) {
        if( not record.contains(input.fieldName(i)))
            RETURN_ERR(false, qsl("TDI::setValues: unexpected field in input record"), input.fieldName(i));
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
    QString keyFieldName {qsl("id")};
    for( int i=0; i<record.count(); i++) {
        if( record.field(i).isAutoValue()) {
            keyFieldName = record.field(i).name();
            break;
        }
    }
    const QString sql {qsl("UPDATE %1 SET %2=? WHERE %3=?")
                           .arg(quotedIdentifier(tablename),
                                quotedIdentifier(n),
                                quotedIdentifier(keyFieldName))};
    return executeSql_wNoRecords(sql, {defaultedFieldValue(record.field(n)), index});
}

tableindex_t TableDataInserter::InsertRecord(const QSqlDatabase &db) const
{   // LOG_CALL;
    if( record.isEmpty())
        RETURN_ERR( SQLITE_invalidRowId, qsl("tdi has an empty record"));

    auto statement = buildInsertStatement(tablename, record, InsertMode::insert, true, true);
    if( not statement)
        RETURN_ERR(SQLITE_invalidRowId, qsl("TDI::WriteData: could not build insert statement"));

    auto oq = prepareWriteQuery(statement->sql, db);
    if( not oq)
        RETURN_ERR(SQLITE_invalidRowId, qsl("TDI::WriteData: prepare insert failed"));

    QSqlQuery q = std::move(oq.value());
    if( not bindWriteParams(q, statement->params))
        RETURN_ERR(SQLITE_invalidRowId, qsl("TDI::WriteData: bind insert parameters failed"));

    bool ret = q.exec();
    tableindex_t lastRecord = q.lastInsertId().toLongLong();
    if( ret)
        RETURN_OK( lastRecord, qsl("TDI::WriteData: Successfully inserted Data into %1 at rowid %2").arg( tablename, i2s(q.lastInsertId().toLongLong())));

    RETURN_ERR(SQLITE_invalidRowId, qsl("TDI::WriteData: Insert record failed: "), q.lastError ().text (), q.lastQuery ());
}

tableindex_t TableDataInserter::InsertData_noAuto(const QSqlDatabase &db) const
{
    if( record.isEmpty())
        RETURN_ERR(SQLITE_invalidRowId, qsl("InsertData_noAuto: empty record"));

    auto statement = buildInsertStatement(tablename, record, InsertMode::insertOrReplace, false, false);
    if( not statement)
        RETURN_ERR(SQLITE_invalidRowId, qsl("InsertData_noAuto: could not build insert statement"));

    auto oq = prepareWriteQuery(statement->sql, db);
    if( not oq)
        RETURN_ERR(SQLITE_invalidRowId, qsl("InsertData_noAuto: prepare failed"));

    QSqlQuery q = std::move(oq.value());
    if( not bindWriteParams(q, statement->params))
        RETURN_ERR(SQLITE_invalidRowId, qsl("InsertData_noAuto: bind failed"));

    bool ret = q.exec();
    tableindex_t lastRecord = q.lastInsertId().toLongLong();
    if( ret)
        RETURN_OK( lastRecord, qsl("InsertData_noAuto: successfully inserted Data at index %1").arg(q.lastInsertId().toLongLong()));
    RETURN_ERR( SQLITE_invalidRowId, qsl("InsertData_noAuto: Insert/replace record failed: "), q.lastError ().text (), qsl("\n"), q.lastQuery ());
}

tableindex_t TableDataInserter::InsertOrReplaceData(const QSqlDatabase &db) const
{
    if( record.isEmpty())
        RETURN_ERR(SQLITE_invalidRowId, qsl("InsertOrReplaceData: empty record"));

    auto statement = buildInsertStatement(tablename, record, InsertMode::insertOrReplace, false, true);
    if( not statement)
        RETURN_ERR(SQLITE_invalidRowId, qsl("InsertOrReplaceData: could not build insert statement"));

    auto oq = prepareWriteQuery(statement->sql, db);
    if( not oq)
        RETURN_ERR(SQLITE_invalidRowId, qsl("InsertOrReplaceData: prepare failed"));

    QSqlQuery q = std::move(oq.value());
    if( not bindWriteParams(q, statement->params))
        RETURN_ERR(SQLITE_invalidRowId, qsl("InsertOrReplaceData: bind failed"));

    bool ret = q.exec();
    tableindex_t lastRecord = q.lastInsertId().toLongLong();
    if( ret)
        RETURN_OK( lastRecord, qsl("InsertOrReplaceData: Successfully inserted Data at index %1").arg( q.lastInsertId().toLongLong()));

    RETURN_ERR( SQLITE_invalidRowId, qsl("InsertOrReplaceData: Insert/replace record failed: "), q.lastError ().text ());
}

tableindex_t TableDataInserter::UpdateRecord() const
{
    if( record.isEmpty())
        RETURN_ERR(SQLITE_invalidRowId, qsl("TDI.UpdateData: empty record"));

    auto keyField = updateKeyField(record);
    if( not keyField)
        RETURN_ERR(SQLITE_invalidRowId, qsl("TDI.UpdateData: could not determine row selector"));

    QStringList assignments;
    QVector<QVariant> params;
    for( int i=0; i<record.count(); i++) {
        const QSqlField field =record.field(i);
        if( field.name() == keyField->first)
            continue;

        assignments.append(qsl("%1=?").arg(quotedIdentifier(field.name())));
        params.push_back(defaultedFieldValue(field));
    }
    if( assignments.isEmpty())
        RETURN_ERR(SQLITE_invalidRowId, qsl("TDI.UpdateData: no fields to update"));

    params.push_back(keyField->second);
    const QString sql {qsl("UPDATE %1 SET %2 WHERE %3=?")
                           .arg(quotedIdentifier(tablename),
                                assignments.join(qsl(", ")),
                                quotedIdentifier(keyField->first))};

    bool ret = executeSql_wNoRecords(sql, params);
    tableindex_t changedRecordId = keyField->second.toLongLong();
    if( ret)
        RETURN_OK( changedRecordId, qsl("TDI.UpdateData: successfull at index %1").arg(changedRecordId));
    RETURN_ERR( SQLITE_invalidRowId, qsl("TDI.UpdateData: update record failed"));
}
