#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QString>
#include <QSqlDatabase>
#include <QElapsedTimer>

extern const QString testDbFilename;
extern const QString testTemplateDb;

void getRidOfFile(QString fn);
void initTestDb_InMemory();
void initTestDb();
void createTestDb();
void createTestDbTemplate();
void cleanupTestDbTemplate();
void initTestDbFromTemplate();

void createTestDb_withRandomData();
void cleanupTestDb_InMemory();
void cleanupTestDb();
void openDbConnection_InMemory();
void openDbConnection(QString file =testDbFilename);
void closeDbConnection( QSqlDatabase db =QSqlDatabase::database());
void createEmptyFile(const QString& fn);
int tableRecordCount(const QString& table, const QSqlDatabase& db =QSqlDatabase::database());
bool dbHasTable(const QString& tname, const QSqlDatabase& db =QSqlDatabase::database());
bool dbTableHasField(const QString& tname, const QString& fname, const QSqlDatabase &db =QSqlDatabase::database());
bool dbsHaveSameTables(const QString &fn1, const QString &fn2);
bool dbsHaveSameTables(const QSqlDatabase& db1, const QSqlDatabase& db2);
bool dbTablesHaveSameFields(const QString& table1, const QString& table2, const QSqlDatabase& db =QSqlDatabase::database());
#endif // TESTHELPER_H
