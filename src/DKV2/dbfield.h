#ifndef DBFIELD_H
#define DBFIELD_H

#include <iso646.h>

#include "helper.h"

class dbfield : public QSqlField
{
public: // types
    // constr. destr. & access fu
    explicit dbfield() : QSqlField(){}
    dbfield(const QString& name,
            const QMetaType::Type type=QMetaType::QString)
        :  QSqlField(name, QMetaType(type))
    {
        Q_ASSERT(isSupportedDBType(type));
        Q_ASSERT( not name.contains(qsl("-")));
        setMetaType(QMetaType(type));
        // autoinc will prevent the reuse of prim.key values after deletion
    }
    bool operator ==(const dbfield &b) const;
    // interface
    QString get_CreateSqlSnippet() const;
    // setter of field properties
    dbfield setUnique(const bool u=true){ Q_ASSERT_X(defaultValue ().isNull (),"setUnique" , "unique values should not have a default value"); unique =u; return *this;}
    dbfield setPrimaryKey(const bool p=true){ primaryKey = p; return *this;}
    dbfield setNotNull(const bool nn=true){ setRequired(nn); return *this;}
    dbfield setDefault(const QVariant& defaultvalue){ Q_ASSERT(unique == false); setDefaultValue(defaultvalue); return *this;}
    dbfield setAutoInc(const bool a=true){ if(a) setPrimaryKey (); setAutoValue(a); return *this;}
    dbfield setDefaultNow() { setDefaultValue(QVariant()); timeStamp =true; return *this;}
private:
    // somewhat a helper
    static bool isSupportedDBType(const QMetaType::Type t);
    // data
    bool unique = false;
    bool primaryKey=false;
    bool timeStamp =false;
};

enum ODOU_Action { NO_ACTION, RESTRICT, SET_NULL, SET_DEFAULT, CASCADE};

struct dbForeignKey
{
    static const QVector<QString> ODOU_Actions;
    // const. destr. & access fu
    dbForeignKey(const dbfield& local, const dbfield& parent, ODOU_Action delAction =NO_ACTION, ODOU_Action updAction =NO_ACTION)
    {
        table = local.tableName();
        field = local.name();
        refTable = parent.tableName();
        refField = parent.name();
        onDelete = qsl("ON DELETE ") +ODOU_Actions.at(delAction);
        onUpdate = qsl("ON UPDATE ") +ODOU_Actions.at(updAction);
    }
    dbForeignKey(const dbfield& local, const QString& parentTable, const QString& parentField, ODOU_Action delAction =NO_ACTION, ODOU_Action updAction =NO_ACTION)
    {
        table = local.tableName();
        field = local.name();
        refTable = parentTable;
        refField = parentField;
        onDelete = qsl("ON DELETE ") +ODOU_Actions.at(delAction);
        onUpdate = qsl("ON UPDATE ") +ODOU_Actions.at(updAction);
    }

    // interface
    QString get_CreateSqlSnippet();
    QString get_SelectSqlWhereClause();

private:
    QString table;
    QString field;
    QString refTable;
    QString refField;
    QString onDelete;
    QString onUpdate;
};

#endif // DBFIELD_H
