#ifndef TESTHELPER_H
#define TESTHELPER_H

#include "../DKV2/helper_core.h"

class dbgTimer
{
    QElapsedTimer t;
    QString fname;
    qint64 labcount =0;
    qint64 lastLab =0;
public:
    dbgTimer() {t.start();}
    dbgTimer(const dbgTimer&) = delete;
    dbgTimer(const QString& fu) : fname(fu){t.start(); qInfo().noquote() << qsl("Debug Timer ") + fname << qsl(" start") << qsl("\n");}
    ~dbgTimer() {qInfo().noquote() << "\n" << (fname.isEmpty() ? QString() : fname+ qsl(" end") )
                                    << "\n" << qsl("Elapsed time: ")<< t.elapsed() << "ms" << qsl("\n");}
    void lab(const QString& msg =QString()) {
        qint64 now =t.elapsed();
        qInfo().noquote() << (fname.isEmpty() ? QString() : fname) <<  qsl(" Lab# ")
                          << (msg.isEmpty() ? QString::number(labcount++) : msg)
                          << ":" << (now-lastLab)
                          << "ms (overall: " << now << ")";
        lastLab =now;
    }
};

inline const QString testDbFilename {qsl("./data/testdb.sqlite")};
inline const QString testTemplateDb {qsl("./data/template.sqlite")};

void getRidOfFile(QString fn);
void initTestDkDb_InMemory();
void initTestDkDb();
void createTestDkDb_wData();
void createTestDkDbTemplate();
void cleanupTestDkDbTemplate();
void initTestDkDbFromTemplate();

void createTestDb_withRandomData();
void cleanupTestDb_InMemory();
void cleanupTestDkDb();
void openDefaultDbConnection_InMemory();
void openDefaultDbConnection(QString file =testDbFilename);
void closeDefaultDbConnection();
void createEmptyFile(const QString& fn);
//int tableRecordCount(const QString& table, const QSqlDatabase& db =QSqlDatabase::database());
bool dbHasTable(const QString& tname, const QSqlDatabase& db =QSqlDatabase::database());
bool dbTableHasField(const QString& tname, const QString& fname, const QSqlDatabase &db =QSqlDatabase::database());
bool dbsHaveSameTables(const QString &fn1, const QString &fn2);
bool dbsHaveSameTables(const QSqlDatabase& db1, const QSqlDatabase& db2);
bool dbTablesHaveSameFields(const QString& table1, const QString& table2, const QSqlDatabase& db =QSqlDatabase::database());

int doAnnualSettlementAllContracts (int year);
bool dbCompare(const QString& left, const QString& right);

#define dbgDumpDB() dbgDumpDatabase(QString(__FUNCTION__));
void dbgDumpDatabase(const QString &testname);

#endif // TESTHELPER_H
