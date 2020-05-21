#ifndef SQLHELPER_H
#define SQLHELPER_H

#include <QVariant>
#include <QDate>
#include <QSqlQuery>
#include <QSqlRecord>
#include "dbfield.h"
#include "dbstructure.h"

bool typesAreDbCompatible( QVariant::Type t1, QVariant::Type t2);

bool tableExists(const QString& tablename, QSqlDatabase db = QSqlDatabase::database());

int rowCount(const QString& table);

bool ensureTable(const dbtable& table, QSqlDatabase db = QSqlDatabase::database());

QString selectQueryFromFields(const QVector<dbfield>& fields, const QString& where = "");

QSqlRecord executeSingleRecordSql(const QVector<dbfield>& fields, const QString& where);

QVariant executeSingleValueSql(const QString& s, QSqlDatabase db = QSqlDatabase::database());
QVariant executeSingleValueSql(const QString& field, const QString& table, const QString& where = "", QSqlDatabase db = QSqlDatabase::database());

QVector<QVariant> executeSingleColumnSql( const QString& field, const QString& table, const QString& where="");

//int ExecuteUpdateSql(const QString& table, const QString& field, const QVariant& newValue, const QString& where);

int getHighestTableId(const QString& tablename);

QString JsonFromRecord( QSqlRecord r);

template <class T>
T sqlVal(QSqlQuery sql, QString fname);

#endif // SQLHELPER_H
