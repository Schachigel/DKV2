#ifndef SQLHELPER_H
#define SQLHELPER_H

#include "helper_core.h"
#include "dbtable.h"

inline const QString dbTypeName{qsl("QSQLITE")};
typedef qlonglong tableindex_t;
inline const tableindex_t SQLITE_minimalRowId =1;
inline const tableindex_t SQLITE_invalidRowId =-1;
inline bool isValidRowId(tableindex_t i) {
    return i not_eq SQLITE_invalidRowId; // id / rowid could have other values, if not autoinc
}

QString DbInsertableString(const QVariant &v);
QString dbCreatetable_type(const QMetaType t);
QString dbAffinityType(const QMetaType t);

int rowCount(const QString& table, const QString& where ="", const QSqlDatabase& db =QSqlDatabase::database());
bool tableExists(const QString& tablename, const QSqlDatabase& db = QSqlDatabase::database());
bool verifyTable( const dbtable& table, const QSqlDatabase& db);
bool ensureTable(const dbtable& table, const QSqlDatabase& db =QSqlDatabase::database());

bool executeSql(const QString& sql, QVector<QSqlRecord>& result, const QSqlDatabase& db =QSqlDatabase::database());
bool executeSql(const QString& sql, const QVector<QPair<QString, QVariant>>& params, QVector<QSqlRecord>& result, const QSqlDatabase& db =QSqlDatabase::database());
bool executeSql(const QString& sql, const QVector<QVariant>& params,                 QVector<QSqlRecord>& result, const QSqlDatabase& db =QSqlDatabase::database());


QVector<QSqlRecord> executeSql(const QVector<dbfield>& fields, const QString& where =QString(), const QString& order =QString(), const QSqlDatabase& db =QSqlDatabase::database());

#define fkh_on  true
#define fkh_off false
bool getForeignKeyHandlingStatus(const QString& alias =QString(), const QSqlDatabase& db =QSqlDatabase::database());
//bool switchForeignKeyHandling(const QSqlDatabase& db, const QString& alias, bool OnOff =fkh_on);
bool switchForeignKeyHandling(bool OnOff =fkh_on, const QSqlDatabase& db =QSqlDatabase::database());
bool switchForeignKeyHandling(bool OnOff, const QString& alias, const QSqlDatabase& db=QSqlDatabase::database ());

QString selectQueryFromFields(const QVector<dbfield>& fields,
                              const QString& where =QString(), const QString& order =QString());

QVariant executeSingleValueSql(const QString& sql, const QSqlDatabase& db =QSqlDatabase::database());
QVariant executeSingleValueSql(const QString& sql, const QVector<QVariant> params, const QSqlDatabase& db =QSqlDatabase::database());
QVariant executeSingleValueSql(const QString& fieldName, const QString& tableName, const QString& where =QString(), const QSqlDatabase& db = QSqlDatabase::database());
QVariant executeSingleValueSql(const dbfield&, const QString& where, const QSqlDatabase& db=QSqlDatabase::database());

// QVector<QVariant> executeSingleColumnSql( const QString field, const QString table, const QString& where);
QVector<QVariant> executeSingleColumnSql(const dbfield& dbField, const QString& where =QString());

QSqlRecord executeSingleRecordSql(const QString& sql, const QSqlDatabase& db =QSqlDatabase::database ());
QSqlRecord executeSingleRecordSql(const QVector<dbfield>& fields, const QString& where =QString(), const QString& order =QString(), const QSqlDatabase& db=QSqlDatabase::database());

bool executeSql_wNoRecords(const QString& sql, const QSqlDatabase& db =QSqlDatabase::database());
bool executeSql_wNoRecords(const QString& sql, const QVariant& v, const QSqlDatabase& db = QSqlDatabase::database());
bool executeSql_wNoRecords(const QString& sql, const QVector<QVariant>& v, const QSqlDatabase& db = QSqlDatabase::database());

int getHighestRowId(const QString& tablename);

//////////////////////////////
//   RAII classes for db handling
//////////////////////////////

struct dbCloser
{   // RAII class for db connections
    dbCloser(const QString& connectionString) : conName (connectionString){}
    dbCloser(const dbCloser&) =delete;
    ~dbCloser(){
        if( QSqlDatabase::database(conName).isValid()){
            QList<QString> cl = QSqlDatabase::connectionNames();
            for( const auto& con : std::as_const(cl)) {
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
    operator QSqlDatabase() const {return db;}
    QString conName() { return closer.conName;}
    dbCloser closer;
    QSqlDatabase db;
};

struct autoRollbackTransaction
{   // RAII class for db connections
    autoRollbackTransaction(const QString& con =QString()) : connection((con)) {
        QSqlDatabase::database(con).transaction();
    };
    autoRollbackTransaction(const autoRollbackTransaction&) =delete;
    bool commit() { LOG_CALL;
        if( not committed) {
            committed =QSqlDatabase::database(connection).commit();
        }
        return committed;
    }
    ~autoRollbackTransaction() {
        if( not committed)
            QSqlDatabase::database(connection).rollback();
    }
private:
    QString connection;
    bool committed =false;
};

class SavepointGuard
{
public:
    explicit SavepointGuard(const QString& name)
        : m_name(name)
    {
        executeSql_wNoRecords(
            qsl("SAVEPOINT %1").arg(m_name));
    }

    SavepointGuard(const SavepointGuard&) = delete;
    SavepointGuard& operator=(const SavepointGuard&) = delete;

    void commit()
    {
        if (!m_committed) {
            executeSql_wNoRecords(
                qsl("RELEASE SAVEPOINT %1").arg(m_name));
            m_committed = true;
        }
    }

    ~SavepointGuard()
    {
        if (!m_committed) {
            executeSql_wNoRecords(
                qsl("ROLLBACK TO %1").arg(m_name));
            executeSql_wNoRecords(
                qsl("RELEASE SAVEPOINT %1").arg(m_name));
        }
    }

private:
    QString m_name;
    bool m_committed = false;
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

void closeAllDatabaseConnections();

//////////////////////////////
struct dbViewDev{
    const QString name;
    const QString sql;
};

namespace
{
const bool persistentView =false;
const bool temporaryView  =true;

inline bool deleteDbView(const QString& name, const QSqlDatabase& db = QSqlDatabase::database())
{
    if( executeSql_wNoRecords(qsl("DROP VIEW IF EXISTS '%1'").arg(name), db)) {
        RETURN_OK( true, qsl("deleteDbView: deleteDbView: dropped view: "), name);
    }
    RETURN_OK( false, "deleteDbView: drop view failed: ", name);
}

inline bool createDbView(const QString& name, const QString& sql, bool temporary, const QSqlDatabase& db = QSqlDatabase::database())
{
/* view deletion / insertion costs only single digit ms - so it is not worth reading and comparing views to avoid deletion
*/
    deleteDbView (name, db);
    QString createViewSql = "CREATE %1 VIEW '%2' AS " + sql;
    createViewSql = createViewSql.arg((temporary ? qsl("TEMP") : QString()), name);
    if( executeSql_wNoRecords(createViewSql, db)) {
        qInfo() << "createDbView: successfully created " << (temporary == temporaryView ? qsl(" temporary view ") : qsl(" persistent view ")) << name;
        return true;
    }
    qCritical() << "createDbView: Faild to create view " << name;
    return false;
}

}

inline bool createTemporaryDbView(const QString& name, const QString& sql, const QSqlDatabase& db = QSqlDatabase::database())
{
    return createDbView(name, sql, temporaryView, db);
}
inline bool createPersistentDbView(const QString& name, const QString& sql, const QSqlDatabase& db = QSqlDatabase::database())
{
    return createDbView(name, sql, persistentView, db);
}

#endif // SQLHELPER_H
