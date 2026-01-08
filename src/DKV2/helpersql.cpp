
#include "helpersql.h"
#include <iso646.h>

#include "helper.h"
#include "helperfin.h"

#include "dbstructure.h"
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
    switch(v.metaType().id())
    {
    case QMetaType::QString:
    case QMetaType::QByteArray:
    case QMetaType::Char:
        return singleQuoted( v.toString().replace(qsl("'"), qsl("''")));
    case QMetaType::QDate: {
        return singleQuoted(v.toDate().toString(Qt::ISODate));
    }
    case QMetaType::QDateTime: {
        return singleQuoted(v.toDateTime().toString(qsl("yyyy-MM-dd hh:mm:ss")));
    }
    case QMetaType::Double: {
        return singleQuoted(QString::number(v.toDouble(), 'f', 2));
    }
    case QMetaType::Bool:
        return (v.toBool()) ? qsl("TRUE") : qsl("FALSE");;
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
        return i2s(v.toInt());
    default:
        qCritical() << "switch(v.type()) DEFAULTED " << v;
    }
    return "'" + v.toString () +"'";
}

QString dbCreatetable_type(const QMetaType t)
{   // these are the strings we use in data definition
    // so that they are expressive, but are still close
    // to the actual affinity data types
    switch( t.id())
    {
    case QMetaType::QString:
    case QMetaType::QByteArray:
    case QMetaType::Char:
        return qsl("TEXT"); // affinity: TEXT
    case QMetaType::QDate:
        return qsl("TEXTDATE"); // affinity: TEXT
    case QMetaType::QDateTime:
        return qsl("DATETIME"); // affinity: TEXT
    case QMetaType::Int:
    case QMetaType::LongLong:
    case QMetaType::UInt:
    case QMetaType::ULongLong:
        return qsl("INTEGER"); // affinity: INTEGER
    case QMetaType::Bool:
        return qsl("BOOLEAN");
    case QMetaType::Double:
        return qsl("DOUBLE"); // affinity: REAL
    default:
        qCritical() << "invalid database type";
        Q_UNREACHABLE();
    }
}
QString dbAffinityType(const QMetaType t)
{   // affinities are, what the database actually uses
    // as a "preferred" type of data stored in a column
    switch( t.id())
    {
    case QMetaType::QString:
    case QMetaType::QByteArray:
    case QMetaType::Char:
    case QMetaType::QDate:
    case QMetaType::QDateTime:
        return qsl("TEXT");
    case QMetaType::Int:
    case QMetaType::LongLong:
    case QMetaType::UInt:
    case QMetaType::ULongLong:
    case QMetaType::Bool:
        return qsl("INTEGER");
    case QMetaType::Double:
        return qsl("REAL");
    default:
        qCritical() << "invalid database type";
        Q_UNREACHABLE();
    }
}

// manage the app wide used database
void closeAllDatabaseConnections()
{   LOG_CALL;
    QList<QString> cl = QSqlDatabase::connectionNames();
    if( cl.count())
        qInfo() << "Found open connections" << cl;

    for( const auto &s : std::as_const(cl)) {
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
bool VarTypes_share_DbType( const QMetaType t1, const QMetaType t2)
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

        if( not VarTypes_share_DbType(field.metaType(), recordFromDb.field(field.name()).metaType()))
            RETURN_OK( false, QString(__FUNCTION__), qsl("failed: field "), field.name (), qsl(" type mismatch: %1 vs. %2")
            .arg(field.metaType ().name(), recordFromDb.field(field.name()).metaType().name()));
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
//     for( const auto& f : std::as_const(fields)) {
    for( int i =0; i < fields.count(); i++) {
        if( fields[i].tableName().isEmpty() or fields[i].name().isEmpty())
            qCritical().noquote () << QString(__FUNCTION__) << ": missing table or field name";
        maintainFieldList (fields[i].name(), fields[i].tableName (), FieldList);

        if( usedTables.contains(fields[i].tableName()))
            continue;
        usedTables.insert(fields[i].tableName());
        maintainTableList(fields[i].tableName (), TableList);
    }
}

QSqlField adjustTypeOfField(const QSqlField& f, QMetaType t)
{
    if( f.value().metaType () == t) return f;

    if( f.value().isNull ()) {
        qInfo() << QString(__FUNCTION__) << "incoming value is NULL" << f;
        // NULL is valid, eg. AnlagenId is initially NULL
        return f;
    }
    QVariant tmpV(f.value());
    QMetaType expected(t);

    if( tmpV.canConvert(expected)) {
        tmpV.convert(expected); // adjust content of field to expected type
    };
    if( tmpV.isNull ()){
        qCritical() << QString(__FUNCTION__) << "field conversion failed" << f << " -> " << tmpV;
        Q_UNREACHABLE();
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
    for( const auto& field: std::as_const(fields))
        result.append(adjustTypeOfField(record.field(field.name()), field.metaType ()));

    return result;
}
std::optional<QSqlQuery> prepQuery( const QString sql, const QSqlDatabase& db =QSqlDatabase::database())
{
    QSqlQuery q(db);
    q.setForwardOnly (true);
    if( q.prepare (sql)){
        qInfo() << qsl("prepQuery") << qsl("Successfully prepared query:\n %1").arg(q.lastQuery());
        return q;
    }
    else
        qCritical() << qsl("prepQuery") << qsl("Failed to prep Query:") << q.lastError ().text () << qsl("\n") << q.lastQuery ();
    return std::nullopt; // moved
}
bool executeQuery( QSqlQuery& q, QVector<QSqlRecord>& records)
{
    if( q.exec()) {
        int nbrElements =q.numRowsAffected ();
        if(nbrElements > 0)
            qInfo() << __FUNCTION__ << qsl("affected %1 rows").arg(nbrElements);
        else
            qInfo() << __FUNCTION__ << qsl("affected no rows");
        records.reserve(nbrElements);
        while(q.next())
            records.push_back(q.record());
        RETURN_OK( true, qsl("executeQuery"), qsl("Successfully returned %1 records").arg(records.count ()));
    } else
        RETURN_ERR( false, qsl("executeQuery"), qsl("Faild to execute Query"), q.lastError ().text (), qsl("\n"), q.lastQuery ());
}
bool bindNamedParams(QSqlQuery &q, const QVector<QPair<QString, QVariant>>& params)
{
    for( const QPair<QString, QVariant>& param: std::as_const(params)) {
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
    for( const QVariant& param: std::as_const(params)) {
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
    auto oq =prepQuery(sql, db);
    if( oq){
        return executeQuery(oq.value(), result);
    }
    else
        return false;
}
// named parameters
bool executeSql(const QString& sql, const QVector<QPair<QString, QVariant>>& params, QVector<QSqlRecord>& result, const QSqlDatabase& db)
{
    auto oq =prepQuery(sql, db);
    if( oq){
        QSqlQuery q =std::move(oq.value());
        return bindNamedParams (q, params) && executeQuery(q, result);
    }
    return false;
}
// positional parameters
bool executeSql(const QString& sql, const QVector<QVariant>& params, QVector<QSqlRecord>& result, const QSqlDatabase& db)
{
    auto oq =prepQuery(sql, db);
    if( oq){
        QSqlQuery q =std::move(oq.value());
        return bindPositionalParams (q, params) && executeQuery (q, result);
    }
    return false;
}
//
QVector<QSqlRecord> executeSql(const QVector<dbfield>& fields, const QString& where, const QString& order, const QSqlDatabase& db)
{
    QString sql = selectQueryFromFields(fields, where, order);
    QVector<QSqlRecord> tmpResult;
    if( not executeSql( sql, tmpResult, db))
        RETURN_ERR(QVector<QSqlRecord>(), QString(__FUNCTION__), qsl("execSql failed"));
    QVector<QSqlRecord> result;
    for( const QSqlRecord& record: std::as_const(tmpResult))
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
    // NOT for "normal" SELECTs on dkv2 tables (normal = no manipulation like SUM, concatination, formatiing etc.
    QVector<QSqlRecord> records;
    if( executeSql(sql, records, db)) {
        if( records.isEmpty() or records.size () > 1)
            return QSqlRecord(); // error handling in called function
        else{
            RETURN_OK( records[0], QString(__FUNCTION__), qsl("returned one Record: %1").arg(records[0].value (0).toString ()));
        }
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
{   // this function will preserv the type defined in our dbstructure if possible
    if( field.name().isEmpty() or field.tableName().isEmpty())
        RETURN_ERR( QVariant(), QString(__FUNCTION__), qsl("invalid parameters"));

    // set the type to what was defined in dkdbstructure
    QMetaType mt =dkdbstructur[field.tableName()][field.name()].metaType();

    Q_ASSERT(mt != QMetaType(QMetaType::UnknownType));
    QVariant result = executeSingleValueSql(field.name(), field.tableName(), where, db);
    if( not result.isValid())
        RETURN_OK( result, QString(__FUNCTION__), qsl("found no result"));

    if(result.convert(mt))
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
    for( const QSqlRecord& record: std::as_const(queryReturn))
            result.push_back(adjustTypeOfField(record.field(0), dbField.metaType()).value());
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

bool createDbIndex( const QString& iName, const QString& iFields, const QSqlDatabase& db)
{
    // sample arguments: qsl("Buchungen_vId"), qsl("'Buchungen'   ( 'VertragsId')")}
    if( not executeSql_wNoRecords (qsl("DROP INDEX IF EXISTS '%1'").arg(iName), db))
        RETURN_ERR(false, qsl("createDbIndex: failed to delete index"));
    if( not executeSql_wNoRecords (qsl("CREATE INDEX '%1' ON %2").arg(iName, iFields), db))
        RETURN_ERR(false, qsl("createDbIndex: failed to create index"), iName, iFields);
    RETURN_OK( true, qsl("successfully created index"), iName);
}
