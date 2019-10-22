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
            QString td="", dbfield ref = dbfield(), refIntOption opt = refIntOption::non);
//     :  QSqlField(name, type), SqlTypeDetails(td), option(opt);

    bool operator ==(const dbfield &b) const;
    QString typeInfo()     const {return SqlTypeDetails;}
    refFieldInfo getReferenzeInfo() const;
    // interface
    QString getCreateSqlSnippet();

private:
    // data
    QString SqlTypeDetails;
    refFieldInfo reference;
    refIntOption option;
};


#endif // DBFIELD_H
