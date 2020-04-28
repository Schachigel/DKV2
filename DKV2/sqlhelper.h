#ifndef SQLHELPER_H
#define SQLHELPER_H

#include <QVariant>
#include <QDate>
#include <QSqlQuery>
#include <QSqlRecord>

#include "dbfield.h"

QSqlDatabase defaultDb();

bool tableExists(const QString& tablename, QSqlDatabase db = defaultDb());

QVector<QString> getFieldsFromTablename(const QString& tablename, QSqlDatabase db= defaultDb());

QString SelectQueryFromFields(const QVector<dbfield>& fields, const QString& where);

QSqlRecord ExecuteSingleRecordSql(const QVector<dbfield>& fields, const QString& where, QSqlDatabase db= defaultDb());

QVariant ExecuteSingleValueSql(const QString& s, QSqlDatabase db = defaultDb());
QVariant   ExecuteSingleValueSql( const QString& field, const QString& table, const QString& where, QSqlDatabase db=defaultDb());

int ExecuteUpdateSql(const QString& table, const QString& field, const QVariant& newValue, const QString& where, QSqlDatabase db=defaultDb());

int getHighestTableId(const QString& tablename, QSqlDatabase db = defaultDb());

QString JsonFromRecord( QSqlRecord r);

template <class T>
T sqlVal(QSqlQuery sql, QString fname);

#endif // SQLHELPER_H
