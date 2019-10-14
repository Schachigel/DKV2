#ifndef DBFIELD_H
#define DBFIELD_H

#include <QString>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlField>

struct dbtable;

struct dbfieldinfo
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
    dbfield(QString n, QVariant::Type t=QVariant::String,
            QString ti="", dbfield ref = dbfield(), refIntOption opt = refIntOption::non) :
          QSqlField(n, t), SqlTypeDetails(ti), option(opt)  { reference.tablename = ref.tableName(); reference.name = ref.name();}

    bool operator ==(const dbfield &b) const;
//    QString tableName()    const {return _tablename;}
//    void setTableName(QString n) {_tablename = n;}
//    QString name()         const {return _name;}
//    QVariant::Type type() const {return _type;}
    QString typeInfo()     const {return SqlTypeDetails;}
    dbfieldinfo getReferenzeInfo();
    // interface
    QString getCreateSqlSnippet();

private:
    // data
//    QString _tablename;
//    QString _name;
//    QVariant::Type _type;

    QString SqlTypeDetails;
    dbfieldinfo reference;
    refIntOption option;
};


#endif // DBFIELD_H
