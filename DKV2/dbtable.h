#ifndef DBTABLE_H
#define DBTABLE_H

#include <QStringLiteral>

#include <QSqlDatabase>

#include "dbfield.h"

class dbstructure;

struct dbtable
{
    friend class dbstructure;
    // constr. destr. & access fu
    dbtable(QString n=qsl("")) : name(n) {}
    void setName(QString n) { name = n;}
    QString Name() const {return name;}
    QVector<dbfield> Fields() const { return fields;}
    QVector<dbForeignKey> ForeignKeys() const {return foreignKeys;}
    dbfield operator[](QString s) const;
    // interface
    dbtable append(const dbfield&);
    dbtable append(const dbForeignKey&);
    void setUnique(const QVector<dbfield>& fs);
    bool create(QSqlDatabase db = QSqlDatabase::database()) const;
private:
    QString name;
    QString unique;
    // QVector<QString> uniqueSet;
    QVector<dbForeignKey> foreignKeys;
    QVector<dbfield> fields;
    // helper
    QString createTableSql() const;
};

#endif // DBTABLE_H
