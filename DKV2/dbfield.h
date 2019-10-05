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

struct dbfield
{
    enum refIntOption {
        non = 0,
        onDeleteCascade,
        onDeleteNull
    };

    QString tablename;
    QString name;
    QVariant::Type vType;

    int padding; // unused
    QString TypeInfo;
    dbfieldinfo reference;
    refIntOption option;
    int padding2; // unused

    explicit dbfield()
    {
        vType = QVariant::Type::Invalid;
    }
    dbfield (const dbfield& dbf) :
        tablename(dbf.tablename), name(dbf.name), vType(dbf.vType), TypeInfo(dbf.TypeInfo), reference(dbf.reference), option(dbf.option)
    {
    }
    dbfield(QString n, QVariant::Type t=QVariant::String,
            QString ti="", dbfieldinfo r = dbfieldinfo(), refIntOption opt = refIntOption::non) :
        name(n), vType(t), TypeInfo(ti), reference(r), option(opt)
    {
    }
    bool operator ==(const dbfield &b) const;
    QString CreateSQL();
    QSqlField toQSqlField();
    dbfieldinfo getInfo();
};


#endif // DBFIELD_H
