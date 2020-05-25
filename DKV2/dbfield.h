#ifndef DBFIELD_H
#define DBFIELD_H

#include <QSqlField>
#include <QString>

struct dbtable;

class dbfield : public QSqlField
{
public: // types
    // constr. destr. & access fu
    explicit dbfield() : QSqlField(){}
    dbfield(QString name,
            QVariant::Type type=QVariant::String,
            QString td="")
     :  QSqlField(name), SqlTypeDetails(td)
    {
        Q_ASSERT(isSupportedType(type));
        Q_ASSERT( ! name.contains("-"));
        outputType = type;
        setType(type);

        SqlTypeDetails = SqlTypeDetails.toUpper();
        setAutoValue(SqlTypeDetails.contains("AUTOINCREMENT"));
        SqlTypeDetails = SqlTypeDetails.replace("AUTOINCREMENT", "").trimmed();
        setRequired(SqlTypeDetails.contains("NOT NULL"));
        SqlTypeDetails = SqlTypeDetails.replace("NOT NULL", "").trimmed();
        if( SqlTypeDetails.contains("PRIMARY KEY"))
            setPrimaryKey(), SqlTypeDetails = SqlTypeDetails.replace("PRIMARY KEY", "").trimmed();
    }
    bool operator ==(const dbfield &b) const;
    QString typeDetails()     const {return SqlTypeDetails;}
    // interface
    QString get_CreateSqlSnippet();
    dbfield setPrimaryKey(){ primaryKey = true; return *this;}
    dbfield setNotNull(){ setRequired(true); return *this;}
    dbfield setDefault(QVariant v){ setDefaultValue(v); return *this;}
    dbfield setAutoInc(){ setAutoValue(true); return *this;}
    // somewhat a helper
    static bool isSupportedType(QVariant::Type t);
    private:
    // data
    bool primaryKey=false;
    QString SqlTypeDetails;
    QVariant::Type outputType;
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
