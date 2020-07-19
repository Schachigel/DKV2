#ifndef DBFIELD_H
#define DBFIELD_H

#include <QSqlField>
#include <QString>
#include <QStringLiteral>
#define qsl(x) QStringLiteral(x)

struct dbtable;

class dbfield : public QSqlField
{
public: // types
    // constr. destr. & access fu
    explicit dbfield() : QSqlField(){}
    dbfield(QString name,
            QVariant::Type type=QVariant::String,
            QString td=qsl(""))
     :  QSqlField(name), SqlTypeDetails(td)
    {
        Q_ASSERT(isSupportedType(type));
        Q_ASSERT( ! name.contains(qsl("-")));
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
    dbfield setUnique(bool u=true){unique = u; return *this;}
    dbfield setPrimaryKey(bool p=true){ primaryKey = p; return *this;}
    dbfield setNotNull(bool nn=true){ setRequired(nn); return *this;}
    dbfield setDefault(QVariant d){ setDefaultValue(d); return *this;}
    dbfield setAutoInc(bool a=true){ setAutoValue(a); return *this;}
    // somewhat a helper
    static bool isSupportedType(QVariant::Type t);
    private:
    // data
    bool unique = false;
    bool primaryKey=false;
    QString SqlTypeDetails;
    QVariant::Type outputType;
};

struct dbForeignKey
{
    // const. destr. & access fu
    dbForeignKey(dbfield local, dbfield parent, QString onDelete =QString(), QString onUpdate=QString())
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
