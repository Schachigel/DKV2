
#include "dbfield.h"
#include "dbtable.h"

bool dbfield::operator ==(const dbfield &b) const
{
    return ((tablename == b.tablename)
            && (name == b.name)
            && (vType == b.vType));
}

// local helper
QVariant::Type dbCompatibleVType(QVariant::Type t)
{
    switch( t)
    {
    case QVariant::String:
    case QVariant::Int:
    case QVariant::Double:
        return t;
    case QVariant::Date:
        return QVariant::String; // sadly ...
    case QVariant::Bool:
        return QVariant::Int;
    default:
        Q_ASSERT(!bool("invalid database type"));
    }
}

QString dbTypeFromVariant(QVariant::Type t)
{
    switch( t)
    {
    case QVariant::String:
        return "STRING";
    case QVariant::Int:
        return "INTEGER";
    case QVariant::Double:
        return "REAL";
    case QVariant::Date:
        return "STRING"; // sadly ...
    case QVariant::Bool:
        return "INTEGER";
    default:
        Q_ASSERT(!bool("invalid database type"));
    }
}

QString dbfield::getCreateSqlSnippet()
{
    QString s( "[" + name + "] " + dbTypeFromVariant(vType) + " " +TypeInfo());
    if( reference.name.isEmpty())
        return s;
    s += " FOREIGN_KEY REFERENCES [" + reference.tablename +"](" + reference.name + ")";
    if( option)
    {
        if( option == refIntOption::onDeleteNull)
            s += " ON DELETE SET NULL";
        if( option == refIntOption::onDeleteCascade)
            s += " ON DELETE CASCADE";
    }
    return s;
}

dbfieldinfo dbfield::getReferenzeInfo()
{
    return dbfieldinfo{tablename, name};
}

