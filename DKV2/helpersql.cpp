#include "helper.h"
#include "helpersql.h"

bool autoDetachDb::attachDb(QString filename)
{
    QString sql {qsl("ATTACH DATABASE '%1' AS '%2'")};
    return executeSql_wNoRecords(sql.arg(filename).arg(alias), QVariant(), QSqlDatabase::database(conname));
}
autoDetachDb::~autoDetachDb() {
    QString sql {qsl("DETACH DATABASE '%1'")};
    executeSql_wNoRecords(sql.arg(alias), QVariant(), QSqlDatabase::database(conname));
}


QString DbInsertableString(QVariant v)
{
    if( v.isNull() || !v.isValid())
        return qsl("''");
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

QString dbCreateTable_type(QVariant::Type t)
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
        Q_ASSERT(!bool("invalid database type"));
        return qsl("INVALID");
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
        Q_ASSERT(!bool("invalid database type"));
        return qsl("INVALID");
    }
}

int rowCount(const QString& table, QSqlDatabase db /* =QSqlDatabase::database() */)
{
    QSqlQuery q(qsl("SELECT count(*) FROM ") + table, db);
    if( q.first())
        return q.value(0).toInt();
    else {
        qCritical() << "row counting query failed" << q.lastError() << Qt::endl << q.lastQuery();
        return -1;
    }
}

bool tableExists(const QString& tablename, QSqlDatabase db)
{
    return db.tables().contains(tablename);
}

bool VarTypes_share_DbType( QVariant::Type t1, QVariant::Type t2)
{
    return dbAffinityType(t1) == dbAffinityType(t2);
}

bool verifyTable( const dbtable& tableDef, QSqlDatabase db)
{
    QSqlRecord recordFromDb = db.record(tableDef.Name());
    if( recordFromDb.count() != tableDef.Fields().count()) {
        qDebug() << "verifyTable() failed: number of fields mismatch. expected / actual: " << tableDef.Fields().count() << " / " << recordFromDb.count();
        return false;
    }
    for( auto& field: tableDef.Fields()) {
        QSqlField FieldFromDb = recordFromDb.field(field.name());
        if( ! FieldFromDb.isValid()) {
            qDebug() << "verifyTable() failed: table exists but field is missing" << field.name();
            return false;
        }
        if( ! VarTypes_share_DbType(field.type(), recordFromDb.field(field.name()).type()))
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
    if( tableExists(table.Name(), db)) {
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
        qCritical() << "SingleValueSql failed to execute: " << q.lastError() << Qt::endl << q.lastQuery() << Qt::endl;;
        return QVariant();
    }
    q.last();
    if(q.at() > 0)
    {
        qDebug() << "SingleValueSql returned more than one value\n" << q.lastQuery() << Qt::endl;;
        return QVariant();
    }
    if(q.at() < 0)
    {
        qDebug() << "SingleValueSql returned no value\n" << q.lastQuery() << Qt::endl;;
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
    if( field.name().isEmpty() || field.tableName().isEmpty())
        return QVariant();
    QVariant result = executeSingleValueSql(field.name(), field.tableName(), where, db);

    if( ! result.isValid()) {
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

QString selectQueryFromFields(const QVector<dbfield>& fields, const QVector<dbForeignKey> keys, const QString& incomingWhere, const QString& order)
{   //LOG_CALL;

    QString FieldList;
    QString TableList;
    QString calculatedWhere;
    QSet<QString> usedTables;

    for( auto& f : qAsConst(fields)) {
        if( f.tableName().isEmpty() || f.name().isEmpty())
            qCritical() << "selectQueryFromFields: missing table or field name";
        if( ! FieldList.isEmpty())
            FieldList +=qsl(", ");
        FieldList +=f.tableName() +qsl(".") +f.name();

        if( ! usedTables.contains(f.tableName())) {
            usedTables.insert(f.tableName());
            if( ! TableList.isEmpty())
                TableList +=qsl(", ");
            TableList += f.tableName();
        }
    }
    for( auto key: keys) {
        if( ! calculatedWhere.isEmpty()) calculatedWhere += qsl(" AND ");
        calculatedWhere += key.get_SelectSqpSnippet();
    }
    QString Where =qsl("%1 AND %2");
    Where = Where.arg((incomingWhere.isEmpty() ? qsl("true") : incomingWhere), (calculatedWhere.isEmpty() ? qsl("true") : calculatedWhere));

    QString Query =qsl("SELECT %1 FROM %2 WHERE %3");
    Query = Query.arg(FieldList, TableList, Where);
    if( ! order.isEmpty())
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

QVector<QVariant> executeSingleColumnSql( const dbfield field, const QString& where)
{   LOG_CALL;
    if( field.tableName().isEmpty() || field.name().isEmpty()) {
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
    if(q.at() != 0) {
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
    q.prepare(sql);
    if( v.isValid()) q.bindValue(0, v);
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
    q.prepare(sql);
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
bool executeSql_wNoRecords(QString sql, QVariant v, QSqlDatabase db)
{
    if( v.isValid())
        return executeSql_wNoRecords(sql, QVector<QVariant>{v}, db);
    else
        return executeSql_wNoRecords(sql, QVector<QVariant>(), db);
}
bool executeSql_wNoRecords(QString sql, QVector<QVariant> v, QSqlDatabase db)
{   LOG_CALL;
    QSqlQuery q(db); q.setForwardOnly(true);
    q.prepare(sql);
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
        qInfo() << "Successfully executed query \n" << q.lastQuery();
        qInfo() << "Number of Rows affected: " << q.numRowsAffected();
        qInfo() << Qt::endl;
        return true;
    }
    qDebug() << "Failed to execute query. Error: " << q.lastQuery() << Qt::endl << q.lastError() ;
//    qInfo() << Qt::endl;
    return false;
}

int getHighestRowId(const QString& tablename)
{   LOG_CALL;
    return executeSingleValueSql(qsl("MAX(rowid)"), tablename).toInt();
}
