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
    QVariant::Type type;
};

struct dbfield
{
    enum refIntOption {
        non = 0,
        onDeleteCascade,
        onDeleteNull
    };

    dbtable* table;
    QString name;
    QVariant::Type vType;

    int padding; // unused
    QString TypeInfo;
    dbfieldinfo reference;
    refIntOption option;
    int padding2; // unused

    explicit dbfield()
    {
        table = nullptr;
        vType = QVariant::Type::Invalid;
    }
    dbfield (const dbfield& dbf) :
        table(dbf.table), name(dbf.name), vType(dbf.vType), TypeInfo(dbf.TypeInfo), reference(dbf.reference), option(dbf.option)
    {
    }
    dbfield(dbtable* table, QString n, QVariant::Type t=QVariant::String,
            QString ti="", dbfieldinfo r = dbfieldinfo(), refIntOption opt = refIntOption::non) :
        table(table), name(n), vType(t), TypeInfo(ti), reference(r), option(opt)
    {
    }
    QString CreateFieldSQL();
    QSqlField getQSqlField();
    dbfieldinfo getInfo();
};


#endif // DBFIELD_H
