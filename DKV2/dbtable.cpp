#include "dbtable.h"
#include "dbfield.h"

dbfield dbtable::operator[](QString s)
{
    for (auto f : fields)
    {
        if( f.name == s)
            return f;
    }
    return dbfield();
}

QString dbtable::CreateTableSQL()
{
    QString sql("CREATE TABLE [" + name + "] (");
    for( int i = 0; i< fields.count(); i++)
    {
        if( i>0) sql.append(", ");
        sql.append(fields[i].CreateSQL());
    }
    sql.append(")");
    return sql;
}


