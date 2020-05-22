#include "helper.h"

#include "dbfield.h"
#include "dbtable.h"

bool dbfield::operator ==(const dbfield &b) const
{
    return ((tableName() == b.tableName())
            && (name() == b.name())
            && (type() == b.type()));
}

QString dbTypeFromVariant(QVariant::Type t)
{
    switch( t)
    {
    case QVariant::String:
        return "TEXT";
    case QVariant::Date:
        return "DATE";
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Bool:
        return "INTEGER";
    case QVariant::Double:
        return "FLOAT";
    default:
        Q_ASSERT(!bool("invalid database type"));
        return "INVALID";
    }
}

QString dbfield::get_CreateSqlSnippet()
{   //LOG_CALL_W(name());
    return name() + " " + dbTypeFromVariant(type()) + " " +typeDetails();
}

QString dbForeignKey::get_CreateSqlSnippet()
{
    return "FOREIGN KEY (" + field + ") REFERENCES " + refTable + " (" + refField + ") " +onDelete + " " +onUpdate;
}

QString dbForeignKey::get_SelectSqpSnippet()
{
    QString sql;
    if( ! table.isEmpty()) sql = table + ".";
    sql += field + "=" + refTable + "." + refField;
    return sql;
}
