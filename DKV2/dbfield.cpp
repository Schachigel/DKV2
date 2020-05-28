#include "helper.h"
#include "sqlhelper.h"

#include "dbfield.h"
// #include "dbtable.h"

/* static */ bool dbfield::isSupportedType(QVariant::Type t)
{
    if( t == QVariant::LongLong) return true; // index col
    if( t == QVariant::Int)      return true; // money in ct
    if( t == QVariant::Date)     return true;
    if( t == QVariant::String)   return true;
    if( t == QVariant::Bool)     return true;
    return false;
}

bool dbfield::operator ==(const dbfield &b) const
{
    return ((tableName() == b.tableName())
            && (name() == b.name())
            && (type() == b.type()));
}

QString dbfield::get_CreateSqlSnippet()
{
    return name()
            + " " + dbCreateTable_type(type())
            + (primaryKey ? " PRIMARY KEY" : "")
            + (!typeDetails().isEmpty() ? " " + typeDetails() : "")
            + (isAutoValue()? " AUTOINCREMENT" : "")
            + ((requiredStatus()==Required)? " NOT NULL" : "")
            + (defaultValue().isValid() ? " DEFAULT "+ dbInsertableString(defaultValue()) : "");
}

//////////////////////
/// dbForeignKey
//////////////////////

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
