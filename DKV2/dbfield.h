#ifndef DBFIELD_H
#define DBFIELD_H

#include <QString>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlField>

struct dbtable;

struct refFieldInfo
{
    QString tablename;
    QString name;
};

class dbfield : public QSqlField
{
public: // types
    enum refIntOption {
        non = 0,
        onDeleteCascade,
        onDeleteNull
    };
    // constr. destr. & access fu
    explicit dbfield() : QSqlField(){}
    dbfield(QString name,
            QVariant::Type type=QVariant::String,
            QString td="")
     :  QSqlField(name, type), SqlTypeDetails(td)
    {
        td = td.toUpper();
    }
    bool operator ==(const dbfield &b) const;
    QString typeDetails()     const {return SqlTypeDetails;}
    refFieldInfo getReferenzeInfo() const;
    // interface
    QString get_CreateSqlSnippet();

private:
    // data
    QString SqlTypeDetails;
};

struct dbForeignKey
{
    // const. destr. & access fu
    dbForeignKey(dbfield local, dbfield parent, QString onDelete ="", QString onUpdate="")
        : onDelete(onDelete), onUpdate( onUpdate)
    {
        table = local.tableName();
        field = local.name();
        refTable = parent.tableName();
        refField = parent.name();
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
