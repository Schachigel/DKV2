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

class dbfield
{
public: // types
    enum refIntOption {
        non = 0,
        onDeleteCascade,
        onDeleteNull
    };
    // constr. destr. & access fu
    explicit dbfield() : vType(QVariant::Type::Invalid){}
    dbfield(QString n, QVariant::Type t=QVariant::String,
            QString ti="", dbfieldinfo r = dbfieldinfo(), refIntOption opt = refIntOption::non) :
          name(n), vType(t), SqlTypeDetails(ti), reference(r), option(opt)  {}
    bool operator ==(const dbfield &b) const;
    QString Tablename()    const {return tablename;}
    void setTablename(QString n) {tablename = n;}
    QString Name()         const {return name;}
    QVariant::Type VType() const {return vType;}
    QString TypeInfo()     const {return SqlTypeDetails;}
    dbfieldinfo getReferenzeInfo();
    // interface
    QString getCreateSqlSnippet();

private:
    // data
    QString tablename;
    QString name;
    QVariant::Type vType;

    int padding; // unused
    QString SqlTypeDetails;
    dbfieldinfo reference;
    refIntOption option;
    int padding2; // unused
};


#endif // DBFIELD_H
