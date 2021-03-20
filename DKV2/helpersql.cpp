#include <iso646.h>

#include "helper.h"
#include "helpersql.h"

const QString dbTypeName {qsl("QSQLITE")};

bool autoDetachDb::attachDb(const QString& filename)
{
    LOG_CALL;
    QString sql {qsl("ATTACH DATABASE '%1' AS '%2'")};
    return executeSql_wNoRecords(sql.arg(filename).arg(alias()), QSqlDatabase::database(conname()));
}
autoDetachDb::~autoDetachDb()
{
    LOG_CALL;
    QString sql {qsl("DETACH DATABASE '%1'")};
    executeSql_wNoRecords(sql.arg(alias()), QSqlDatabase::database(conname()));
}


QString DbInsertableString(const QVariant& v)
{
    if( v.isNull() or !v.isValid())
        return qsl("''");
    QString s;
    switch(v.type())
    {
    case QVariant::Date: {
        s = v.toDate().toString(Qt::ISODate);
        break;
    }
    case QVariant::DateTime: {
        s =v.toDateTime().toString(qsl("yyyy.MM.dd - hh:mm"));
        break;
    }
    case QVariant::Double: {
        s = QString::number(v.toDouble(), 'f', 2);
        break;
    }
    case QVariant::Bool:
        s = (v.toBool()) ? qsl("1") : qsl("0");
        break;
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
        s = QString::number(v.toInt());
        break;
    case QVariant::String:
    case QVariant::Char:
        s = v.toString().replace("'", "''");;
        break;
    default:
        qDebug() << "switch(v.type()) DEFAULTED " << v;
        s = v.toString();
    }
    return "'" + s +"'";
}

QString dbCreateTable_type(const QVariant::Type t)
{   // these are the strings we use in data definition
    // so that they are expressive, but are still close
    // to the actual affinity data types
    switch( t)
    {
    case QVariant::String:
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
        qCritical() << "row counting query failed" << q.lastError() << Qt::endl << q.lastQuery();
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
    Q_ASSERT( not alias.isEmpty());
    QString sql {qsl("PRAGMA %1.FOREIGN_KEYS = %2")};
    sql =sql.arg(alias, OnOff?qsl("ON"):qsl("OFF"));
    return executeSql_wNoRecords(sql, db);
}
bool switchForeignKeyHandling(const QSqlDatabase& db, bool OnOff)
{
    if( OnOff)
        return executeSql_wNoRecords(qsl("PRAGMA FOREIGN_KEYS = ON"), db);
    else
        return executeSql_wNoRecords(qsl("PRAGMA FOREIGN_KEYS = OFF"), db);
}


QVariant executeSingleValueSql(const QString& sql, QSqlDatabase db)
{
    QSqlQuery q(db);
    if( !q.exec(sql)) {
        qCritical() << "SingleValueSql failed to execute: " << q.lastError() << Qt::endl << q.lastQuery() << Qt::endl;;
        return QVariant();
    }
    q.last();
    if(q.at() > 0) {
        qDebug() << "SingleValueSql returned more than one value\n" << q.lastQuery() << Qt::endl;;
        return QVariant();
    }
    if(q.at() < 0) {
        // qDebug() << "SingleValueSql returned no value\n" << q.lastQuery() << Qt::endl;;
        return QVariant();
    }
    qInfo() << "sql " << sql << " returned " << q.value(0);
    return q.value(0);
}
QVariant executeSingleValueSql(const QString& fieldName, const QString& tableName, const QString& where, QSqlDatabase db)
{
    QString sql = qsl("SELECT ") + fieldName + qsl(" FROM ") + tableName + (where.isEmpty() ? qsl("") : (qsl(" WHERE ") + where));
    return executeSingleValueSql(sql, db);
}
QVariant executeSingleValueSql(const dbfield& field, const QString& where, QSqlDatabase db)
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
{   //LOG_CALL;

    QString FieldList;
    QString TableList;
    QString calculatedWhere;
    QSet<QString> usedTables;

    for( auto& f : qAsConst(fields)) {
        if( f.tableName().isEmpty() or f.name().isEmpty())
            qCritical() << "selectQueryFromFields: missing table or field name";
        if( not FieldList.isEmpty())
            FieldList +=qsl(", ");
        FieldList +=f.tableName() +qsl(".") +f.name();

        if( not usedTables.contains(f.tableName())) {
            usedTables.insert(f.tableName());
            if( not TableList.isEmpty())
                TableList +=qsl(", ");
            TableList += f.tableName();
        }
    }
    for( auto key: keys) {
        if( not calculatedWhere.isEmpty()) calculatedWhere += qsl(" AND ");
        calculatedWhere += key.get_SelectSqpSnippet();
    }
    QString Where =qsl("%1 AND %2");
    Where = Where.arg((incomingWhere.isEmpty() ? qsl("true") : incomingWhere), (calculatedWhere.isEmpty() ? qsl("true") : calculatedWhere));

    QString Query =qsl("SELECT %1 FROM %2 WHERE %3");
    Query = Query.arg(FieldList, TableList, Where);
    if( not order.isEmpty())
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
    if( q.exec(sql))
        while( q.next()) {
            result.push_back(adjustedType(q.record().field(0), field.type()).value());
        }
    else
        qCritical() << "SingleColumnSql failed " << q.lastError() << Qt::endl << q.lastQuery();

    return result;
}
QSqlRecord executeSingleRecordSql(const QVector<dbfield>& fields, const QString& where, const QString& order)
{
    QString sql = selectQueryFromFields(fields, QVector<dbForeignKey>(), where, order);
    qDebug() << "ExecuteSingleRecordSql:\n" << sql;

    QSqlQuery q;
    if( !q.exec(sql)) {
        qCritical() << "SingleRecordSql failed " << q.lastError() << Qt::endl << q.lastQuery();
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
QVector<QSqlRecord> executeSql(const QVector<dbfield>& fields, const QString& where, const QString& order)
{
    QString sql = selectQueryFromFields(fields, QVector<dbForeignKey>(), where, order);
    QVector<QSqlRecord> result;
    QSqlQuery q; q.setForwardOnly(true);
    if( !q.exec(sql)) {
        qCritical() << "executeSql failed " << q.lastError() << Qt::endl << q.lastQuery();
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
bool executeSql(const QString& sql, const QVariant& v, QVector<QSqlRecord>& result)
{
    QSqlQuery q; q.setForwardOnly(true);
    if( q.prepare(sql)) {
        qDebug() << "Faild to prep Query. Error:" << q.lastError();
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
        qDebug() << "Faild to execute Query. Error:" << q.lastError() << Qt::endl << q.lastQuery();
        return false;
    }
}
bool executeSql(const QString& sql, const QVector<QVariant>& v, QVector<QSqlRecord>& result)
{
    QSqlQuery q; q.setForwardOnly(true);
    if( not q.prepare(sql)) {
        qDebug() << "Faild to prep Query. Error:" << q.lastError();
        return false;
    }
    for( int i =0; i<v.size(); i++) {
        q.bindValue(i, v[i]);
    }
    if( q.exec()) {
        while(q.next()) {
            result.push_back(q.record());
        }
        return true;
    } else {
        qDebug() << "Faild to execute Query. Error:" << q.lastError() << Qt::endl << q.lastQuery();
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
        qCritical() << "failed to prep query " << q.lastError() << Qt::endl << q.lastQuery();
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
        qInfo() << "Successfully executed query " << q.lastQuery() << " with "
        << (q.numRowsAffected() ? QString::number(q.numRowsAffected()) : QString("no")) << " affected Rows";
        qInfo() << Qt::endl;
        return true;
    }
    qDebug() << "Failed to execute query. Error: " << q.lastQuery() << Qt::endl << q.lastError() ;
    return false;
}

int getHighestRowId(const QString& tablename)
{   LOG_CALL;
    return executeSingleValueSql(qsl("MAX(rowid)"), tablename).toInt();
}
