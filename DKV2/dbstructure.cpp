#include <qdebug.h>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "dbstructure.h"

dbstructure dbstructure::appendTable(dbtable t)
{
    for (auto table: Tables)
        if( table.name == t.name)
        {
            qCritical() << "Versuch eine Tabelle wiederholt zur Datenbank hinzuzufügen";
            Q_ASSERT(!bool("redundent table in structure"));
        }
    Tables.append(t);
    return *this;
}

dbtable dbstructure::operator[](const QString& name)
{
    for( dbtable table : Tables)
    {
        if( table.name == name)
            return table;
    }
    Q_ASSERT(!bool("access to unknown database table"));
    return dbtable("");
}

bool dbstructure::createDb(QSqlDatabase db)
{
    QSqlQuery q(db);
    bool ret{true};
    for(dbtable table :getTables())
    {
        QString tableSql(table.CreateTableSQL());
        ret &= q.exec(tableSql);
        if(!ret)
        {
            qCritical() << "Table creation failed: " << q.lastError() << "\n" << tableSql << endl;
            break;
        }
        else
            qDebug() << "Created table:" << table.name  << "\n" << tableSql << endl;
    }
    return ret;
}

bool dbstructure::checkDb()
{

}

QString dbstructure::checkTablesSql()
{
    QString sql("SELECT * FROM ");
    for( int i=0; i<Tables.count(); i++)
    {
        if( i>0) sql.append(", ");
        sql.append(Tables[i].name);
    }
    return sql;
}
