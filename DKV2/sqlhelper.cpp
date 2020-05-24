#include <QDebug>
#include <QSqlRecord>
#include <QJsonDocument>
#include <QJsonObject>

#include "helper.h"
#include "sqlhelper.h"

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

QString selectQueryFromFields(const QVector<dbfield>& fields, const QVector<dbForeignKey> keys, const QString& incomingWhere)
{   //LOG_CALL;

    QString Select;
    QString From;
    QString calculatedWhere;
    QSet<QString> usedTables;

    for( auto f : fields) {
        if( f.tableName().isEmpty())
            qCritical() << "selectQueryFromFields: missing table name";
        if( f.name().isEmpty())
            qCritical() << "selectQueryFromFields: missing field name";
        if( ! Select.isEmpty())
            Select += ", ";
        Select += f.tableName() + "." + f.name();

        if( ! usedTables.contains(f.tableName()))
        {
            usedTables.insert(f.tableName());
            if( ! From.isEmpty())
                From += ", ";
            From += f.tableName();
        }
    }
    for( auto key: keys) {
        if( ! calculatedWhere.isEmpty()) calculatedWhere += " AND ";
        calculatedWhere += key.get_SelectSqpSnippet();
    }
    QString Where;
    if( incomingWhere.isEmpty() && calculatedWhere.isEmpty())
        Where = "";
    else
        Where = " WHERE ";

    if( incomingWhere.isEmpty())
        Where += calculatedWhere;
    else if( calculatedWhere.isEmpty())
        Where += incomingWhere;
    else
        Where += incomingWhere + " AND " + calculatedWhere;

    QString Query = "SELECT " + Select + " FROM " + From + Where;
    qInfo() << "selectQueryFromFields created Query: " << Query;
    return Query;
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
        tmpField.setType(dbFieldEntry.type());
        result.append(tmpField);
    }
    qDebug() << "record from db   : " << q.record()<< endl;
    qDebug() << "calculated result: " << result << endl;
    return result;
}

QVariant executeSingleValueSql(const QString& sql, QSqlDatabase db)
{   LOG_CALL_W(sql);
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
    return q.value(0);
}

QVariant executeSingleValueSql( const QString& field, const QString& table, const QString& where, QSqlDatabase db)
{//   LOG_CALL;
    QString sql = "SELECT " + field + " FROM " + table + (where.isEmpty()? "" : (" WHERE " + where));
    return executeSingleValueSql(sql, db);
}

QVector<QVariant> executeSingleColumnSql( const QString& field, const QString& table, const QString& where)
{   LOG_CALL;
    QVector<QVariant> result;
    QSqlQuery q;
    if( q.exec("SELECT " + field + " FROM " + table + (where.isEmpty()? "" : (" WHERE " + where))))
        while( q.next()) {
            result.push_back(q.record().value(0));
        }
    else
        qCritical() << "SingleColumnSql failed " << q.lastError() << endl << q.lastQuery();

    return result;
}

//int ExecuteUpdateSql(const QString& table, const QString& field, const QVariant& newValue, const QString& where)
//{   LOG_CALL;
//    QString sql = "UPDATE " + table;
//    sql += " SET " + field + " = ";

//    QString value (newValue.toString());
//    if( value[0] != "'") value.push_front("'");
//    if( value[value.size()-1] != "'") value.push_back("'");

//    sql += value;
//    sql += " WHERE " + where;
//    QSqlQuery q;
//    q.prepare(sql);
//    if( q.exec())
//    {
//        qDebug() << "successfully executed update sql " << q.lastQuery();
//        if( q.numRowsAffected())
//            qDebug() << q.numRowsAffected() << " record were modified";

//        return q.numRowsAffected();
//    }
//    qCritical() << "faild to execute update sql: " << q.lastError()
//                << endl << q.lastQuery();
//    return -1;
//}

int getHighestTableId(const QString& tablename)
{   LOG_CALL;
    QString sql = "SELECT max(ROWID) FROM " + tablename;
    return executeSingleValueSql(sql).toInt();
}

QString JsonFromRecord( QSqlRecord r)
{   LOG_CALL;
    QMap<QString, QVariantMap> jRecord;
    for(int i=0; i< r.count(); i++)
    {
        QSqlField f(r.field(i));
        QVariantMap& vMap = jRecord[f.tableName()];
        vMap[f.name()] = f.value();
    }
    QJsonObject root;
    for( auto table : jRecord.keys())
    {
        QString tablename(table);
        QVariantMap map = jRecord.value(table);
        root.insert(table, QJsonObject::fromVariantMap(map));
    }
    QJsonDocument doc;
    doc.setObject(root);
    return doc.toJson();
}

template<>
bool sqlVal(QSqlQuery sql, QString fname)
{
    return sql.record().value(fname).toBool();
}

template <>
int sqlVal(QSqlQuery sql, QString fname)
{
    return sql.record().value(fname).toInt();
}

template <>
double sqlVal(QSqlQuery sql, QString fname)
{
    return sql.record().value(fname).toDouble();
}

template <>
QString sqlVal(QSqlQuery sql, QString fname)
{
    return sql.record().value(fname).toString();
}

template<>
QDate sqlVal(QSqlQuery sql, QString fname)
{
    return sql.record().value(fname).toDate();
}
