#include "dbfield.h"

#include "helper_core.h"
#include "helpersql.h"

/* static */ bool dbfield::isSupportedDBType(QMetaType::Type t)
{
    if(t == QMetaType::LongLong) return true; // index col
    if(t == QMetaType::Int)      return true; // money in ct
    if(t == QMetaType::QDate)     return true;
    if(t == QMetaType::QDateTime) return true; // booking date & time
    if(t == QMetaType::QString)   return true;
    if(t == QMetaType::Bool)     return true;
    if(t == QMetaType::Double)   return true;
    Q_ASSERT_X(true, "isSupportedType: ", "unsupported database data type");
    return false;
}

bool dbfield::operator ==(const dbfield &b) const
{
    return ((tableName() == b.tableName())
            and (name()  == b.name())
            and (metaType()  == b.metaType()));
}

QString dbfield::get_CreateSqlSnippet() const
{
    return name()
           + qsl(" ") + dbCreatetable_type(metaType())
            + (primaryKey ? qsl(" PRIMARY KEY") : "")
            + (isAutoValue()? qsl(" AUTOINCREMENT") : "")
            + ((requiredStatus()==Required)? qsl(" NOT NULL") : "")
            + ((unique ? qsl(" UNIQUE") : "" ))
            + (defaultValue().isValid()
               ? qsl(" DEFAULT ")+ DbInsertableString(defaultValue())
               : (timeStamp ? qsl( " DEFAULT CURRENT_TIMESTAMP") : ""));
}

//////////////////////
/// dbForeignKey
//////////////////////

const QVector<QString> dbForeignKey::ODOU_Actions ={
    qsl("NO ACTION"),
    qsl("RESTRICT"),
    qsl("SET NULL"),
    qsl("SET DEFAULT"),
    qsl("CASCADE")};

QString dbForeignKey::get_CreateSqlSnippet()
{
    Q_ASSERT_X( (not refTable.isEmpty()) && (not refField.isEmpty()), "creating foreignKey sql", "table and name must not be empty");
    return qsl("FOREIGN KEY (%1) REFERENCES %2 (%3) %4 %5").arg(field, refTable, refField, onDelete, onUpdate);
}
