#ifndef SQLHELPER_H
#define SQLHELPER_H

#include <QVariant>
#include <QDate>
#include <QSqlQuery>
#include <QSqlRecord>
#include "dbfield.h"
#include "dbstructure.h"

bool tableExists(const QString& tablename, QSqlDatabase db = QSqlDatabase::database());

int rowCount(const QString& table);

bool createTables( const dbstructure& structure, QSqlDatabase db = QSqlDatabase::database());

bool ensureTable(const dbtable& table, QSqlDatabase db = QSqlDatabase::database());

QVector<QString> getFieldsFromTablename(const QString& tablename, QSqlDatabase db = QSqlDatabase::database());

QString SelectQueryFromFields(const QVector<dbfield>& fields, const QString& where);

QSqlRecord ExecuteSingleRecordSql(const QVector<dbfield>& fields, const QString& where);

QVariant ExecuteSingleValueSql(const QString& s, QSqlDatabase db = QSqlDatabase::database());
QVariant   ExecuteSingleValueSql( const QString& field, const QString& table, const QString& where, QSqlDatabase db = QSqlDatabase::database());

int ExecuteUpdateSql(const QString& table, const QString& field, const QVariant& newValue, const QString& where);

int getHighestTableId(const QString& tablename);

QString JsonFromRecord( QSqlRecord r);

template <class T>
T sqlVal(QSqlQuery sql, QString fname);

#endif // SQLHELPER_H
