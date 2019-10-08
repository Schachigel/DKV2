#include <qdebug.h>
#include <QSqlDatabase>
#include <QSqlQuery>

#include "dbstructure.h"

dbstructure dbstructure::appendTable(dbtable t)
{
    for (auto table: Tables)
        if( table.Name() == t.Name())
        {
            qCritical() << "Versuch eine Tabelle wiederholt zur Datenbank hinzuzufügen";
            Q_ASSERT(!bool("redundent table in structure"));
        }
    Tables.append(t);
    return *this;
}

dbtable dbstructure::operator[](const QString& name) const
{
    for( dbtable table : Tables)
    {
        if( table.Name() == name)
            return table;
    }
    Q_ASSERT(!bool("access to unknown database table"));
    return dbtable();
}

bool dbstructure::createDb(QSqlDatabase db) const
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
            qDebug() << "Created table:" << table.Name()  << "\n" << tableSql;
    }
    return ret;
}
