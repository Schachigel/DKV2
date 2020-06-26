#ifndef SQLHELPER_H
#define SQLHELPER_H

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QDate>

#include "dbfield.h"
#include "dbstructure.h"

bool vTypesShareDbType( QVariant::Type t1, QVariant::Type t2);
QString dbInsertableString(QVariant v);
QString dbCreateTable_type(QVariant::Type t);
QString dbAffinityType(QVariant::Type t);

int rowCount(const QString& table);

bool tableExists(const QString& tablename, QSqlDatabase db = QSqlDatabase::database());
bool verifyTable( const dbtable& table, QSqlDatabase db);
bool ensureTable(const dbtable& table, QSqlDatabase db = QSqlDatabase::database());

QVariant executeSingleValueSql(const QString& sql, QSqlDatabase db = QSqlDatabase::database());
QVariant executeSingleValueSql(const QString& fieldName, const QString& tableName, const QString& where = "", QSqlDatabase db = QSqlDatabase::database());
QVariant executeSingleValueSql(const dbfield&, const QString& where, QSqlDatabase db=QSqlDatabase::database());

QString selectQueryFromFields(const QVector<dbfield>& fields,
                              const QVector<dbForeignKey> keys =QVector<dbForeignKey>(),
                              const QString& where ="", const QString& order ="");
// QVector<QVariant> executeSingleColumnSql( const QString field, const QString table, const QString& where);
QVector<QVariant> executeSingleColumnSql( const dbfield field, const QString& where="");
QSqlRecord executeSingleRecordSql(const QVector<dbfield>& fields, const QString& where ="");
QVector<QSqlRecord> executeSql(const QVector<dbfield>& fields, const QString& where="", const QString& order="");
bool executeSql(QString sql);

int getHighestRowId(const QString& tablename);

#endif // SQLHELPER_H
