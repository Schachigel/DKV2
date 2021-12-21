#ifndef SQLHELPER_H
#define SQLHELPER_H

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QDate>

#include "dbfield.h"
#include "dbstructure.h"

extern const QString dbTypeName;
extern const int SQLITE_minimalRowId;

struct dbCloser
{   // RAII class for db connections
    dbCloser(const QString& c) : conName (c){}
    dbCloser(const dbCloser&) =delete;
    ~dbCloser(){
        if( QSqlDatabase::database(conName).isValid()){
            QList<QString> cl = QSqlDatabase::connectionNames();
            for( const auto& con : qAsConst(cl)) {
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

struct autoDb{
    enum accessType { read_only =0, read_write};
    autoDb(const QString& file, const QString& connect, accessType at=read_write) : closer(connect){
        db =QSqlDatabase::addDatabase(dbTypeName, closer.conName);
        db.setDatabaseName(file);
        if(at == read_only) db.setConnectOptions(qsl("QSQLITE_OPEN_READONLY"));
        if( not db.open()) {
            qCritical() << "could not open db conncection " << closer.conName;
            qCritical() << db.lastError();
        }
    }
    operator QSqlDatabase() const {return db;};
    QString conName() { return closer.conName;};
    dbCloser closer;
    QSqlDatabase db;
};

struct autoRollbackTransaction
{   // RAII class for db connections
    autoRollbackTransaction(const QString& con =QString()) : connection((con)) {
        QSqlDatabase::database(con).transaction();
    };
    autoRollbackTransaction(const autoRollbackTransaction&) =delete;
    void commit() { LOG_CALL;
        if( not comitted) QSqlDatabase::database(connection).commit();
        comitted =true;
    }
    ~autoRollbackTransaction() {
        if( not comitted)
            QSqlDatabase::database(connection).rollback();
    }
private:
    QString connection;
    bool comitted =false;
};

struct autoDetachDb
{   // RAII class for db file attachment
    autoDetachDb(const QString& a, const QString& conName)
        : _alias(a), _conname(conName)
    {

    }
    autoDetachDb(const autoDetachDb&) =delete;
    bool attachDb(const QString &filename);
    ~autoDetachDb();
    const QString alias(){return _alias;}
    const QString conname(){return _conname;}
private:
    QString _alias;
    QString _conname;
};


// bool vTypesShareDbType( QVariant::Type t1, QVariant::Type t2);
QString DbInsertableString(const QVariant &v);
QString dbCreateTable_type(const QVariant::Type t);
QString dbAffinityType(const QVariant::Type t);

int rowCount(const QString& table, const QSqlDatabase& db =QSqlDatabase::database());
bool tableExists(const QString& tablename, const QSqlDatabase& db = QSqlDatabase::database());
bool verifyTable( const dbtable& table, const QSqlDatabase& db);
bool ensureTable(const dbtable& table, const QSqlDatabase& db =QSqlDatabase::database());

#define fkh_on  true
#define fkh_off false
bool switchForeignKeyHandling(const QSqlDatabase& db, const QString& alias, bool OnOff =fkh_on);
bool switchForeignKeyHandling(const QSqlDatabase& db =QSqlDatabase::database(), bool OnOff =fkh_on);

QVariant executeSingleValueSql(const QString& sql, const QSqlDatabase& db = QSqlDatabase::database());
QVariant executeSingleValueSql(const QString& fieldName, const QString& tableName, const QString& where =QString(), const QSqlDatabase& db = QSqlDatabase::database());
QVariant executeSingleValueSql(const dbfield&, const QString& where, const QSqlDatabase& db=QSqlDatabase::database());

QString selectQueryFromFields(const QVector<dbfield>& fields,
                              const QVector<dbForeignKey>& keys =QVector<dbForeignKey>(),
                              const QString& where =QString(), const QString& order =QString());
// QVector<QVariant> executeSingleColumnSql( const QString field, const QString table, const QString& where);
QVector<QVariant> executeSingleColumnSql( const dbfield& field, const QString& where =QString());
QSqlRecord executeSingleRecordSql(const QVector<dbfield>& fields, const QString& where =QString(), const QString& order =QString());
QSqlRecord executeSingleRecordSql(const QString& sql);

QVector<QSqlRecord> executeSql(const QVector<dbfield>& fields, const QString& where =QString(), const QString& order =QString());
bool executeSql(const QString& sql, const QVariant& v, QVector<QSqlRecord>& result, const QSqlDatabase& db =QSqlDatabase::database());
bool executeSql(const QString& sql, QVector<QSqlRecord>& result);

bool executeSql_wNoRecords(const QString& sql, const QSqlDatabase& db =QSqlDatabase::database());
bool executeSql_wNoRecords(const QString& sql, const QVariant& v, const QSqlDatabase& db = QSqlDatabase::database());
bool executeSql_wNoRecords(const QString& sql, const QVector<QVariant>& v, const QSqlDatabase& db = QSqlDatabase::database());

int getHighestRowId(const QString& tablename);

struct dbViewDev{
    const QString name;
    const QString sql;
};
bool createDBView(const QString& name, const QString& sql, const QSqlDatabase& db = QSqlDatabase::database());
bool deleteDBView(const QString& name, const QSqlDatabase& db = QSqlDatabase::database());
bool createDBViews( const QMap<QString, QString>& views, const QSqlDatabase& db);

struct temporaryDbView
{
    temporaryDbView(const QString& name, const QString& sql, const QSqlDatabase& db = QSqlDatabase::database())
        : name(name) {
        Q_ASSERT(createDBView(name, sql, db));
    }
    ~temporaryDbView() {
        deleteDBView(name);
    }
    const QString name;
};

#endif // SQLHELPER_H
