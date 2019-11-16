#include <QDebug>
#include <QSqlRecord>
#include <QJsonDocument>
#include <QJsonObject>

#include "helper.h"
#include "sqlhelper.h"

QString SelectQueryFromFields(const QVector<dbfield>& fields, const QString& where)
{
    QString Select ("SELECT ");
    QString From ("FROM ");
    QString Where("WHERE " + where);

    QStringList usedTables;
    for(int i=0; i < fields.count(); i++)
    {
        const dbfield& f = fields[i];
        if( i!=0)
            Select += ", ";
        Select += "[" + f.tableName() + "].[" + f.name() + "]";

        if( !usedTables.contains(f.tableName()))
        {
            if( usedTables.count()!= 0)
                From += ", ";
            usedTables.push_back(f.tableName());
            From += "[" + f.tableName() +"]";
        }

        refFieldInfo ref = f.getReferenzeInfo();
        if( !ref.tablename.isEmpty())
        {
            Where += " AND [" + ref.tablename + "].[" + ref.name + "]=[" + f.tableName() +"].[" + f.name() +"]";
        }
    }
    return Select + " " + From + " " + Where;
}

QSqlRecord ExecuteSingleRecordSql(const QVector<dbfield>& fields, const QString& where, const QString& con)
{LOG_ENTRY_and_EXIT;
    QString sql = SelectQueryFromFields(fields, where);
    qDebug() << "ExecuteSingleRecordSql:\n" << sql;
    QSqlQuery q(QSqlDatabase::database(con));
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

QVariant ExecuteSingleValueSql(const QString& s, const QString& con)
{   LOG_ENTRY_and_EXIT;
    QSqlQuery q(QSqlDatabase::database(con));
    q.prepare(s);
    if( !q.exec())
    {
        qCritical() << "SingleValueSql failed to execute: " << q.lastError() << endl << q.lastQuery();
        return QVariant();
    }
    q.last();
    if(q.at() != 0)
    {
        qCritical() << "SingleValueSql returned more then one value\n" << q.lastQuery();
        return QVariant();
    }
    return q.value(0);
}

QVariant ExecuteSingleValueSql( const QString& field, const QString& table, const QString& where, const QString& con)
{LOG_ENTRY_and_EXIT;
    QString sql = "SELECT " + field + " FROM " + table + " WHERE " + where;
    return ExecuteSingleValueSql(sql, con);
}

bool ExecuteUpdateSql(const QString& table, const QString& field, const QVariant& newValue, const QString& where, const QString& con)
{LOG_ENTRY_and_EXIT;
    QString sql = "UPDATE " + table;
    sql += " SET " + field + " = '" + newValue.toString();
    sql += "' WHERE " + where;
    QSqlQuery q(QSqlDatabase::database(con));
    q.prepare(sql);
    if( q.exec())
    {
        qDebug() << "successfully executed update sql " << q.lastQuery();
        return true;
    }
    qCritical() << "faild to execute update sql: " << q.lastError()
                << endl << q.lastQuery();
    return false;
}


int getHighestTableId(const QString& tablename, const QString& con)
{LOG_ENTRY_and_EXIT;
    QString sql = "SELECT max(ROWID) FROM " + tablename;
    return ExecuteSingleValueSql(sql, con).toInt();
}

QString JsonFromRecord( QSqlRecord r)
{LOG_ENTRY_and_EXIT;
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
