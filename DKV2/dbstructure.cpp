#include <qdebug.h>
#include "helper.h"
#include <QSqlDatabase>
#include <QSqlQuery>

#include "dbstructure.h"

dbstructure dbstructure::appendTable(dbtable t)
{
    qDebug() << "adding table to db structure " << t.name;
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
    qDebug() << "accessing db table " << name;
    for( dbtable table : Tables)
    {
        if( table.Name() == name)
            return table;
    }
    Q_ASSERT(!bool("access to unknown database table"));
    return dbtable();
}

bool dbstructure::createDb(QSqlDatabase db) const
{LOG_ENTRY_and_EXIT;
    QSqlQuery q(db);
    bool ret{true};
    for(dbtable table :getTables())
    {
        if(!table.create(db))
            break;
        else
            qDebug() << "Created table:" << table.Name();
    }
    return ret;
}
