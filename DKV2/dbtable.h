#ifndef DBTABLE_H
#define DBTABLE_H

#include "dbfield.h"

//
// describe a table, so that it can be created in sqlite using SQL
// features
// -fields (dbfield)
// -foreign keys
// -multi field uniqueness
//

struct dbtable
{
    // constr. destr. & access fu
    dbtable(const QString& n="") : name(n) {}
    void setName(const QString& n) { name = n;}
    QString Name() const {return name;}
    const QVector<dbfield> Fields() const { return fields;}
    const QVector<dbForeignKey> ForeignKeys() const {return foreignKeys;}
    const dbfield operator[](const QString& s) const;
    // interface
    dbtable append(const dbfield&);
    dbtable append(const dbForeignKey&);
    void setUnique(const QVector<dbfield>& fs);
    bool create(const QSqlDatabase &db = QSqlDatabase::database()) const;

    // helper; public for testing
    QString createTableSql() const;
private:
    QString name;
    QString unique;
    QVector<dbfield> fields;
    QVector<dbForeignKey> foreignKeys;
};

#endif // DBTABLE_H
