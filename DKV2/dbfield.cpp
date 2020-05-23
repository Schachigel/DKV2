#include "helper.h"

#include "dbfield.h"
#include "dbtable.h"

bool dbfield::operator ==(const dbfield &b) const
{
    return ((tableName() == b.tableName())
            && (name() == b.name())
            && (type() == b.type()));
}

QString dbfield::get_CreateSqlSnippet()
{   //LOG_CALL_W(name());
    return name()
            + " " + dbTypeFromVariantType(type())
            + (!typeDetails().isEmpty() ? " " + typeDetails() : "")
            + (isAutoValue()? " AUTOINCREMENT" : "")
            + ((requiredStatus()==Required)? " NOT NULL" : "")
            + (defaultValue().isValid() ? " DEFAULT "+ dbInsertableStringFromVariant(defaultValue()) : "");
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
