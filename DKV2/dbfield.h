#ifndef DBFIELD_H
#define DBFIELD_H

#include <iso646.h>

#include "pch.h"

#include "helper.h"

struct dbtable;

class dbfield : public QSqlField
{
public: // types
    // constr. destr. & access fu
    explicit dbfield() : QSqlField(){}
    dbfield(const QString& name,
            const QVariant::Type type=QVariant::String,
            const QString& td=qsl(""))
     :  QSqlField(name), SqlTypeDetails(td)
    {
        Q_ASSERT(isSupportedType(type));
        Q_ASSERT( not name.contains(qsl("-")));
        outputType = type;
        setType(type);
        SqlTypeDetails = SqlTypeDetails.toUpper();
        setAutoValue(SqlTypeDetails.contains(qsl("AUTOINCREMENT")));
        SqlTypeDetails = SqlTypeDetails.replace(qsl("AUTOINCREMENT"), qsl("")).trimmed();
        setRequired(SqlTypeDetails.contains(qsl("NOT NULL")));
        SqlTypeDetails = SqlTypeDetails.replace(qsl("NOT NULL"), qsl("")).trimmed();
        setPrimaryKey(SqlTypeDetails.contains(qsl("PRIMARY KEY")));
        SqlTypeDetails = SqlTypeDetails.replace(qsl("PRIMARY KEY"), qsl("")).trimmed();
        setUnique(SqlTypeDetails.contains(qsl("UNIQUE")));
        SqlTypeDetails = SqlTypeDetails.replace(qsl("UNIQUE"), qsl("")).trimmed();
    }
    bool operator ==(const dbfield &b) const;
    QString typeDetails()     const {return SqlTypeDetails;}
    // interface
    QString get_CreateSqlSnippet() const;
    dbfield setUnique(const bool u=true){unique = u; return *this;}
    dbfield setPrimaryKey(const bool p=true){ primaryKey = p; return *this;}
    dbfield setNotNull(const bool nn=true){ setRequired(nn); return *this;}
    dbfield setDefault(const QVariant& d){ setDefaultValue(d); return *this;}
    dbfield setAutoInc(const bool a=true){ setAutoValue(a); return *this;}
    dbfield setDefaultNow() {timeStamp =true; return *this;}
    // somewhat a helper
    static bool isSupportedType(const QVariant::Type t);
    private:
    // data
    bool unique = false;
    bool primaryKey=false;
    bool timeStamp =false;
    QString SqlTypeDetails;
    QVariant::Type outputType;
};

struct dbForeignKey
{
    // const. destr. & access fu
    dbForeignKey(const dbfield& local, const dbfield& parent, const QString& onDelete =QString(), const QString& onUpdate=QString())
        : onDelete(onDelete), onUpdate( onUpdate)
    {
        table = local.tableName();
        field = local.name();
        refTable = parent.tableName();
        refField = parent.name();
    }
    dbForeignKey(const dbfield& local, const QString& parentTable, const QString& parentField, const QString& onDelete =QString(), const QString& onUpdate=QString())
        : onDelete(onDelete), onUpdate( onUpdate)
    {
        table = local.tableName();
        field = local.name();
        refTable = parentTable;
        refField = parentField;
    }

    // interface
    QString get_CreateSqlSnippet();
    QString get_SelectSqpSnippet();

private:
    QString table;
    QString field;
    QString refTable;
    QString refField;
    QString onDelete;
    QString onUpdate;
};

#endif // DBFIELD_H
