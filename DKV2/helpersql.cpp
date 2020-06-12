#include "helper.h"
#include "helpersql.h"

bool vTypesShareDbType( QVariant::Type t1, QVariant::Type t2)
{
    return dbAffinityType(t1) == dbAffinityType(t2);
}
QString dbInsertableString(QVariant v)
{
    if( v.isNull() || !v.isValid())
        return "''";
    QString s;
    switch(v.type())
    {
    case QVariant::Date: {
        s = v.toDate().toString(Qt::ISODate);
        break;
    }
    case QVariant::Double: {
        s = QString::number(v.toDouble(), 'f', 2);
        break;
    }
    case QVariant::Bool:
        s = (v.toBool()) ? "1" : "0";
        break;
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
        s = QString::number(v.toInt());
        break;
    case QVariant::String:
    case QVariant::Char:
        s = v.toString();
        break;
    default:
        qDebug() << "vTypeToDbType defaulted " << v;
        s = v.toString();
    }
    return "'" + s +"'";
}

QString dbCreateTable_type(QVariant::Type t)
{   // these are the strings we use in data definition
    // so that they are expressive, but are still close
    // to the actual affinity data types
    switch( t)
    {
    case QVariant::String:
    case QVariant::Char:
        return "TEXT"; // affinity: TEXT
    case QVariant::Date:
        return "TEXTDATE"; // affinity: TEXT
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::UInt:
    case QVariant::ULongLong:
        return "INTEGER"; // affinity: INTEGER
    case QVariant::Bool:
        return "BOOLEAN";
    case QVariant::Double:
        return "DOUBLE"; // affinity: REAL
    default:
        Q_ASSERT(!bool("invalid database type"));
        return "INVALID";
    }
}
QString dbAffinityType(QVariant::Type t)
{   // affinities are, what the database actually uses
    // as a "preferred" type of data stored in a column
    switch( t)
    {
    case QVariant::String:
    case QVariant::Char:
    case QVariant::Date:
        return "TEXT";
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::UInt:
    case QVariant::ULongLong:
    case QVariant::Bool:
        return "INTEGER";
    case QVariant::Double:
        return "REAL";
    default:
        Q_ASSERT(!bool("invalid database type"));
        return "INVALID";
    }
}

int rowCount(const QString& table)
{
    QSqlQuery q("SELECT count(*) FROM " + table);
    if( q.first())
        return q.value(0).toInt();
    else {
        qCritical() << "row counting query failed" << q.lastError() << endl << q.lastQuery();
        return -1;
    }
}

bool tableExists(const QString& tablename, QSqlDatabase db)
{
    return db.tables().contains(tablename);
}
bool verifyTable( const dbtable& tableDef, QSqlDatabase db)
{
    QSqlRecord recordFromDb = db.record(tableDef.Name());
    if( recordFromDb.count() != tableDef.Fields().count()) {
        qDebug() << "verifyTable() failed: number of fields mismatch. expected / actual: " << tableDef.Fields().count() << " / " << recordFromDb.count();
        return false;
    }
    for( auto field: tableDef.Fields()) {
        QSqlField FieldFromDb = recordFromDb.field(field.name());
        if( ! FieldFromDb.isValid()) {
            qDebug() << "verifyTable() failed: table exists but field is missing" << field.name();
            return false;
        }
        if( ! vTypesShareDbType(field.type(), recordFromDb.field(field.name()).type()))
        {
            qDebug() << "ensureTable() failed: field " << field.name() <<
                        " type mismatch. expected / actual: " << field.type() << " / " << recordFromDb.field(field.name()).type();
            return false;
        }

    }
    return true;
}
bool ensureTable( const dbtable& table, QSqlDatabase db)
{   LOG_CALL_W(table.Name());
    if( tableExists(table.Name(), db))
    {
        return verifyTable(table, db);
    }
    // create table
    return table.create(db);
}

QVariant executeSingleValueSql(const QString& sql, QSqlDatabase db)
{
    QSqlQuery q(db);
    if( !q.exec(sql))
    {
        qCritical() << "SingleValueSql failed to execute: " << q.lastError() << endl << q.lastQuery() << endl;;
        return QVariant();
    }
    q.last();
    if(q.at() > 0)
    {
        qDebug() << "SingleValueSql returned more than one value\n" << q.lastQuery() << endl;;
        return QVariant();
    }
    if(q.at() < 0)
    {
        qDebug() << "SingleValueSql returned no value\n" << q.lastQuery() << endl;;
        return QVariant();
    }
    qInfo() << "sql " << sql << " returned " << q.value(0);
    return q.value(0);
}
QVariant executeSingleValueSql(const QString& fieldName, const QString& tableName, const QString& where, QSqlDatabase db)
{
    QString sql = "SELECT " + fieldName + " FROM " + tableName + (where.isEmpty()? "" : (" WHERE " + where));
    return executeSingleValueSql(sql, db);
}
QVariant executeSingleValueSql(const dbfield& field, const QString& where, QSqlDatabase db)
{
    if( field.name().isEmpty() || field.tableName().isEmpty())
        return QVariant();
    QVariant result = executeSingleValueSql(field.name(), field.tableName(), where, db);

    if( ! result.isValid()) {
        qDebug() << "executeSingleValueSql found no value";
        return result;
    }
    if(result.convert(field.type()))
        return result;
    else {
        qCritical() << "executeSingleValueSql(): variant type conversion failed";
        return QVariant();
    }
}

QString selectQueryFromFields(const QVector<dbfield>& fields, const QVector<dbForeignKey> keys, const QString& incomingWhere)
{   //LOG_CALL;

    QString FieldList;
    QString TableList;
    QString calculatedWhere;
    QSet<QString> usedTables;

    for( auto f : fields) {
        if( f.tableName().isEmpty() || f.name().isEmpty())
            qCritical() << "selectQueryFromFields: missing table or field name";
        if( ! FieldList.isEmpty())
            FieldList += ", ";
        FieldList += f.tableName() + "." + f.name();

        if( ! usedTables.contains(f.tableName())) {
            usedTables.insert(f.tableName());
            if( ! TableList.isEmpty())
                TableList += ", ";
            TableList += f.tableName();
        }
    }
    for( auto key: keys) {
        if( ! calculatedWhere.isEmpty()) calculatedWhere += " AND ";
        calculatedWhere += key.get_SelectSqpSnippet();
    }
    QString Where = "WHERE %1 AND %2";
    Where = Where.arg(incomingWhere.isEmpty() ? "true" : incomingWhere);
    Where = Where.arg(calculatedWhere.isEmpty() ? "true" : calculatedWhere);

    QString Query = "SELECT %1 FROM %2 %3";
    Query = Query.arg(FieldList).arg(TableList).arg(Where);
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

//QVector<QVariant> executeSingleColumnSql( const QString field, const QString table, const QString& where)
//{
//    QString sql = "SELECT %1 FROM %2 WHERE %3";
//    sql = sql.arg(field).arg(table).arg(where.isEmpty() ? "true" : where);
//    QSqlQuery q;
//    QVector<QVariant> result;
//    if( q.exec(sql)){
//        while(q.next()) {
//            result.push_back(q.record().field(0).value());
//        }
//        return result;
//    }
//    qCritical() << "Failed to execute single column sql " << q.lastError() << endl << q.lastQuery();
//    return QVector<QVariant>();
//}

QVector<QVariant> executeSingleColumnSql( const dbfield field, const QString& where)
{   LOG_CALL;
    if( field.tableName().isEmpty() || field.name().isEmpty()) {
        qCritical() << "incomplete dbfield";
        return QVector<QVariant>();
    }
    QString sql = "SELECT %1 FROM %2 %3";
    sql = sql.arg(field.name()).arg(field.tableName()).arg((where.isEmpty() ? "" : (" WHERE " + where)));
    QSqlQuery q;
    QVector<QVariant> result;
    if( q.exec(sql))
        while( q.next()) {
            result.push_back(adjustedType(q.record().field(0), field.type()).value());
        }
    else
        qCritical() << "SingleColumnSql failed " << q.lastError() << endl << q.lastQuery();

    return result;
}
QSqlRecord executeSingleRecordSql(const QVector<dbfield>& fields, const QString& where)
{
    QString sql = selectQueryFromFields(fields, QVector<dbForeignKey>(), where);
    qDebug() << "ExecuteSingleRecordSql:\n" << sql;

    QSqlQuery q;
    if( !q.exec(sql)) {
        qCritical() << "SingleRecordSql failed " << q.lastError() << endl << q.lastQuery();
        return QSqlRecord();
    }
    q.last();
    if(q.at() != 0) {
        qCritical() << "SingleRecordSql returned more then one value\n" << q.lastQuery();
        return QSqlRecord();
    }
    // adjust the database types to the expected types
    QSqlRecord result;
    for( auto dbFieldEntry : fields) {
        QSqlField tmpField =q.record().field(dbFieldEntry.name());
        result.append(adjustedType(tmpField,dbFieldEntry.type()));
    }
    return result;
}
QVector<QSqlRecord> executeSql(const QVector<dbfield>& fields, const QString& where)
{
    QString sql = selectQueryFromFields(fields, QVector<dbForeignKey>(), where);
    qDebug() << "executeSql:\n" << sql;

    QSqlQuery q; q.setForwardOnly(true);
    if( !q.exec(sql)) {
        qCritical() << "executeSql failed " << q.lastError() << endl << q.lastQuery();
        return QVector<QSqlRecord>();
    }
    QVector<QSqlRecord> result;
    while( q.next()) {
        // adjust the database types to the expected types
        QSqlRecord oneRecord;
        for( auto dbFieldEntry : fields) {
            // adjust to original variant data type
            QSqlField tmpField {adjustedType(q.record().field(dbFieldEntry.name()), dbFieldEntry.type())};
            oneRecord.append(tmpField);
        }
        result.push_back(oneRecord);
    }
    return result;
}

int getHighestRowId(const QString& tablename)
{   LOG_CALL;
    return executeSingleValueSql("max(rowid)", tablename).toInt();
}
