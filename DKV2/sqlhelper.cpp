#include <QDebug>
#include <QSqlRecord>
#include <QJsonDocument>
#include <QJsonObject>

#include "helper.h"
#include "sqlhelper.h"

bool typesAreDbCompatible( QVariant::Type t1, QVariant::Type t2)
{
    if( t1 == t2) return true;
    if( t1 == QVariant::LongLong) t1 = QVariant::Int;
    if( t2 == QVariant::LongLong) t2 = QVariant::Int;
    if( t1 == QVariant::Date) t1 = QVariant::String;
    if( t2 == QVariant::Date) t2 = QVariant::String;
    if( t1 == QVariant::Bool) t1 = QVariant::Int;
    if( t2 == QVariant::Bool) t2 = QVariant::Int;
    if( t1 == t2) return true;
    return false;
}

bool tableExists(const QString& tablename, QSqlDatabase db)
{
    return db.tables().contains(tablename);
}

int rowCount(const QString& table)
{
    QSqlQuery q;
    if( q.exec( "SELECT count(*) FROM " + table))
    {
        q.first();
        return q.value(0).toInt();
    }
    else
    {
        qCritical() << "row counting query failed" << q.lastError() << endl << q.lastQuery();
        return -1;
    }
}

bool verifyTable( const dbtable& tableDef, QSqlDatabase db)
{
    QSqlRecord givenRecord = db.record(tableDef.Name());
//    qDebug() << "record from database: " << givenRecord;
    if( givenRecord.count() != tableDef.Fields().count()) {
        qDebug() << "verifyTable() failed: number of fields mismatch. expected / actual: " << tableDef.Fields().count() << " / " << givenRecord.count();
        return false;
    }
    for (int i=0; i < tableDef.Fields().count(); i++) {
        QString expectedFieldName = tableDef.Fields()[i].name();
        QVariant::Type expectedType = tableDef.Fields()[i].type();

        if( ! givenRecord.contains(expectedFieldName)) {
            qDebug() << "verifyTable() failed: table exists but field is missing" << expectedFieldName;
            return false;
        }
// todo: repair type comparison (Vertraege.KreditorId fails)
//        QSqlField givenField = givenRecord.field(tableDef.Fields()[i].name());
//        QVariant::Type givenType = givenField.type();
//        if( ! typesAreDbCompatible(expectedType, givenType))
//        {
//            qDebug() << "ensureTable() failed: field " << expectedFieldName << " type mismatch. expected / actual: " << expectedType << " / " << givenType;
//            return false;
//        }
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

QString selectQueryFromFields(const QVector<dbfield>& fields, const QString& incomingWhere)
{   //LOG_CALL;

    QString Select ("SELECT ");
    QString From ("FROM ");
    QString calculatedWhere;

    QStringList usedTables;
    for(int i=0; i < fields.count(); i++)
    {
        const dbfield& f = fields[i];
        if( f.tableName().isEmpty())
            qCritical() << "selectQueryFromFields: missing table name";
        if( f.name().isEmpty())
            qCritical() << "selectQueryFromFields: missing field name";
        if( i!=0)
            Select += ", ";
        Select += f.tableName() + "." + f.name();

        if( !usedTables.contains(f.tableName()))
        {
            if( usedTables.count()!= 0)
                From += ", ";
            usedTables.push_back(f.tableName());
            From += f.tableName();
        }

        refFieldInfo ref = f.getReferenzeInfo();
        if( !ref.tablename.isEmpty())
        {
            if( ! calculatedWhere.isEmpty()) calculatedWhere += " AND ";
            calculatedWhere += ref.tablename + "." + ref.name + "=" + f.tableName() +"." + f.name();
        }
    }
    QString Where;
    if( incomingWhere.isEmpty() && calculatedWhere.isEmpty())
        Where = "";
    else if( incomingWhere.isEmpty())
        Where = "WHERE " + calculatedWhere;
    else if( calculatedWhere.isEmpty())
        Where = "WHERE " + incomingWhere;
    else
        Where = "WHERE " + incomingWhere + " AND " + calculatedWhere;

    QString Query = Select + " " + From + " " + Where;
    qInfo() << "selectQueryFromFields created Query: " << Query;
    return Query;
}

QSqlRecord executeSingleRecordSql(const QVector<dbfield>& fields, const QString& where)
{   //LOG_CALL;
    QString sql = selectQueryFromFields(fields, where);
    qDebug() << "ExecuteSingleRecordSql:\n" << sql;
    QSqlQuery q;
    q.prepare(sql);
    if( !q.exec())
    {
        qCritical() << "SingleRecordSql failed " << q.lastError() << endl << q.lastQuery();
        return QSqlRecord();
    }
    q.last();
    if(q.at() != 0)
    {
        qCritical() << "SingleRecordSql returned more then one value\n" << q.lastQuery();
        return QSqlRecord();
    }
    return q.record();
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
