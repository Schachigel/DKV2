#include <qdebug.h>

#include "dbstructure.h"


void dbstructure::addTable(const dbtable& t)
{
    for (auto table: Tables)
        if( table.name == t.name)
        {
            qCritical() << "Versuch eine Tabelle wiederholt zur Datenbank hinzuzufügen";
            Q_ASSERT(!bool("redundent table in structure"));
        }
    Tables.append(t);
}

dbtable dbstructure::operator[](QString name)
{
    for( dbtable table : Tables)
    {
        if( table.name == name)
            return table;
    }
    Q_ASSERT(!bool("access to unknown database table"));
    return dbtable("");
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

