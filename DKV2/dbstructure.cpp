#include <QSqlDatabase>

#include "helper.h"
#include "helpersql.h"
#include "dbstructure.h"

dbstructure dbstructure::appendTable(dbtable t)
{   // LOG_CALL;
    qDebug() << "adding table to db structure " << t.name;
    for (auto table: Tables) {
        if( table.Name() == t.Name()) {
            qCritical() << "Versuch eine Tabelle wiederholt zur Datenbank hinzuzufügen";
            Q_ASSERT(!bool("redundent table in structure"));
        }
    }
    Tables.append(t);
    return *this;
}

dbtable dbstructure::operator[](const QString& name) const
{   // LOG_CALL;
    // qDebug() << "accessing db table " << name;
    for( dbtable table : Tables)
    {
        if( table.Name() == name)
            return table;
    }
    qCritical() << "trying to access unknown table " << name;
    Q_ASSERT(!bool("access to unknown database table"));
    return dbtable();
}

bool dbstructure::createDb(QSqlDatabase db) const
{   LOG_CALL;
    QSqlQuery enableRefInt("PRAGMA foreign_keys = ON", db);
    for(dbtable table :getTables()) {
        if(!ensureTable(table, db)) {
            qCritical() << "could not create table " << table.name;
            return false;
        }
    }
    return true;
}
