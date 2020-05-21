#ifndef DBTABLE_H
#define DBTABLE_H

#include <QSqlDatabase>

#include "sqlhelper.h"
#include "dbfield.h"

class dbstructure;

class dbtable
{
    friend class dbstructure;
public:
    // constr. destr. & access fu
    dbtable(QString n="") : name(n) {}
    QString Name() const {return name;}
    QVector<dbfield> Fields() const { return fields;}
    dbfield operator[](QString s) const;
    // interface
    dbtable append(const dbfield&);
    void setUnique(const QVector<dbfield>& fs);
    bool create(QSqlDatabase db = QSqlDatabase::database()) const;
private:
    QString name;
    QString unique;
    QVector<dbfield> fields;
    // helper
    QString createTableSql() const;
};

#endif // DBTABLE_H
