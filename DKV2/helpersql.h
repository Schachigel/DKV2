#ifndef SQLHELPER_H
#define SQLHELPER_H

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QDate>

#include "dbfield.h"
#include "dbstructure.h"

struct dbCloser
{   // RAII class for db connections
    dbCloser(QString c) : conName (c){}
    ~dbCloser(){
        if( QSqlDatabase::database(conName).isValid()){
            QList<QString> cl = QSqlDatabase::connectionNames();
            for( auto con : cl) {
                if( con == conName) {
                    QSqlDatabase::database(con).close();
                    QSqlDatabase::removeDatabase(con);
                    qInfo() << "closed db connection " << con;
                }
                else qInfo() << "unclosed db connection: " << con;
            }
            QSqlDatabase::database(conName).removeDatabase(conName);
        }
    }
    QString conName;
};

struct autoRollbackTransaction
{   // RAII class for db connections
    autoRollbackTransaction(QString con =QString()) : connection((con)) {
        QSqlDatabase::database(con).transaction();
    };
    void commit() {
        QSqlDatabase::database(connection).commit();
        comitted =true;
    }
    ~autoRollbackTransaction() {
        if( ! comitted)
            QSqlDatabase::database(connection).rollback();
    }
private:
    QString connection;
    bool comitted =false;
};

struct autoDetachDb
{   // RAII class for db file attachment
    autoDetachDb(QString a, QString conName)
        : alias(a), conname(conName)
    {

    }
    bool attachDb(QString filename);
    ~autoDetachDb();

    QString alias;
    QString conname;
};


// bool vTypesShareDbType( QVariant::Type t1, QVariant::Type t2);
QString DbInsertableString(QVariant v);
QString dbCreateTable_type(QVariant::Type t);
QString dbAffinityType(QVariant::Type t);

int rowCount(const QString& table, QSqlDatabase db =QSqlDatabase::database());

bool tableExists(const QString& tablename, QSqlDatabase db = QSqlDatabase::database());
bool verifyTable( const dbtable& table, QSqlDatabase db);
bool ensureTable(const dbtable& table, QSqlDatabase db = QSqlDatabase::database());

QVariant executeSingleValueSql(const QString& sql, QSqlDatabase db = QSqlDatabase::database());
QVariant executeSingleValueSql(const QString& fieldName, const QString& tableName, const QString& where =QString(), QSqlDatabase db = QSqlDatabase::database());
QVariant executeSingleValueSql(const dbfield&, const QString& where, QSqlDatabase db=QSqlDatabase::database());

QString selectQueryFromFields(const QVector<dbfield>& fields,
                              const QVector<dbForeignKey> keys =QVector<dbForeignKey>(),
                              const QString& where =QString(), const QString& order =QString());
// QVector<QVariant> executeSingleColumnSql( const QString field, const QString table, const QString& where);
QVector<QVariant> executeSingleColumnSql( const dbfield field, const QString& where =QString());
QSqlRecord executeSingleRecordSql(const QVector<dbfield>& fields, const QString& where =QString(), const QString& order =QString());
QVector<QSqlRecord> executeSql(const QVector<dbfield>& fields, const QString& where =QString(), const QString& order =QString());
bool executeSql(const QString& sql, const QVariant& v, QVector<QSqlRecord>& result);
bool executeSql(const QString& sql, const QVector<QVariant>& v, QVector<QSqlRecord>& result);
bool executeSql_wNoRecords(QString sql, QVariant v, QSqlDatabase db = QSqlDatabase::database());
bool executeSql_wNoRecords(QString sql, QVector<QVariant> v =QVector<QVariant>(), QSqlDatabase db = QSqlDatabase::database());

int getHighestRowId(const QString& tablename);

#endif // SQLHELPER_H
