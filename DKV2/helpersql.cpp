#include <iso646.h>

#include "helper.h"
#include "helperfin.h"
#include "helpersql.h"

#include "dbtable.h"

bool autoDetachDb::attachDb(const QString& filename)
{
    LOG_CALL;
    QString sql {qsl("ATTACH DATABASE '%1' AS '%2'")};
    return executeSql_wNoRecords(sql.arg(filename, alias()), QSqlDatabase::database(conname()));
}
autoDetachDb::~autoDetachDb()
{
    LOG_CALL;
    QString sql {qsl("DETACH DATABASE '%1'")};
    executeSql_wNoRecords(sql.arg(alias()), QSqlDatabase::database(conname()));
}

QString DbInsertableString(const QVariant& v)
{
    // there are more types here, than there are supported DB types, because the types
    // are input from external sources
    if( v.isNull() or not v.isValid()){
        qCritical() << "Data to be inserted is not valid:>" << v << "<";
        return qsl("''");
    }
    switch(v.type())
    {
    case QVariant::String:
    case QVariant::ByteArray:
    case QVariant::Char:
        return singleQuoted( v.toString().replace(qsl("'"), qsl("''")));
    case QVariant::Date: {
        return singleQuoted(v.toDate().toString(Qt::ISODate));
    }
    case QVariant::DateTime: {
        return singleQuoted(v.toDateTime().toString(qsl("yyyy-MM-dd hh:mm:ss")));
    }
    case QVariant::Double: {
        return singleQuoted(QString::number(v.toDouble(), 'f', 2));
    }
    case QVariant::Bool:
        return (v.toBool()) ? qsl("TRUE") : qsl("FALSE");;
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
        return i2s(v.toInt());
    default:
        qCritical() << "switch(v.type()) DEFAULTED " << v;
    }
    return "'" + v.toString () +"'";
}

QString dbCreatetable_type(const QVariant::Type t)
{   // these are the strings we use in data definition
    // so that they are expressive, but are still close
    // to the actual affinity data types
    switch( t)
    {
    case QVariant::String:
    case QVariant::ByteArray:
    case QVariant::Char:
        return qsl("TEXT"); // affinity: TEXT
    case QVariant::Date:
        return qsl("TEXTDATE"); // affinity: TEXT
    case QVariant::DateTime:
        return qsl("DATETIME"); // affinity: TEXT
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::UInt:
    case QVariant::ULongLong:
        return qsl("INTEGER"); // affinity: INTEGER
    case QVariant::Bool:
        return qsl("BOOLEAN");
    case QVariant::Double:
        return qsl("DOUBLE"); // affinity: REAL
    default:
        Q_ASSERT( not bool("invalid database type"));
        return qsl("INVALID");
    }
}
QString dbAffinityType(const QVariant::Type t)
{   // affinities are, what the database actually uses
    // as a "preferred" type of data stored in a column
    switch( t)
    {
    case QVariant::String:
    case QVariant::ByteArray:
    case QVariant::Char:
    case QVariant::Date:
    case QVariant::DateTime:
        return qsl("TEXT");
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::UInt:
    case QVariant::ULongLong:
    case QVariant::Bool:
        return qsl("INTEGER");
    case QVariant::Double:
        return qsl("REAL");
    default:
        Q_ASSERT( not bool("invalid database type"));
        return qsl("INVALID");
    }
}

// manage the app wide used database
void closeAllDatabaseConnections()
{   LOG_CALL;
    QList<QString> cl = QSqlDatabase::connectionNames();
    if( cl.count())
        qInfo() << "Found open connections" << cl;

    for( const auto &s : qAsConst(cl)) {
        QSqlDatabase::database(s).close();
        QSqlDatabase::removeDatabase(s);
    }
    cl.clear();
    cl = QSqlDatabase::connectionNames();
    if( cl.size())
        qInfo() << "not all connection to the database could be closed" << cl;
    qInfo() << "All Database connections were removed";
}

int rowCount(const QString& table, const QString& where, const QSqlDatabase& db /* =QSqlDatabase::database() */)
{
    QString sql {qsl("SELECT count(*) FROM [%1]").arg(table)};
    if( where.size())
        sql += qsl(" WHERE %1").arg(where);

    QSqlQuery q(sql, db);
    if( q.first())
        RETURN_OK( q.value(0).toInt(), qsl("rowCount %1").arg(q.value(0).toInt()), sql);
    else
        RETURN_ERR(-1, qsl("rowCount failed"), q.lastError ().text (), q.lastQuery ());
}

bool tableExists(const QString& tablename, const QSqlDatabase& db)
{
    return db.tables().contains(tablename);
}
bool VarTypes_share_DbType( const QVariant::Type t1, const QVariant::Type t2)
{
    return dbAffinityType(t1) == dbAffinityType(t2);
}
bool verifyTable(const dbtable& tableDef, const QSqlDatabase &db)
{
    QSqlRecord recordFromDb = db.record(tableDef.Name());
    if( recordFromDb.count() not_eq tableDef.Fields().count())
        RETURN_OK( false, QString(__FUNCTION__), tableDef.Name(), qsl("number of fields mismatch: %1 vs. %2")
               .arg( i2s(tableDef.Fields().count()), i2s(recordFromDb.count())));

    for( auto& field: tableDef.Fields()) {
        QSqlField FieldFromDb = recordFromDb.field(field.name());
        if( not FieldFromDb.isValid())
            RETURN_OK( false, QString(__FUNCTION__), qsl("failed: table exists but field is missing"), field.name());

        if( not VarTypes_share_DbType(field.type(), recordFromDb.field(field.name()).type()))
            RETURN_OK( false, QString(__FUNCTION__), qsl("failed: field "), field.name (), qsl(" type mismatch: %1 vs. %2")
                       .arg(field.type (), recordFromDb.field(field.name()).type()));
    }
    RETURN_OK( true, QString(__FUNCTION__), qsl("verified table %1").arg(tableDef.Name ()));
}
bool ensureTable( const dbtable& table,const QSqlDatabase& db)
{   LOG_CALL_W(table.Name());
    if( tableExists(table.Name(), db)) {
        return verifyTable(table, db);
    }
    // create table
    return table.create(db);
}

namespace {
void maintainFieldList( const QString& fieldname, const QString& tablename, QString& FieldList)
{
    if( FieldList.size())
        FieldList +=qsl(", ");
    FieldList +=qsl("%1.%2").arg( tablename, fieldname);
}
void maintainTableList (const QString& tablename, QString& TableList)
{
    if( TableList.size())
        TableList +=qsl(", ");
    TableList += tablename;
}
void sqlSnippetsFromFieldlist(const QVector<dbfield>& fields, QSet<QString>& usedTables, QString& FieldList, QString& TableList)
{
    for( const auto& f : qAsConst(fields)) {
        if( f.tableName().isEmpty() or f.name().isEmpty())
            qCritical().noquote () << QString(__FUNCTION__) << ": missing table or field name";
        maintainFieldList (f.name(), f.tableName (), FieldList);

        if( usedTables.contains(f.tableName()))
            continue;
        usedTables.insert(f.tableName());
        maintainTableList(f.tableName (), TableList);
    }
}

QSqlField adjustTypeOfField(const QSqlField& f, QVariant::Type t)
{
    if( f.value().type () == t) return f;
    QVariant tmpV = f.value();
    tmpV.convert(t); // adjust content of field to expected type
    if( f.value().isNull ()) {
        qInfo() << QString(__FUNCTION__) << "incoming value is NULL" << f;
    } else {
        if( tmpV.isNull ()){
            qCritical() << QString(__FUNCTION__) << "field conversion failed" << f << " -> " << tmpV;
            Q_ASSERT( "data field conversion should not fail");
        }
    }
    QSqlField ret (f);
    ret.setValue(tmpV); // make a new field containing the adjusted variant
    return ret;
}
QSqlRecord adjustSqlRecordTypesToDbFieldTypes(const QVector<dbfield>& fields, const QSqlRecord& record)
{
    if( fields.size () != record.count ())
        RETURN_ERR(QSqlRecord(), QString(__FUNCTION__), qsl("Anzahl der Felder stimmt nicht überein"));

    QSqlRecord result;
    for( const auto& field: qAsConst(fields))
        result.append(adjustTypeOfField(record.field(field.name()), field.type ()));

    return result;
}
QSqlQuery prepQuery( const QString sql, const QSqlDatabase& db =QSqlDatabase::database())
{
    QSqlQuery q(db);
    q.setForwardOnly (true);
    if( q.prepare (sql))
        RETURN_OK( q, qsl("prepQuery"), qsl("Successfully prepared query: %1").arg(q.lastQuery ()));
    else
        RETURN_ERR(q, qsl("prepQuery"), qsl("Failed to prep Query:"), q.lastError ().text (), qsl("\n"), q.lastQuery ());
}
bool executeQuery( QSqlQuery& q, QVector<QSqlRecord>& records)
{
    if( q.exec()) {
        while(q.next())
            records.push_back(q.record());
        RETURN_OK( true, qsl("executeQuery"), qsl("Successfully returned %1 records").arg(records.count ()));
    } else
        RETURN_ERR( false, qsl("executeQuery"), qsl("Faild to execute Query"), q.lastError ().text (), qsl("\n"), q.lastQuery ());
}
bool bindNamedParams(QSqlQuery &q, const QVector<QPair<QString, QVariant>>& params)
{
    for( const QPair<QString, QVariant>& param: qAsConst (params)) {
        QString paramName =param.first;
        QVariant paramValue =param.second;
        if( paramName.isEmpty ())
            RETURN_ERR( false, QString(__FUNCTION__), qsl("Empty param name"));
        q.bindValue (paramName, paramValue);
    }
    if( params.size () not_eq q.boundValues ().size())
        RETURN_ERR(false, qsl("Not all parameters were consumed by the query (bound: %1, given: %2)").arg(i2s(q.boundValues ().size ()), i2s(params.size())));

    if( q.boundValues ().size())
    {
        qInfo() << "bound values: " << /*QStringList*/q.boundValues ();
        return true;
    }
    else
        RETURN_OK( true, qsl("no bindings"));
}
bool bindPositionalParams(QSqlQuery& q , const QVector<QVariant> params)
{
    for( const QVariant& param: qAsConst (params)) {
        if( param.isValid()) {
            q.addBindValue(param);
        } else
            RETURN_ERR(false, qsl("invalid sql parameter"), param.toString ());
    } // eo for
    if( q.boundValues ().size () not_eq params.size ())
        RETURN_ERR(false, QString(__FUNCTION__), qsl("not all parameters were consumed by the query"));
    if( q.boundValues ().size()){
        qInfo() << "bound values: " << /*QStringList*/q.boundValues ();
        return true;
    }
    else
        RETURN_OK(true, qsl("no bindings"));
}
} // namespace

// no parameters
bool executeSql(const QString& sql, QVector<QSqlRecord>& result, const QSqlDatabase& db /*= ...*/ )
{
    QSqlQuery q =prepQuery(sql, db);
    if( q.lastError ().type ()== QSqlError::NoError)
        return executeQuery(q, result);
    else
        return false;
}
// named parameters
bool executeSql(const QString& sql, const QVector<QPair<QString, QVariant>>& params, QVector<QSqlRecord>& result, const QSqlDatabase& db)
{
    QSqlQuery q =prepQuery(sql, db);
    return bindNamedParams (q, params) && executeQuery(q, result);
}
// positional parameters
bool executeSql(const QString& sql, const QVector<QVariant>& params, QVector<QSqlRecord>& result, const QSqlDatabase& db)
{
    QSqlQuery q =prepQuery(sql, db);
    return bindPositionalParams (q, params) && executeQuery (q, result);
}
//
QVector<QSqlRecord> executeSql(const QVector<dbfield>& fields, const QString& where, const QString& order, const QSqlDatabase& db)
{
    QString sql = selectQueryFromFields(fields, where, order);
    QVector<QSqlRecord> tmpResult;
    if( not executeSql( sql, tmpResult, db))
        RETURN_ERR(QVector<QSqlRecord>(), QString(__FUNCTION__), qsl("execSql failed"));
    QVector<QSqlRecord> result;
    for( const QSqlRecord& record: qAsConst(tmpResult))
        // adjust the database types to the expected types
        result.push_back(adjustSqlRecordTypesToDbFieldTypes (fields, record));

    RETURN_OK(result, qsl("executeSql returns %1 records").arg(result.size ()));
}

bool getForeignKeyHandlingStatus(const QString& alias, const QSqlDatabase& db)
{
    QString sql {qsl("PRAGMA %1FOREIGN_KEYS").arg( alias.size() ? qsl("%1.") : "")};
    return executeSingleValueSql (sql, db).toBool();
}
bool switchForeignKeyHandling(bool OnOff, const QString& alias, const QSqlDatabase& db )
{
    QString sql {qsl("PRAGMA %1FOREIGN_KEYS=%2").arg((alias.size() ? qsl("%1.").arg(alias) : ""),
                                                     (OnOff ? qsl("ON"):qsl("OFF")))};
    return executeSql_wNoRecords(sql, db);
}
bool switchForeignKeyHandling(bool OnOff, const QSqlDatabase& db )
{
    return switchForeignKeyHandling (OnOff, QString(), db);
}

QString selectQueryFromFields(const QVector<dbfield>& dbField, const QString& incomingWhere, const QString& order)
{   LOG_CALL;

    QString FieldSnippet;
    QString TablesSnippet;
    QSet<QString> usedTables;
    sqlSnippetsFromFieldlist(dbField, usedTables, FieldSnippet, TablesSnippet);

    const QString orderBy {order.size() ? qsl(" ORDER BY %1").arg(order) : QString()};
    const QString where =incomingWhere.size() ? qsl(" WHERE %1").arg(incomingWhere) : "";

    QString Query =qsl("SELECT %1 FROM %2 %3 %4")
            .arg(FieldSnippet, TablesSnippet, where, orderBy);
    RETURN_OK(Query, qsl("selectQueryFromFields created Query: %1").arg( Query));
}

QSqlRecord executeSingleRecordSql(const QString& sql, const QSqlDatabase& db)
{
    QVector<QSqlRecord> records;
    if( executeSql(sql, records, db)) {
        if( records.isEmpty() or records.size () > 1)
            return QSqlRecord(); // error handling in called function
        else
            RETURN_OK( records[0], QString(__FUNCTION__), qsl("returned one value: %1").arg(records[0].value (0).toString ()));
    }
    else
        return QSqlRecord(); // error handling in called function
}

QSqlRecord executeSingleRecordSql(const QVector<dbfield>& fields, const QString& where, const QString& order, const QSqlDatabase& db)
{
    QString sql =selectQueryFromFields(fields, where, order);
    return adjustSqlRecordTypesToDbFieldTypes (fields, executeSingleRecordSql (sql, db));
}

QVariant executeSingleValueSql(const QString& sql, const QSqlDatabase& db)
{
    QSqlRecord v =executeSingleRecordSql (sql, db);
    // error handling is done in executeSingleRecordSql
    if( v.isEmpty () or v.count () > 1)
        return QVariant();
    else if( v.value (0).isNull ())
        return QVariant();
    else
        return v.value(0);
}
QVariant executeSingleValueSql(const QString& sql, const QVector<QVariant> params, const QSqlDatabase& db /*=QSqlDatabase::database()*/)
{
    QVector<QSqlRecord> result;
    if( executeSql(sql, params, result, db))
    {
        if( result.count() > 1)
            RETURN_ERR( QVariant(), QString(__FUNCTION__), qsl("Query returned too many records"));
        if( result.isEmpty())
            RETURN_OK( QVariant(), QString(__FUNCTION__), qsl("Query returned no record"));
        RETURN_OK( result[0].value(0), QString(__FUNCTION__));
    }
    else
        RETURN_ERR( QVariant(), QString(__FUNCTION__), qsl("Query execution failed"));
}

QVariant executeSingleValueSql(const QString& fieldName, const QString& tableName, const QString& where, const QSqlDatabase& db)
{
    if( fieldName.isEmpty() or tableName.isEmpty ())
        RETURN_ERR(QVariant(), QString(__FUNCTION__), qsl("invalid field- or tablename %1, %2").arg( fieldName, tableName));
    QString sql = qsl("SELECT %1 FROM %2").arg( fieldName, tableName);
    if( where.size())
        sql += (qsl(" WHERE %1").arg(where));
    return executeSingleValueSql(sql, db);
}
QVariant executeSingleValueSql(const dbfield& field, const QString& where, const QSqlDatabase& db)
{
    if( field.name().isEmpty() or field.tableName().isEmpty())
        RETURN_ERR( QVariant(), QString(__FUNCTION__), qsl("invalid parameters"));

    QVariant result = executeSingleValueSql(field.name(), field.tableName(), where, db);

    if( not result.isValid())
        RETURN_OK( result, QString(__FUNCTION__), qsl("found no result"));

    if(result.convert(field.type()))
        RETURN_OK( result, QString(__FUNCTION__), qsl("single valid value found"));
    else
        RETURN_ERR( QVariant(), qsl("executeSingleValueSql(): variant type conversion failed"));
}

QVector<QVariant> executeSingleColumnSql( const dbfield& dbField, const QString& where)
{
    if( dbField.tableName().isEmpty() or dbField.name().isEmpty()) {
        Q_ASSERT("execluteSingleColumnSql failed w wrong parameters");
        RETURN_ERR( QVector<QVariant>(), qsl("incomplete dbfield"));
    }

    QString sql {qsl("SELECT %1 FROM %2").arg(dbField.name(), dbField.tableName())};
    if( not where.isEmpty()) sql += qsl(" WHERE %1").arg( where);

    QVector<QSqlRecord> queryReturn;
    if( not executeSql(sql, queryReturn))
        RETURN_ERR(QVector<QVariant>(), QString(__FUNCTION__), qsl("Query execution failed"));

    QVector<QVariant> result;
    for( const QSqlRecord& record: queryReturn)
            result.push_back(adjustTypeOfField(record.field(0), dbField.type()).value());
    return result;
}

bool executeSql_wNoRecords(const QString& sql, const QSqlDatabase& db)
{
    return executeSql_wNoRecords(sql, QVector<QVariant>(), db);
}
bool executeSql_wNoRecords(const QString& sql, const QVariant& param, const QSqlDatabase& db)
{
    if( param.isValid())
        return executeSql_wNoRecords(sql, QVector<QVariant>{param}, db);
    else
        return executeSql_wNoRecords(sql, QVector<QVariant>(), db);
}
bool executeSql_wNoRecords(const QString& sql, const QVector<QVariant>& params, const QSqlDatabase& db)
{
    QVector<QSqlRecord> result;
    return executeSql(sql, params, result, db);
}

int getHighestRowId(const QString& tablename)
{   LOG_CALL;
    return executeSingleValueSql(qsl("MAX(rowid)"), tablename).toInt();
}

bool deleteDbView(const QString& name, const QSqlDatabase& db)
{   LOG_CALL;
    if( executeSql_wNoRecords(qsl("DROP VIEW IF EXISTS %1").arg(name), db)) {
        qInfo() << "dropped view: " << name;
        return true;
    }
    RETURN_OK( false, "drop view failed: ", name);
}

bool createDbViews( const QMap<QString, QString>& views, const QSqlDatabase& db)
{
    foreach(QString view, views.keys()) {
        if( not createPersistentDbView (view, views[view], db))
            return false;
    }
    return true;
}
bool createIndicesFromSQL( const QStringList Sqls, const QSqlDatabase& db)
{   LOG_CALL;
    QVector<QSqlRecord> indices;
    executeSql (qsl("SELECT name from sqlite_master WHERE type=='index'"), indices, db);
    for( int i =0; i<indices.count (); i++) {
        QString tablename =indices[i].field(0).value().toString ();
        if( tablename.startsWith (qsl("sqlite"), Qt::CaseInsensitive))
            continue;
        else
            executeSql_wNoRecords (qsl("DROP INDEX [%1]").arg(tablename), db);
    }
    for(int i=0; i < Sqls.count (); i++) {
        if( not executeSql_wNoRecords (Sqls[i], db))
            return false;
    }
    return true;
}
