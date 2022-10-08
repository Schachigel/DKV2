#include <iso646.h>

#include "helper.h"
#include "dbtable.h"
#include "helpersql.h"


const QString dbTypeName {qsl("QSQLITE")};
const int SQLITE_minimalRowId =1;

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
        return QString::number(v.toInt());
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

int rowCount(const QString& table, const QSqlDatabase& db /* =QSqlDatabase::database() */)
{
    QString sql;
    if( table.startsWith(qsl("SELECT"), Qt::CaseInsensitive))
        sql =qsl("SELECT count(*) FROM (%1)").arg(table);
    else
        sql =qsl("SELECT count(*) FROM ") +table;

    QSqlQuery q(sql, db);
    if( q.first()){
       int ret =q.value(0).toInt();
       qDebug() << sql << " returned " << ret;
       return ret;
    }
    else {
        qCritical() << "row counting query failed" << q.lastError() << "\n" << q.lastQuery();
        return -1;
    }
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
    if( recordFromDb.count() not_eq tableDef.Fields().count()) {
        qDebug() << "verifyTable(" << tableDef.Name() <<  ") failed: number of fields mismatch. expected / actual: " << tableDef.Fields().count() << " / " << recordFromDb.count();
        return false;
    }
    for( auto& field: tableDef.Fields()) {
        QSqlField FieldFromDb = recordFromDb.field(field.name());
        if( not FieldFromDb.isValid()) {
            qDebug() << "verifyTable() failed: table exists but field is missing" << field.name();
            return false;
        }
        if( not VarTypes_share_DbType(field.type(), recordFromDb.field(field.name()).type()))
        {
            qDebug() << "ensureTable() failed: field " << field.name() <<
                        " type mismatch. expected / actual: " << field.type() << " / " << recordFromDb.field(field.name()).type();
            return false;
        }
    }
    return true;
}
bool ensureTable( const dbtable& table,const QSqlDatabase& db)
{   LOG_CALL_W(table.Name());
    if( tableExists(table.Name(), db)) {
        return verifyTable(table, db);
    }
    // create table
    return table.create(db);
}

bool switchForeignKeyHandling(const QSqlDatabase& db, const QString& alias, bool OnOff)
{
    Q_ASSERT( alias.size());
    QString sql {qsl("PRAGMA %1.FOREIGN_KEYS = %2")};
    sql =sql.arg(alias, OnOff?qsl("ON"):qsl("OFF"));
    return executeSql_wNoRecords(sql, db);
}
bool switchForeignKeyHandling(const QSqlDatabase& db /*def. DB*/, bool OnOff /*=true*/)
{
    if( OnOff)
        return executeSql_wNoRecords(qsl("PRAGMA FOREIGN_KEYS = ON"), db);
    else
        return executeSql_wNoRecords(qsl("PRAGMA FOREIGN_KEYS = OFF"), db);
}

QVariant executeSingleValueSql(const QString& sql, const QSqlDatabase& db)
{
    QSqlQuery q(db);
    if( not q.exec(sql)) {
        qCritical() << "SingleValueSql failed to execute: " << q.lastError() << "\n" << q.lastQuery();
        return QVariant();
    }
    q.last();
    if(q.at() > 0) {
        qDebug() << "SingleValueSql returned more than one value\n" << q.lastQuery();
        return QVariant();
    }
    if(q.at() < 0) {
        // qDebug() << "SingleValueSql returned no value\n" << q.lastQuery() << qsl("\n");;
        return QVariant();
    }
    qInfo() << "sql " << sql << " returned " << q.value(0);
    return q.value(0);
}
QVariant executeSingleValueSql(const QString& fieldName, const QString& tableName, const QString& where, const QSqlDatabase& db)
{
    QString sql = qsl("SELECT ") + fieldName + qsl(" FROM ") + tableName + (where.isEmpty() ? qsl("") : (qsl(" WHERE ") + where));
    return executeSingleValueSql(sql, db);
}
QVariant executeSingleValueSql(const dbfield& field, const QString& where, const QSqlDatabase& db)
{
    if( field.name().isEmpty() or field.tableName().isEmpty())
        return QVariant();
    QVariant result = executeSingleValueSql(field.name(), field.tableName(), where, db);

    if( not result.isValid()) {
//        qDebug() << "executeSingleValueSql found no value";
        return result;
    }
    if(result.convert(field.type()))
        return result;
    else {
        qCritical() << "executeSingleValueSql(): variant type conversion failed";
        return QVariant();
    }
}

QString selectQueryFromFields(const QVector<dbfield>& fields, const QVector<dbForeignKey>& keys, const QString& incomingWhere, const QString& order)
{   LOG_CALL;

    QString FieldList;
    QString TableList;
    QString calculatedWhere;
    QSet<QString> usedTables;

    for( const auto& f : qAsConst(fields)) {
        // qDebug() << f;
        if( f.tableName().isEmpty() or f.name().isEmpty())
            qCritical() << "selectQueryFromFields: missing table or field name";
        if( FieldList.size())
            FieldList +=qsl(", ");
        FieldList +=f.tableName() +qsl(".") +f.name();

        if( not usedTables.contains(f.tableName())) {
            usedTables.insert(f.tableName());
            if( TableList.size())
                TableList +=qsl(", ");
            TableList += f.tableName();
        }
    }
    for( auto key: keys) {
        if( calculatedWhere.size()) calculatedWhere += qsl(" AND ");
        calculatedWhere += key.get_SelectSqlWhereClause();
    }
    QString Where =qsl("%1 AND %2");
    Where = Where.arg((incomingWhere.isEmpty() ? qsl("true") : incomingWhere), (calculatedWhere.isEmpty() ? qsl("true") : calculatedWhere));

    QString Query =qsl("SELECT %1 FROM %2 WHERE %3");
    Query = Query.arg(FieldList, TableList, Where);
    if( order.size())
        Query = Query +qsl(" ORDER BY ") +order;
    qInfo() << "selectQueryFromFields created Query: " << Query;
    return Query;
}
QSqlField adjustedType(const QSqlField& f, QVariant::Type t)
{
    QVariant tmpV = f.value();
    tmpV.convert(t);
    QSqlField ret (f);
    ret.setValue(tmpV);
    return ret;
}

QVector<QVariant> executeSingleColumnSql( const dbfield& field, const QString& where)
{   LOG_CALL;
    if( field.tableName().isEmpty() or field.name().isEmpty()) {
        qCritical() << "incomplete dbfield";
        return QVector<QVariant>();
    }
    QString sql {qsl("SELECT %1 FROM %2 %3")};
    sql = sql.arg(field.name(), field.tableName(), (where.isEmpty() ? qsl("") : (qsl(" WHERE ") + where)));
    QSqlQuery q;
    QVector<QVariant> result;
    if( q.exec(sql)){
        qInfo() << "executed " << sql;
        while( q.next()) {
            result.push_back(adjustedType(q.record().field(0), field.type()).value());
        }
    }
    else
        qCritical() << "SingleColumnSql failed " << q.lastError() << "\n" << q.lastQuery();

    return result;
}
QSqlRecord executeSingleRecordSql(const QVector<dbfield>& fields, const QString& where, const QString& order)
{
    QString sql = selectQueryFromFields(fields, QVector<dbForeignKey>(), where, order);
    qDebug() << "ExecuteSingleRecordSql:\n" << sql;

    QSqlQuery q;
    if( not q.exec(sql)) {
        qCritical() << "SingleRecordSql failed " << q.lastError() << "\n" << q.lastQuery();
        return QSqlRecord();
    }
    q.last();
    if(q.at() not_eq 0) {
        qCritical() << "SingleRecordSql returned more then one value or non\n" << q.lastQuery();
        return QSqlRecord();
    }
    // adjust the database types to the expected types
    QSqlRecord result;
    for( auto& dbFieldEntry : qAsConst(fields)) {
        QSqlField tmpField =q.record().field(dbFieldEntry.name());
        result.append(adjustedType(tmpField,dbFieldEntry.type()));
    }
    return result;
}

QSqlRecord executeSingleRecordSql(const QString& sql)
{   LOG_CALL_W (sql);
    QSqlQuery q; q.setForwardOnly(true);
    q.prepare(sql);
    if( not (q.exec() and q.next())) {
        qCritical() << "failed to execute query /get first record" << q.lastError() << "\n" << q.lastQuery();
        return QSqlRecord();
    }
    QSqlRecord res =q.record();
    int count =1;
    while(q.next()) count++;
    if( count > 1) qCritical() << "single record query returned more than one value";
    return res;
}

QVector<QSqlRecord> executeSql(const QVector<dbfield>& fields, const QString& where, const QString& order)
{
    QString sql = selectQueryFromFields(fields, QVector<dbForeignKey>(), where, order);
    QVector<QSqlRecord> result;
    QSqlQuery q; q.setForwardOnly(true);
    if( not q.exec(sql)) {
        qCritical() << "executeSql failed " << q.lastError() << "\n" << q.lastQuery();
        return result;
    }
    while( q.next()) {
        // adjust the database types to the expected types
        QSqlRecord oneRecord;
        for( auto& dbFieldEntry : qAsConst(fields)) {
            // adjust to original variant data type
            QSqlField tmpField {adjustedType(q.record().field(dbFieldEntry.name()), dbFieldEntry.type())};
            oneRecord.append(tmpField);
        }
        result.push_back(oneRecord);
    }
    qInfo() << "executeSql returns " << result;
    return result;
}
bool executeSql(const QString& sql, const QVariant& v, QVector<QSqlRecord>& result, const QSqlDatabase& db /*= ...*/ )
{
    QSqlQuery q(db); q.setForwardOnly(true);
    if( not q.prepare(sql)) {
        qDebug() << "Faild to prep Query. Error:" << sql << q.lastError();
        return false;
    }
    if( v.isValid())
        q.bindValue(0, v);
    if( q.exec()) {
        while(q.next()) {
            result.push_back(q.record());
        }
        return true;
    } else {
        qDebug() << "Faild to execute Query. Error:" << q.lastError() << "\n" << q.lastQuery();
        return false;
    }
}
bool executeSql(const QString& sql, /*const QVector<QVariant>& v,*/ QVector<QSqlRecord>& result)
{
    QSqlQuery q; q.setForwardOnly(true);
    if( not q.prepare(sql)) {
        qDebug() << "Faild to prep Query. Error:" << q.lastError() << "\n" << q.lastQuery();
        return false;
    }
    if( q.exec()) {
        while(q.next()) {
            result.push_back(q.record());
        }
        return true;
    } else {
        qDebug() << "Faild to execute Query. Error:" << q.lastError() << q.lastQuery();
        return false;
    }
}

bool executeSql_wNoRecords(const QString& sql, const QSqlDatabase& db)
{
    return executeSql_wNoRecords(sql, QVector<QVariant>(), db);
}
bool executeSql_wNoRecords(const QString& sql, const QVariant& v, const QSqlDatabase& db)
{
    if( v.isValid())
        return executeSql_wNoRecords(sql, QVector<QVariant>{v}, db);
    else
        return executeSql_wNoRecords(sql, QVector<QVariant>(), db);
}
bool executeSql_wNoRecords(const QString& sql, const QVector<QVariant>& v, const QSqlDatabase& db)
{   LOG_CALL;
    QSqlQuery q(db); // q.setForwardOnly(true);
    if( not q.prepare(sql)) {
        qCritical() << "failed to prep query " << q.lastError() << "\n" << q.lastQuery();
        return false;
    }
    for( int i =0; i< v.count(); i++) {
        if( v[i].isValid()) {
            q.addBindValue(v[i]);
            qInfo() << "bound value " << v[i];
        } else {
            qCritical() << "Invalid sql parameter at index " << i;
            return false;
        }
    }
    if( q.exec()) {
        qInfo().noquote () << "Successfully executed query " << q.lastQuery() << " with "
        << (q.numRowsAffected() ? QString::number(q.numRowsAffected()) : qsl("no")) << qsl(" affected Rows");
        return true;
    }
    qDebug() << "Failed to execute query. Error: " << q.lastError() << "\n" << q.lastQuery() ;
    return false;
}

int getHighestRowId(const QString& tablename)
{   LOG_CALL;
    return executeSingleValueSql(qsl("MAX(rowid)"), tablename).toInt();
}

bool createDbView(const QString& name, const QString& sql, bool temporary, const QSqlDatabase& db /*= QSqlDatabase::database()*/)
{   LOG_CALL_W(name);

    deleteDbView (name, db);
    QString createViewSql = "CREATE %1 VIEW %2 AS " + sql;
    createViewSql = createViewSql.arg((temporary ? qsl("TEMP") : QString()), name);
    if( executeSql_wNoRecords(createViewSql, db)) {
        return true;
    }
    qCritical() << "Faild to create view " << name;
    return false;
}

bool deleteDbView(const QString& name, const QSqlDatabase& db)
{   LOG_CALL;
    if( executeSql_wNoRecords(qsl("DROP VIEW IF EXISTS %1").arg(name), db)) {
        qInfo() << "dropped view: " << name;
        return true;
    }
    qInfo() << "drop view failed: " << name;
    return false;
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
    executeSql (qsl("SELECT name from sqlite_master WHERE type=='index'"), QVariant(), indices, db);
    for( int i =0; i<indices.count (); i++) {
        QString tablename =indices[i].field(0).value().toString ();
        if( tablename.startsWith (qsl("sqlite"), Qt::CaseInsensitive))
            continue;
        else
            executeSql_wNoRecords (qsl("DROP INDEX %1").arg(tablename), db);
    }
    for(int i=0; i < Sqls.count (); i++) {
        if( not executeSql_wNoRecords (Sqls[i], db))
            return false;
    }
    return true;
}
