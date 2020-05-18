#include <QDebug>
#include <QSqlRecord>
#include <QJsonDocument>
#include <QJsonObject>

#include "helper.h"
#include "sqlhelper.h"

bool tableExists (const QString& tablename, QSqlDatabase db)
{   // LOG_CALL_W(tablename);
    QSqlQuery query(db);
    query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name='" + tablename + "'");
    if( !query.exec())
    {
        qDebug() << "tableExists: query exec in tableExists failed " << query.lastError() << "  " << query.lastQuery();
        return false;
    }
    if( query.first())
    {
        qDebug() << "tableExists: succeded for table " << tablename;
        return true;
    }
    return false;
}

int rowCount(const QString& table)
{
    QSqlQuery q;
    if( q.exec( "SELECT rowid FROM " + table))
    {
        q.last();
        return q.at()+1; // zero based counting
    }
    else
    {
        qCritical() << "row counting query failed" << q.lastError() << endl << q.lastQuery();
        return -1;
    }
}

bool createTables( const dbstructure& structure, QSqlDatabase db)
{   dbgTimer timer(__func__);

    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON", db);
    db.transaction();
    if( structure.createDb())
    {
        db.commit();
        return true;
    }
    db.rollback();
    return false;
}

bool ensureTable( const dbtable& table, QSqlDatabase db)
{   LOG_CALL_W(table.Name());
    if( tableExists(table.Name(), db))
    {
        QVector<QString> fields = getFieldsFromTablename(table.Name(), db);
        for (int i=0; i < table.Fields().count(); i++)
        {
            QString expectedFieldName = table.Fields()[i].name();
            if( fields.indexOf(expectedFieldName) == -1 )
            {
                qDebug() << "ensureTable() failed: table exists with wrong field" << expectedFieldName;
                return false;
            }
        }
        return true;
    }
    // create table
    return table.create(db);
}

QVector<QString> getFieldsFromTablename(const QString& tablename, QSqlDatabase db)
{   LOG_CALL;
    QVector<QString> result;
    QSqlQuery query(db);
    query.prepare("PRAGMA table_info( "+ tablename+ ")");
    if( !query.exec())
    {
        qDebug() << "query exec in getFields failed " << query.lastError();
        return result;
    }
    while (query.next()) {
        result.append(query.value(1).toString());
    }
    return result;
}

QString SelectQueryFromFields(const QVector<dbfield>& fields, const QString& where)
{   LOG_CALL;
    QString Select ("SELECT ");
    QString From ("FROM ");
    QString Where("WHERE " + where);

    QStringList usedTables;
    for(int i=0; i < fields.count(); i++)
    {
        const dbfield& f = fields[i];
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
            Where += " AND [" + ref.tablename + "].[" + ref.name + "]=[" + f.tableName() +"].[" + f.name() +"]";
        }
    }
    QString Query = Select + " " + From + " " + Where;
    qDebug() << "Created Query: " << Query;
    return Query;
}

QSqlRecord ExecuteSingleRecordSql(const QVector<dbfield>& fields, const QString& where)
{   LOG_CALL;
    QString sql = SelectQueryFromFields(fields, where);
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

QVariant ExecuteSingleValueSql(const QString& s, QSqlDatabase db)
{   LOG_CALL_W(s);
    QSqlQuery q(db);
    q.prepare(s);
    if( !q.exec())
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

QVariant ExecuteSingleValueSql( const QString& field, const QString& table, const QString& where, QSqlDatabase db)
{   LOG_CALL;
    QString sql = "SELECT " + field + " FROM " + table + " WHERE " + where;
    return ExecuteSingleValueSql(sql);
}

int ExecuteUpdateSql(const QString& table, const QString& field, const QVariant& newValue, const QString& where)
{   LOG_CALL;
    QString sql = "UPDATE " + table;
    sql += " SET " + field + " = ";

    QString value (newValue.toString());
    if( value[0] != "'") value.push_front("'");
    if( value[value.size()-1] != "'") value.push_back("'");

    sql += value;
    sql += " WHERE " + where;
    QSqlQuery q;
    q.prepare(sql);
    if( q.exec())
    {
        qDebug() << "successfully executed update sql " << q.lastQuery();
        if( q.numRowsAffected())
            qDebug() << q.numRowsAffected() << " record were modified";

        return q.numRowsAffected();
    }
    qCritical() << "faild to execute update sql: " << q.lastError()
                << endl << q.lastQuery();
    return -1;
}

int getHighestTableId(const QString& tablename)
{   LOG_CALL;
    QString sql = "SELECT max(ROWID) FROM " + tablename;
    return ExecuteSingleValueSql(sql).toInt();
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
