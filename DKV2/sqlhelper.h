#ifndef SQLHELPER_H
#define SQLHELPER_H

#include <QVariant>
#include <QSqlQuery>

#include "dbfield.h"

QString SelectQueryFromFields(const QVector<dbfield>& fields, const QString& where);

QSqlRecord ExecuteSingleRecordSql(const QVector<dbfield>& fields, const QString& where, const QString& con="");

QVariant   ExecuteSingleValueSql( const QString& s, const QString& connection="");
QVariant   ExecuteSingleValueSql( const QString& field, const QString& table, const QString& where, const QString& con="");

int getHighestTableId(const QString& tablename, const QString& con="");

QString JsonFromRecord( QSqlRecord r);

#endif // SQLHELPER_H
